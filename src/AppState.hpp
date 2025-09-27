#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include "imgui.h"
#include "SDL2/SDL.h"
#include <filesystem>
#include <string>

struct AppState
{
    bool progamShouldExit = false;

    ImVec2 mainWindowSize = ImVec2(0, 0);

    bool pythonSetupComplete = false;
    bool pythonSetupInProgress = false;

    bool errorShowLog = false;
    std::filesystem::path logFile;
};

#endif // APP_STATE_H