#include "GUI.hpp"

void GUI::render(const AppState& state)
{
    if (ImGui::BeginMainMenuBar())
    {
        renderMenuBar(state);
        ImGui::EndMainMenuBar();
    }
}

void GUI::renderMenuBar(const AppState& state)
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Exit", "Alt+F4"))
        {
            // Signal the application to exit
            const_cast<AppState&>(state).progamShouldExit = true;
        }
        ImGui::EndMenu();
    }
}
