#pragma once

#include "AppState.hpp"
#include <imgui.h>

namespace settings_util
{
    void loadSettings(AppState& state);
    void saveSettings(AppState& state);
    void renderSettings(AppState& state, ImGuiWindowFlags flags = 0);
};