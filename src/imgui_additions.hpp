#pragma once
#ifndef IMGUI_ADDITIONS_HPP
#define IMGUI_ADDITIONS_HPP
#include "imgui.h"

namespace ImGui {
    // Spinner widget, from https://github.com/ocornut/imgui/issues/1901
    bool Spinner(const char* label, float radius, int thickness, const ImU32& color);
}

#endif // IMGUI_ADDITIONS_HPP