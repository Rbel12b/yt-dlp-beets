#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include "imgui.h"
#include "SDL2/SDL.h"
#include <filesystem>
#include <string>
#include <vector>
// #include "process_terminal.hpp"

struct AppState
{
    bool progamShouldExit = false;

    ImVec2 mainWindowSize = ImVec2(0, 0);

    bool pythonSetupComplete = false;
    bool pythonSetupInProgress = false;

    bool errorShowLog = false;
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
};

#endif // APP_STATE_H