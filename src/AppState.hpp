#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

class AppState;

#include "imgui.h"
#include "SDL2/SDL.h"
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
#include "Updater.hpp"
#include <cstdint>
#include "Beets.hpp"

class AppState
{
public:
    struct Download
    {
        struct Playlist
        {
            bool noPlaylist = true;
            char selectionBuffer[64];
            size_t displayedIndex = 0;
        };
        bool start = false;
        bool audioOnly = true;
        char urlBuffer[1024];
        std::string optionsFileName = "";
        Playlist playlist;
    };

    struct GUI
    {
        enum class Tab
        {
            MAIN,
            SETTINGS
        };
        Tab tab = Tab::MAIN;
    } gui;

    Version *version;
    bool readyForUpdate = false;
    std::string repoUrl = "https://github.com/Rbel12b/yt-dlp-beets";
    Updater *updater;
    bool newVersionPopup = false;
    bool downloadUpdate = false;

    bool progamShouldExit = false;

    ImVec2 mainWindowSize = ImVec2(0, 0);

    bool pythonSetupComplete = false;
    struct
    {
        bool enabled = false;
        std::string text = "";
        int progress = -1;
        bool progressDisabled = true;
    } commandInProgress;

    std::filesystem::path logFile;

    bool startCommand = false;
    std::string startCmdline = "";

    Download download;

    struct
    {
        std::filesystem::path fileName;

        std::string audioDir;
        std::string tempAudioDir;
        std::string videoDir;

        struct
        {
            Download download;
        } defaults;

    } settings;
    struct
    {
        bool import = false;
        bool pickDir = false;
        std::filesystem::path dir;
        beets::BeetsBackend* backend;
    } beets;

    struct
    {
        bool enabled = false;
        std::filesystem::path filePath;
        std::string msg;
        bool errorLog = false;
    } showFile;
};

#endif // APP_STATE_H