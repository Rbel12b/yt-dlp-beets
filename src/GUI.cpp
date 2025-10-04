#include "GUI.hpp"
#include <vector>
#include <string>
#include "Utils.hpp"
#include "python_setup.hpp"
#include "yt-dlp.hpp"
#include "imgui_additions.hpp"
#include "SettingsUtil.hpp"

void GUI::render(AppState &state)
{
    if (ImGui::BeginMainMenuBar())
    {
        renderMenuBar(state);
        ImGui::EndMainMenuBar();
    }

    renderErrorLogPopup(state);
    renderNewVersionPopup(state);
    renderInProgressPopup(state);

    ImVec2 remainingSize = ImVec2(state.mainWindowSize.x, state.mainWindowSize.y - 19);
    ImVec2 mainImGuiWindowPos(0, state.mainWindowSize.y - remainingSize.y);
    ImVec2 mainImGuiWindowSize = remainingSize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::SetNextWindowPos(mainImGuiWindowPos);
    ImGui::SetNextWindowSize(mainImGuiWindowSize);

    switch (state.gui.tab)
    {
    case AppState::GUI::Tab::SETTINGS:
        settings_util::renderSettings(state);
        break;

    case AppState::GUI::Tab::MAIN:
        if (ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
        {
            renderMain(state);
            ImGui::End();
        }
        break;
    
    default:
        break;
    }

    ImGui::PopStyleVar();
}

void GUI::renderInProgressPopup(AppState &state)
{
    if (state.commandInProgress.enabled)
    {
        ImGui::OpenPopup("inprogress");
    }
    ImVec2 windowSize = ImVec2(300, 100);
    ImGui::SetNextWindowPos(ImVec2((state.mainWindowSize.x - windowSize.x) * 0.5f, (state.mainWindowSize.y - windowSize.y) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    if (ImGui::BeginPopupModal("inprogress", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar))
    {
        const char *text = state.commandInProgress.text.c_str();
        auto textSize = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f - 10));
        ImGui::Text(text);
        if (state.commandInProgress.progressDisabled || state.commandInProgress.progress < 0)
        {
            ImGui::SetCursorPos(ImVec2((windowSize.x - 30) * 0.5f, (windowSize.y - 30) * 0.5f + 10));
            ImGui::Spinner("##spinner", 15.0f, 4, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        }
        else
        {
            ImGui::ProgressBar((float)state.commandInProgress.progress / 100);
        }

        if (!state.commandInProgress.enabled)
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::End();
    }
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
        if (ImGui::MenuItem("Settings"))
        {
            state.gui.tab = AppState::GUI::Tab::SETTINGS;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("About"))
    {
        if (ImGui::MenuItem("Licenses"))
        {
            state.showFile.msg = "Licenses:";
            state.showFile.filePath = Utils::getLicensePath();
            state.showFile.errorLog = false;
            state.showFile.enabled = true;
        }
        ImGui::MenuItem("Version: ", state.version ? state.version->m_version.c_str() : "unknown", nullptr, false);
        ImGui::EndMenu();
    }
}

void GUI::renderErrorLogPopup(AppState &state)
{
    static std::string content; // file contents
    static std::vector<char> contentBuf;        // mutable buffer for ImGui InputTextMultiline
    static std::filesystem::path filePath;
   
    if (state.showFile.enabled)
    {
        if (state.showFile.errorLog)
        {
            filePath = state.logFile;
        }
        else
        {
            filePath = state.showFile.filePath;
        }

        if (!Utils::loadFileToString(filePath.string(), content))
            content = std::string("[Failed to open file: ") + filePath.string() + "]";

        contentBuf.assign(content.begin(), content.end());
        contentBuf.push_back('\0');
        ImGui::OpenPopup("showFile");
        state.showFile.enabled = false; // Only open once
    }

    if (ImGui::BeginPopupModal("showFile", nullptr, ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        if (state.showFile.errorLog)
        {
            ImGui::Text("An error occurred. The application will now exit.");
            ImGui::TextUnformatted("Log file:");
        }
        else if (!state.showFile.msg.empty())
        {
            ImGui::Text(state.showFile.msg.c_str());
        }

        // Use InputText with read-only so user can easily select and copy the path
        {
            // Prepare a mutable buffer for the path (it's small)
            static std::vector<char> pathBuf;
            std::string pathStr = filePath.string();
            pathBuf.assign(pathStr.begin(), pathStr.end());
            pathBuf.push_back('\0');
            ImGui::InputText("##showFilePath", pathBuf.data(), pathBuf.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();
            if (ImGui::Button("Copy path"))
            {
                ImGui::SetClipboardText(pathStr.c_str());
            }
        }

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            state.showFile.enabled = false;
            state.progamShouldExit = state.showFile.errorLog; // Signal the application to exit when an error occured
        }

        ImGui::Separator();

        ImGui::TextUnformatted("Contents:");
        ImGui::BeginChild("showFileContentChild", ImVec2(800, 400), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (!contentBuf.empty()) {
            // InputTextMultiline accepts the mutable buffer and size.
            // We use ImGuiInputTextFlags_ReadOnly so the user can't edit it.
            ImGui::InputTextMultiline("##fileContents",
                                    contentBuf.data(),
                                    contentBuf.size(),
                                    ImVec2(-1, -1),
                                    ImGuiInputTextFlags_ReadOnly);
        } else {
            ImGui::TextUnformatted("[file is empty]");
        }

        ImGui::EndChild();

        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void GUI::renderMain(AppState &state)
{
    ImGui::Text("Input URL:");
    ImGui::SameLine();
    ImGui::InputText("##Input_URL", state.download.urlBuffer, sizeof(state.download.urlBuffer) - 1);

    ImGui::Checkbox("Audio only", &state.download.audioOnly);

    ImGui::Checkbox("Do not download playlist", &state.download.playlist.noPlaylist);

    ImGui::Text("Selection:");
    ImGui::SameLine();
    ImGui::InputText("##playlist_selection", state.download.playlist.selectionBuffer, sizeof(state.download.playlist.selectionBuffer));

    if (ImGui::Button("Download"))
    {
        state.download.start = true;
    }

    if (ImGui::Button("Import downloaded files to beets library"))
    {
        state.beets.dir = state.settings.tempAudioDir;
        state.beets.pickDir = false;
        state.beets.import = true;
    }

    if (ImGui::Button("Import directory to beets library"))
    {
        state.beets.pickDir = true;
        state.beets.import = true;
    }
}

void GUI::renderNewVersionPopup(AppState &state)
{
    if (state.newVersionPopup)
    {
        ImGui::OpenPopup("newvesion");
        state.newVersionPopup = false;
    }
    if (!ImGui::BeginPopupModal("newvesion", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        return;
    }

    ImGui::Dummy(ImVec2(10, 10));
    ImGui::Dummy(ImVec2(10, 10));
    ImGui::SameLine();
    ImGui::Text("A new version is available,\ndo you want to download it?\n(The program will restart)");
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10, 10));

    ImGui::Dummy(ImVec2(10, 10));
    ImGui::Dummy(ImVec2(50, 10));
    ImGui::SameLine();
    if (ImGui::Button("Yes"))
    {
        state.downloadUpdate = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("No"))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::Dummy(ImVec2(10, 10));

    ImGui::SetItemDefaultFocus();
    ImGui::EndPopup();
}
