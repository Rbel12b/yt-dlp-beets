// Utils.cpp
#include "Utils.hpp"
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#endif

namespace Utils
{

    std::filesystem::path GetExecutableDir()
    {
        std::filesystem::path exeDir;

#ifdef _WIN32
        char buffer[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        if (len != 0)
        {
            exeDir = std::filesystem::path(buffer).parent_path();
        }
#else
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            exeDir = std::filesystem::path(buffer).parent_path();
        }
#endif

        return exeDir;
    }

    std::filesystem::path GetUserDataDir()
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

    bool DownloadFile(const std::string &url, const std::filesystem::path &dest)
    {
        std::string cmd = "curl -L -o \"" + dest.string() + "\" \"" + url + "\"";
        int ret = system(cmd.c_str());
        return ret == 0;
    }

} // namespace Utils
