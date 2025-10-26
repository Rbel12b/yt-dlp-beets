#include "Utils.hpp"
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <array>
#include <memory>
#include <vector>
#include <curl/curl.h>
#include "AppState.hpp"
#include <functional>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <limits.h>
#include <fcntl.h>
#endif

namespace Utils
{
    static const std::string appName = "yt-dlp-beets";

    // Simple splitter: supports double quotes and backslash escaping.
    // Not a full shell parser, but handles common cases like:
    //   --opt "some value" arg\ with\ spaces
    static std::vector<std::string> splitArgString(const std::string &s)
    {
        std::vector<std::string> out;
        std::string cur;
        bool inQuotes = false;
        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];
            if (c == '"')
            {
                inQuotes = !inQuotes;
                continue;
            }
            if (c == '\\' && i + 1 < s.size())
            {
                // simple escape
                ++i;
                cur.push_back(s[i]);
                continue;
            }
            if (!inQuotes && isspace(static_cast<unsigned char>(c)))
            {
                if (!cur.empty())
                {
                    out.push_back(cur);
                    cur.clear();
                }
                continue;
            }
            cur.push_back(c);
        }
        if (!cur.empty())
            out.push_back(cur);
        return out;
    }

#if defined(_WIN32)
    // UTF-8 -> wide
    static std::wstring utf8_to_wstring(const std::string &s)
    {
        if (s.empty())
            return {};
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        if (wlen == 0)
            return {};
        std::wstring w;
        w.resize(wlen);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], wlen);
        return w;
    }
#endif

// Windows helper
#ifdef _WIN32
    std::string getKnownFolder(REFKNOWNFOLDERID folderId)
    {
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &path)))
        {
            char buffer[MAX_PATH];
            wcstombs(buffer, path, MAX_PATH);
            CoTaskMemFree(path);
            return std::string(buffer);
        }
        return {};
    }
#endif

    std::string getVideosDir()
    {
#ifdef _WIN32
        return getKnownFolder(FOLDERID_Videos);
#else
        const char *home = std::getenv("HOME");
        if (!home)
            home = "";
        return std::filesystem::path(home) / "Videos";
#endif
    }

    std::string getMusicDir()
    {
#ifdef _WIN32
        return getKnownFolder(FOLDERID_Music);
#else
        const char *home = std::getenv("HOME");
        if (!home)
            home = "";
        return std::filesystem::path(home) / "Music";
#endif
    }

    std::string getDownloadsDir()
    {
#ifdef _WIN32
        return getKnownFolder(FOLDERID_Downloads);
#else
        const char *home = std::getenv("HOME");
        if (!home)
            home = "";
        return std::filesystem::path(home) / "Downloads";
#endif
    }

    std::filesystem::path getExecutableDir()
    {
        return getExecutable().parent_path();
    }

    std::filesystem::path getExecutable()
    {
        std::filesystem::path exePath;
#ifdef _WIN32
        char buffer[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        if (len != 0)
        {
            exePath = std::filesystem::path(buffer);
        }
#else
        // AppImage provides its own path in APPIMAGE
        if (const char *appImage = std::getenv("APPIMAGE"))
        {
            exePath = std::filesystem::path(appImage);
        }
        else
        {
            char buffer[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (len != -1)
            {
                buffer[len] = '\0';
                exePath = std::filesystem::path(buffer);
            }
        }
#endif
        return exePath;
    }

    std::filesystem::path getUserDataDir()
    {
        std::filesystem::path dataDir;

#ifdef _WIN32
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path)))
        {
            dataDir = path;
            dataDir /= appName;
        }
#else
        const char *home = getenv("HOME");
        if (!home)
        {
            struct passwd *pw = getpwuid(getuid());
            if (pw)
                home = pw->pw_dir;
        }
        if (home)
        {
            dataDir = home;
            dataDir /= ".config";
            dataDir /= appName;
        }
