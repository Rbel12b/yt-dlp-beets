#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include "imgui.h"
#include "SDL2/SDL.h"
#include <filesystem>
#include <string>
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

};

#endif // APP_STATE_H