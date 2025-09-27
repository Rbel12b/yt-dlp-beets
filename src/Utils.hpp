// Utils.hpp
#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h> // For SHGetKnownFolderPath
#include <combaseapi.h> // CoTaskMemFree
#else
#include <cstdlib>
#endif

namespace Utils
{

// Windows helper
#ifdef _WIN32
    std::string getKnownFolder(REFKNOWNFOLDERID folderId);
#endif

    std::string getVideosDir();
    std::string getMusicDir();
    std::string getDownloadsDir();

    // Returns the absolute path of the running executable
    std::filesystem::path GetExecutableDir();

    // Returns a user-writable directory for app data
    // Windows -> %LOCALAPPDATA%\yt-dlp-beets
    // Linux   -> $HOME/.config/yt-dlp-beets
    std::filesystem::path GetUserDataDir();

    // Cross-platform file download using curl
    // url: remote file URL
    // dest: local path to save the file
    bool DownloadFile(const std::string &url, const std::filesystem::path &dest);

    // Cross-platform command execution
    // Windows: hides console windows
    // Linux/macOS: runs via system()
    int RunCommand(const std::string &cmd);

    bool LoadFileToString(const std::string &path, std::string &out);

    bool runInteractiveTerminal(const std::string &command);

    std::filesystem::path GetBundledExePath(const std::string &name);
}

#endif // UTILS_HPP
