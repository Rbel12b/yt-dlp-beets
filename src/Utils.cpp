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
#include <AppState.hpp>

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
        static const std::string appName = "yt-dlp-beets";
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
            state->commandProgress = percent;
        }
        else
        {
            state->commandProgress = 0;
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

    int runCommand(const std::string &cmd)
    {
#ifdef _WIN32
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;

        // Get underlying OS handles from the C runtime FILE*
        HANDLE hStdOut = (HANDLE)_get_osfhandle(_fileno(stdout));
        HANDLE hStdErr = (HANDLE)_get_osfhandle(_fileno(stderr));

        // Duplicate as inheritable
        HANDLE hOutInherit = NULL, hErrInherit = NULL;
        DuplicateHandle(GetCurrentProcess(), hStdOut,
                        GetCurrentProcess(), &hOutInherit,
                        0, TRUE, DUPLICATE_SAME_ACCESS);
        DuplicateHandle(GetCurrentProcess(), hStdErr,
                        GetCurrentProcess(), &hErrInherit,
                        0, TRUE, DUPLICATE_SAME_ACCESS);

        si.hStdOutput = hOutInherit;
        si.hStdError = hErrInherit;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        // Mutable command buffer
        std::string fullCmd = "cmd /C \"" + cmd + "\"";
        char *cmdline = fullCmd.data();

        BOOL ok = CreateProcessA(
            nullptr,
            cmdline,
            nullptr,
            nullptr,
            TRUE, // allow handle inheritance
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi);

        if (!ok)
        {
            if (hOutInherit)
                CloseHandle(hOutInherit);
            if (hErrInherit)
                CloseHandle(hErrInherit);
            return -1;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Close inherited handles (parent side copies)
        if (hOutInherit)
            CloseHandle(hOutInherit);
        if (hErrInherit)
            CloseHandle(hErrInherit);

        return static_cast<int>(exitCode);
#else
        return system(cmd.c_str());
#endif
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

    std::string runCommandOutput(const std::string &cmd)
    {
        std::cout << "executing: " << cmd << "\n";
        std::string output;
#if defined(_WIN32)
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;

        HANDLE hRead = NULL, hWrite = NULL;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0))
        {
            throw std::runtime_error("Failed to create pipe");
        }
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = hWrite;
        si.hStdError = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        std::string fullCmd = "cmd /C " + cmd;
        char *mutableCmd = fullCmd.data();

        if (!CreateProcessA(
                nullptr, mutableCmd, nullptr, nullptr,
                TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
                &si, &pi))
        {
            CloseHandle(hRead);
            CloseHandle(hWrite);
            throw std::runtime_error("CreateProcess failed");
        }

        CloseHandle(hWrite); // parent only reads

        std::array<char, 4096> buffer;
        DWORD bytesRead;
        while (ReadFile(hRead, buffer.data(), (DWORD)buffer.size() - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer.data();
            std::cout << buffer.data(); // forward live to parent's stdout
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hRead);

#else
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            throw std::runtime_error("pipe() failed");
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // child
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);
            _exit(127); // exec failed
        }

        // parent
        close(pipefd[1]);
        std::array<char, 4096> buffer;
        ssize_t n;
        while ((n = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0)
        {
            buffer[n] = '\0';
            output += buffer.data();
            std::cout << buffer.data();
        }
        close(pipefd[0]);

        int status = 0;
        waitpid(pid, &status, 0);
#endif
        return output;
    }

    bool runCommandDetached(const std::filesystem::path &exePath, const std::string &argString)
    {
#if defined(_WIN32)
        // Use ShellExecuteExW: it matches Explorer (double-click) behavior and accepts
        // parameters separately (no weird quoting of the exe path). See MS docs for ShellExecuteEx.
        std::wstring exeW = exePath.wstring();
        std::wstring paramsW = utf8_to_wstring(argString);

        SHELLEXECUTEINFOW sei{};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS; // if we want, we can get hProcess; we'll close it.
        sei.hwnd = NULL;
        sei.lpVerb = L"open";
        sei.lpFile = exeW.c_str(); // executable or document
        sei.lpParameters = paramsW.empty() ? nullptr : paramsW.c_str();
        sei.lpDirectory = NULL;
        sei.nShow = SW_SHOWNORMAL;

        if (!ShellExecuteExW(&sei))
        {
            // failed to launch
            return false;
        }

        // If ShellExecuteEx returned a process handle, close it (we're detached).
        if (sei.hProcess)
        {
            CloseHandle(sei.hProcess);
        }
        return true;

#else
        // Build argv: argv[0] = exePath, rest from parsed argString
        std::string exeStr = exePath.string();
        std::vector<std::string> tokens = splitArgString(argString);

        // Build argv array (char* const [])
        std::vector<char *> argv;
        argv.reserve(1 + tokens.size() + 1);
        argv.push_back(const_cast<char *>(exeStr.c_str()));
        for (auto &t : tokens)
            argv.push_back(const_cast<char *>(t.c_str()));
        argv.push_back(nullptr);

        pid_t pid = fork();
        if (pid < 0)
        {
            return false; // fork failed
        }
        if (pid > 0)
        {
            // parent: don't wait, return success
            return true;
        }

        // child
        // detach from controlling terminal / session
        setsid();

        // Optional: redirect stdin/out/err to /dev/null to avoid tying to parent's fds.
        // (If you *want* child to inherit parent's stdout/stderr, remove these steps.)
        int fd = open("/dev/null", O_RDWR);
        if (fd != -1)
        {
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > 2)
                close(fd);
        }

        // execv expects a NULL-terminated array of char* with argv[0] typically the program name
        execv(exeStr.c_str(), argv.data());
        // execv returns only on failure:
        _exit(127);
#endif
    }

    std::filesystem::path getBundledExePath(const std::string &name)
    {
#ifdef _WIN32
        return getExecutableDir() / "bin" / (name + ".exe");
#else
        return name;
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
            licensePath = std::filesystem::path(appdir) / "usr/share/licenses/yt-dlp-beets/LICENSES_COMBINED.txt";
        }
#endif
        return licensePath;
    }

} // namespace Utils
