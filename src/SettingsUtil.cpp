#include "SettingsUtil.hpp"
#include "Utils.hpp"
#include <fstream>
#include <json/json.h>
#include <iostream>
#include <imgui_stdlib.h>

void settings_util::loadSettings(AppState &state)
{
    if (std::filesystem::exists(state.settings.fileName))
    {
        Json::Value settings;
        try
        {
            std::ifstream file(state.settings.fileName);
            file >> settings;
            file.close();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Failed to parse settings" << e.what() << '\n';
        }

        state.settings.audioDir = settings["audioDir"].asString();
        state.settings.videoDir = settings["videoDir"].asString();
        state.settings.tempAudioDir = settings["tempAudioDir"].asString();

        auto defaults = settings["defaults"];

        state.settings.defaults.download.audioOnly = defaults["download"]["audioOnly"].asBool();
        state.settings.defaults.download.playlist.noPlaylist = defaults["download"]["playlist"]["noPlaylist"].asBool();

        strncpy(state.settings.defaults.download.playlist.selectionBuffer, defaults["download"]["playlist"]["selectionBuffer"].asString().c_str(), 63);

        return;
    }
    state.settings.audioDir = Utils::getMusicDir();
    state.settings.videoDir = Utils::getVideosDir();
    state.settings.tempAudioDir = (std::filesystem::path(Utils::getDownloadsDir()) / "Music").string();
    saveSettings(state);
}

void settings_util::saveSettings(AppState &state)
{
    Json::Value settings;

    settings["audioDir"] = state.settings.audioDir;
    settings["videoDir"] = state.settings.videoDir;
    settings["tempAudioDir"] = state.settings.tempAudioDir;

    Json::Value defaults;
    
    defaults["download"]["audioOnly"] = state.settings.defaults.download.audioOnly;
    defaults["download"]["playlist"]["noPlaylist"] = state.settings.defaults.download.playlist.noPlaylist;
    
    defaults["download"]["playlist"]["selectionBuffer"] = std::string(state.settings.defaults.download.playlist.selectionBuffer);

    settings["defaults"] = defaults;

    std::ofstream file(state.settings.fileName);
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // use 2 spaces
    file << Json::writeString(writer, settings);
    file.close();
}

void settings_util::renderSettings(AppState &state, ImGuiWindowFlags flags)
{
    if (!ImGui::Begin("settings", nullptr, flags))
    {
        return;
    }
    ImGui::Text("Beets library (Audio library):");
    ImGui::SameLine();
    ImGui::InputText("##audioDir", &state.settings.audioDir);

    ImGui::Text("Video library:");
    ImGui::SameLine();
    ImGui::InputText("##videoDir", &state.settings.videoDir);

    ImGui::Text("Temporary audio files (before importing them to the Beets library):");
    ImGui::InputText("##tempAudioDir", &state.settings.tempAudioDir);
    ImGui::NewLine();

    ImGui::Text("Default values:");
    ImGui::Indent(15);
    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox("Audio only", &state.settings.defaults.download.audioOnly);

    ImGui::Checkbox("Do not download playlist", &state.settings.defaults.download.playlist.noPlaylist);

    ImGui::Text("Selection:");
    ImGui::SameLine();
    ImGui::InputText("##playlist_selection", state.settings.defaults.download.playlist.selectionBuffer, sizeof(state.download.playlist.selectionBuffer));

    ImGui::EndGroup();

    if (ImGui::Button("save"))
    {
        saveSettings(state);
        state.gui.tab = AppState::GUI::Tab::MAIN;
    }

    ImGui::End();
}