#endif

        if (!dataDir.empty() && !std::filesystem::exists(dataDir))
        {
            std::filesystem::create_directories(dataDir);
        }

        return dataDir;
    }

    std::filesystem::path getTempDir()
    {
        std::filesystem::path tempDir;

#ifdef _WIN32
        char path[MAX_PATH];
        DWORD len = GetTempPathA(MAX_PATH, path);
        if (len > 0 && len < MAX_PATH)
        {
            tempDir = path;
            tempDir /= appName;
        }
#else
        const char *tmp = getenv("TMPDIR");
        if (!tmp || *tmp == '\0')
            tmp = "/tmp";
        tempDir = tmp;
        tempDir /= appName;
#endif

        if (!tempDir.empty() && !std::filesystem::exists(tempDir))
        {
            std::filesystem::create_directories(tempDir);
        }

        return tempDir;
    }

    static size_t writeFileCallback(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        std::ofstream *ofs = static_cast<std::ofstream *>(stream);
        ofs->write(static_cast<char *>(ptr), size * nmemb);
        return size * nmemb;
    }

    static int progressCallback(void *clientp,
                                curl_off_t dltotal, curl_off_t dlnow,
                                curl_off_t ultotal, curl_off_t ulnow)
    {
        AppState *state = static_cast<AppState *>(clientp);

        if (dltotal > 0)
        {
            int percent = static_cast<int>((dlnow * 100) / dltotal);
            state->commandInProgress.progress = percent;
        }
        else
        {
            state->commandInProgress.progress = 0;
        }
        return 0; // return non-zero to abort
    }

    bool downloadFile(const std::string &url, const std::filesystem::path &dest, AppState &state)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
            return false;

        std::ofstream ofs(dest, std::ios::binary);
        if (!ofs)
        {
            curl_easy_cleanup(curl);
            std::cerr << "Failed to open file for write: " << dest << "\n";
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.81.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);

        // Progress callback
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &state);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        ofs.close();

        if (res != CURLE_OK)
            std::cerr << "Download failed: " << curl_easy_strerror(res) << std::endl;

        return res == CURLE_OK;
    }

    bool loadFileToString(const std::string &path, std::string &out)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (!ifs)
            return false;
        out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return true;
    }

    bool runInteractiveTerminal(const std::string &command)
    {
#if defined(_WIN32)
        // Windows: start a new cmd window for interactive use
        std::string fullCmd = "cmd /C \"" + command + "\"";
        return std::system(fullCmd.c_str()) == 0;

#elif defined(__linux__)
        // Linux: try common terminals
        const char *terminals[] = {"gnome-terminal", "konsole", "xterm", nullptr};
        for (int i = 0; terminals[i] != nullptr; ++i)
        {
            std::string fullCmd = std::string(terminals[i]) + " -- " + command + " &";
            if (std::system(fullCmd.c_str()) == 0)
                return true;
        }
        std::cerr << "No supported terminal emulator found.\n";
        return false;
#else
        std::cerr << "Unsupported platform.\n";
        return false;
#endif
    }

    int setEnv(std::string name, std::string value)
    {
    #if defined(_WIN32)
        // On Windows, use _putenv_s (safe CRT version)
        if (_putenv_s(name.c_str(), value.c_str()) != 0) {
            std::cerr << "Failed to set " << name << " environment variable.\n";
            return false;
        }
    #else
        // On POSIX, use setenv
        if (setenv(name.c_str(), value.c_str(), 1) != 0) {
            perror("setenv");
            return false;
        }
    #endif
        return true;
    }

    std::filesystem::path getBundledExePath(const std::string &name)
    {
#ifdef _WIN32
        return getExecutableDir() / "bin" / (name + ".exe");
#else
        return name;
#endif
    }

    std::filesystem::path getBundledFilePath(const std::filesystem::path &name)
    {
#ifdef _WIN32
        return getExecutableDir() / name;
#else
        std::filesystem::path path;
        const char *appdir = std::getenv("APPDIR");
        if (!appdir)
        {
            std::cerr << "APPDIR not set!\n";
            path = "." / name;
        }
        else
        {
            path = std::filesystem::path(appdir) / "usr/bin" / name;
        }
        return path;
#endif
    }

    std::filesystem::path getLicensePath()
    {
        std::filesystem::path licensePath;
#ifdef _WIN32
        licensePath = getExecutableDir() / "licenses" / "LICENSES_COMBINED.txt";
#else
        const char *appdir = std::getenv("APPDIR");
        if (!appdir)
        {
            std::cerr << "APPDIR not set!\n";
            licensePath = getExecutableDir() / ".." / "thirdparty" / "licenses" / "LICENSES_COMBINED.txt";
        }
        else
        {
            licensePath = std::filesystem::path(appdir) / "usr/share/licenses/" / appName / "LICENSES_COMBINED.txt";
        }
#endif
        return licensePath;
    }

} // namespace Utils
