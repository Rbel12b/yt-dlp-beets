// Utils.hpp
#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <filesystem>

namespace Utils
{

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
}

#endif // UTILS_HPP
