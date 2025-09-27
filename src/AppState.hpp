#pragma once
#ifndef APP_STATE_H
#define APP_STATE_H

#include "imgui.h"
#include "SDL2/SDL.h"

struct AppState
{
    bool progamShouldExit = false;

    ImVec2 mainWindowSize = ImVec2(0, 0);

    bool pythonSetupComplete = false;
    bool pythonSetupInProgress = false;
};

#endif // APP_STATE_H