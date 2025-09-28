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
// #include "process_terminal.hpp"

class AppState
{
public:
    Version* version;
    bool readyForUpdate = false;
    std::string repoUrl = "https://github.com/Rbel12b/yt-dlp-beets";
    Updater* updater;
    bool newVersionPopup = false;
    bool downloadUpdate = false;

    bool progamShouldExit = false;

    ImVec2 mainWindowSize = ImVec2(0, 0);

    bool pythonSetupComplete = false;
    bool commandInProgress = false;
    std::string inProgressText = "";
    int commandProgress = -1;

    std::filesystem::path logFile;

    bool startCommand = false;
    std::string startCmdline = "";
    // ProcessTerminal* processTerm = nullptr;

    struct
    {
        bool start = false;
        bool audioOnly = true;
        char urlBuffer[1024];
        char flagsBuffer[1024];
        bool showplaylistOptions = false;
        struct
        {
            bool noPlaylist = true;
            char selectionBuffer[64];
            std::vector<std::string> flags;
        } playlist;
    } download;
    
    std::filesystem::path audioDir;
    std::filesystem::path tempAudioDir;
    std::filesystem::path videoDir;

    struct
    {
        bool import = false;
        bool pickDir = false;
        std::filesystem::path dir;
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