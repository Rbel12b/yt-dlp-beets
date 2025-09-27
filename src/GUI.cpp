#include "GUI.hpp"
#include <vector>
#include <string>
#include "Utils.hpp"
#include "python_setup.hpp"

void GUI::render(AppState &state)
{
    if (ImGui::BeginMainMenuBar())
    {
        renderMenuBar(state);
        ImGui::EndMainMenuBar();
    }
    renderErrorLogPopup(state);

    ImVec2 remainingSize = ImVec2(state.mainWindowSize.x, state.mainWindowSize.y - 19);
    ImVec2 mainImGuiWindowPos(0, state.mainWindowSize.y - remainingSize.y);
    ImVec2 mainImGuiWindowSize = remainingSize;

    // if (state.processTerm)
    // {
    //     ImVec2 terminalImGuiWindowSize = remainingSize;
    //     terminalImGuiWindowSize.y = terminalImGuiWindowSize.y / 3;
    //     mainImGuiWindowSize.y -= terminalImGuiWindowSize.y;
    //     ImGui::SetNextWindowPos(ImVec2(0, mainImGuiWindowPos.y + mainImGuiWindowSize.y));
    //     ImGui::SetNextWindowSize(terminalImGuiWindowSize);
    //     state.processTerm->render("Terminal", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    // }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::SetNextWindowPos(mainImGuiWindowPos);
    ImGui::SetNextWindowSize(mainImGuiWindowSize);
    if (!ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        return;
    }

    if (ImGui::Button("start beets"))
    {
        auto python = PythonSetup::getPythonPath();
        state.startCmdline = "\"" + python.string() + "\" -m beets import ~/Documents/Non-Album";
        // state.startCmdline = "echo Hi";
        state.startCommand = true;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void GUI::renderMenuBar(AppState &state)
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Exit", "Alt+F4"))
        {
            // Signal the application to exit
            state.progamShouldExit = true;
        }
        ImGui::EndMenu();
    }
}

void GUI::renderErrorLogPopup(AppState &state)
{
    static std::string content; // file contents
    static std::vector<char> contentBuf;        // mutable buffer for ImGui InputTextMultiline
   
    if (state.errorShowLog)
    {
        if (!Utils::LoadFileToString(state.logFile.string(), content))
            content = std::string("[Failed to open log file: ") + state.logFile.string() + "]";

        contentBuf.assign(content.begin(), content.end());
        contentBuf.push_back('\0');
        ImGui::OpenPopup("Error");
        state.errorShowLog = false; // Only open once
    }

    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("An error occurred. The application will now exit.");

        ImGui::TextUnformatted("Log file:");
        // Use InputText with read-only so user can easily select and copy the path
        {
            // Prepare a mutable buffer for the path (it's small)
            static std::vector<char> pathBuf;
            std::string pathStr = state.logFile.string();
            pathBuf.assign(pathStr.begin(), pathStr.end());
            pathBuf.push_back('\0');
            ImGui::InputText("##logpath", pathBuf.data(), pathBuf.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if (ImGui::Button("Copy path"))
            {
                ImGui::SetClipboardText(pathStr.c_str());
            }
        }

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            state.errorShowLog = false;
            state.progamShouldExit = true; // Signal the application to exit
        }

        ImGui::Separator();

        ImGui::TextUnformatted("Contents:");
        ImGui::BeginChild("LogContentChild", ImVec2(800, 400), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (!contentBuf.empty()) {
            // InputTextMultiline accepts the mutable buffer and size.
            // We use ImGuiInputTextFlags_ReadOnly so the user can't edit it.
            ImGui::InputTextMultiline("##logcontents",
                                    contentBuf.data(),
                                    contentBuf.size(),
                                    ImVec2(-1, -1),
                                    ImGuiInputTextFlags_ReadOnly);
        } else {
            ImGui::TextUnformatted("[Log is empty]");
        }

        ImGui::EndChild();

        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}
