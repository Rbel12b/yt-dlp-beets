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
#else
    auto exPath = Utils::getExecutable();
    auto AppImagePath = exPath.string() + ".new";
    if (!Utils::downloadFile(state.repoUrl + "/releases/latest/download/yt-dlp-beets.AppImage", AppImagePath))
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
#else
    auto exPath = Utils::getExecutable();
    auto AppImagePath = exPath.string() + ".new";
    if (!std::filesystem::exists(AppImagePath))
    {
        std::cout << "latest appimage not found, maybe donwload it?";
        return;
    }
    std::filesystem::remove(exPath);
    std::filesystem::rename(AppImagePath, exPath);
    Utils::runCommand("chmod +x \"" + AppImagePath + "\"");
    Utils::runCommandDetached(AppImagePath);
#endif
}
