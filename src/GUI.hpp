#pragma once
#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "AppState.hpp"

class GUI
{
public:
    GUI() = default;
    ~GUI() = default;
    void render(AppState& state);

private:
    void renderMenuBar(AppState& state);
    void renderErrorLogPopup(AppState& state);
    void renderMain(AppState& state);
    void renderPlayListOptions(AppState& state);
};

#endif // GUI_H