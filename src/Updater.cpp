#include "Updater.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>

bool Updater::checkUpdate(AppState& state)
{
    auto versionFilePath = Utils::getUserDataDir() / ".version";
    if (!Utils::downloadFile(state.repoUrl + "/releases/latest/download/yt-dlp-beets.version", versionFilePath))
    {
        std::cout << "Failed to get latest version number";
        return false;
    }

    std::ifstream versionFile(versionFilePath, std::ios_base::in);
    std::string versionStr;
    versionFile >> versionStr;
    versionFile.close();

    std::cout << "latest version: " << versionStr << ", current version: " << state.version->m_version << "\n"; 

    if (versionStr == "Not Found")
    {
        return false;
    }

    Version latest = versionStr;

    if (latest > *state.version)
    {
        std::cout << "New version available.\n";
        updateAvailable = true;
        return true;
    }
    return false;
}

bool Updater::donwloadUpdate(AppState state)
{
#ifdef _WIN32
    auto newExePath = Utils::getUserDataDir() / "yt-dlp-beets-installer.exe";
    if (!Utils::downloadFile(state.repoUrl + "/releases/latest/download/yt-dlp-beets-installer-win64.exe", newExePath))
    {
        std::cout << "Failed to get latest appimage";
        return true;
    }
#else
    auto exePath = Utils::getExecutable();
    auto newExePath = exePath.string() + ".new";
    if (!Utils::downloadFile(state.repoUrl + "/releases/latest/download/yt-dlp-beets.AppImage", newExePath))
    {
        std::cout << "Failed to get latest appimage";
        return true;
    }
#endif
    return false;
}

void Updater::update(AppState& state)
{
#ifdef _WIN32
    auto newExePath = Utils::getUserDataDir() / "yt-dlp-beets-installer.exe";
    if (!std::filesystem::exists(newExePath))
    {
        std::cout << "latest installer not found, maybe download it?";
        return;
    }
    if (!Utils::runCommandDetached(newExePath.string(), ""))
    {
        std::cout << "Failed to start installer: " << newExePath << "\n";
    }
#else
    auto exePath = Utils::getExecutable();
    auto newExePath = exePath.string() + ".new";
    if (!std::filesystem::exists(newExePath))
    {
        std::cout << "latest appimage not found, maybe download it?";
        return;
    }
    std::filesystem::remove(exePath);
    std::filesystem::rename(newExePath, exePath);
    Utils::runCommand("chmod +x \"" + exePath.string() + "\"");
    if (!Utils::runCommandDetached(exePath, ""))
    {
        std::cout << "Failed to new AppIamge: " << newExePath << "\n";
    }
#endif
}
