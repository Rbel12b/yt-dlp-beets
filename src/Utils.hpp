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
    std::filesystem::path getBundledExePath(const std::string &name);

    // Returns the absolute path of the running executable
    std::filesystem::path getExecutableDir();
    std::filesystem::path getExecutable();

    // Returns a user-writable directory for app data
    // Windows -> %LOCALAPPDATA%\yt-dlp-beets
    // Linux   -> $HOME/.config/yt-dlp-beets
    std::filesystem::path getUserDataDir();

    // Cross-platform file download using curl
    // url: remote file URL
    // dest: local path to save the file
    bool downloadFile(const std::string &url, const std::filesystem::path &dest);

    bool loadFileToString(const std::string &path, std::string &out);

    // Cross-platform command execution
    // Windows: hides console windows
    // Linux/macOS: runs via system()
    int runCommand(const std::string &cmd);

    bool runInteractiveTerminal(const std::string &command);

    bool runCommandDetached(const std::string &cmd);

    /**
     * execute a command and return the output.
     * @param cmd the command to execute
     * @returns the output (stdout)
     * @exception throws std::runtime_error if failed
     */
    std::string runCommandOutput(const std::string& cmd);
    

}

#endif // UTILS_HPP
