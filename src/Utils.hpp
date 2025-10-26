#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <filesystem>
#include "AppState.hpp"
#include <functional>

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
    std::filesystem::path getBundledExePath(const std::string &name);
    std::filesystem::path getBundledFilePath(const std::filesystem::path &name);
    std::filesystem::path getLicensePath();

    // Returns the absolute path of the running executable
    std::filesystem::path getExecutableDir();
    std::filesystem::path getExecutable();

    // Returns a user-writable directory for app data
    // Windows -> %LOCALAPPDATA%\yt-dlp-beets
    // Linux   -> $HOME/.config/yt-dlp-beets
    std::filesystem::path getUserDataDir();

    std::filesystem::path getTempDir();

    // Cross-platform file download using curl
    // url: remote file URL
    // dest: local path to save the file
    bool downloadFile(const std::string &url, const std::filesystem::path &dest, AppState& state);

    bool loadFileToString(const std::string &path, std::string &out);

    bool runInteractiveTerminal(const std::string &command);

    int setEnv(std::string name, std::string value);
}

#endif // UTILS_HPP
