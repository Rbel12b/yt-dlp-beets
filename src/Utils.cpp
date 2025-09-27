// Utils.cpp
#include "Utils.hpp"
#include <filesystem>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <io.h>
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
        int ret = RunCommand(cmd);
        return ret == 0;
    }

    int RunCommand(const std::string &cmd)
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

    bool LoadFileToString(const std::string &path, std::string &out)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (!ifs)
            return false;
        out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return true;
    }

} // namespace Utils
