#include "App.hpp"
#include "imgui.h"
#include <thread>
#include <chrono>
#include "python_setup.hpp"
#include "Utils.hpp"
#include <filesystem>
#include <iostream>
#include <atomic>
#include "imgui_additions.hpp"
#include "yt-dlp.hpp"
#include "portable-file-dialogs.h"
#include "Beets.hpp"
#include "Updater.hpp"
#include "version.def"

App::App()
{
    for (size_t i = 0; i < sizeof(state.download.urlBuffer); i++)
    {
        state.download.urlBuffer[i] = '\0';
    }
    for (size_t i = 0; i < sizeof(state.download.playlist.selectionBuffer); i++)
    {
        state.download.playlist.selectionBuffer[i] = '\0';
    }

    state.audioDir = Utils::getMusicDir();
    state.videoDir = Utils::getVideosDir();
    state.tempAudioDir = std::filesystem::path(Utils::getDownloadsDir()) / "Music";
    std::string versionStr(reinterpret_cast<const char *>(___yt_dlp_beets_version),
                           static_cast<size_t>(___yt_dlp_beets_version_len));
    state.version = new Version("");
    (*state.version) = versionStr;
    state.updater = new Updater();
}

App::~App()
{
}

void App::render()
{
    state.mainWindowSize = renderer.getWindowSize();

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
    gui.render(state);
}

void App::keyCallback(const SDL_KeyboardEvent &keyEvent)
{
    if (keyEvent.type == SDL_KEYDOWN)
    {
        if (keyEvent.keysym.sym == SDLK_F4 && (keyEvent.keysym.mod & KMOD_ALT))
        {
            state.progamShouldExit = true;
        }
    }
}

int App::run(int argc, char **argv, std::filesystem::path logFile)
{
    state.logFile = logFile;
    std::filesystem::path exeDir = Utils::getExecutableDir();
    try
    {
        std::filesystem::current_path(exeDir);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        // If changing directory fails, log the error but continue
        std::cerr << "Warning: Could not change working directory to executable directory: " << e.what() << std::endl;
    }
#ifdef _WIN32
    std::filesystem::path pythonExe = (exeDir / "python" / "python.exe");
#else
    std::filesystem::path pythonExe = "python3"; // Assume system python3 on Linux
#endif
    std::filesystem::path pythonPath = "";

    renderer.setRenderFunction(std::bind(&App::render, this));
    renderer.setKeyCallback(std::bind(&App::keyCallback, this, std::placeholders::_1));

    if (renderer.startRenderLoop(&state) != 0)
        return -1; // Failed to start render loop

    while (renderer.isRunning())
    {
        if (!state.pythonSetupComplete)
        {
            state.commandInProgress.text = "Setting up Python environment...";
            state.commandInProgress.progress = -1;
            state.commandInProgress.progressDisabled = true;
            state.commandInProgress.enabled = true;
            if (PythonSetup::SetupPythonEnv(pythonExe, state) != 0)
            {
                state.showFile.errorLog = true;
                state.showFile.enabled = true;
                state.commandInProgress.enabled = false;
                break; // Failed to setup python
            }
            pythonPath = PythonSetup::getPythonPath();
            if (beets::ensureConfig(state))
            {
                state.showFile.errorLog = true;
                state.showFile.enabled = true;
                state.commandInProgress.enabled = false;
                break; // Failed to setup beets
            }
            state.pythonSetupComplete = true;
            state.commandInProgress.progress = -1;
            state.commandInProgress.progressDisabled = false;
            if (state.updater && state.updater->checkUpdate(state))
            {
                state.newVersionPopup = true;
            }
            state.commandInProgress.enabled = false;
        }
        if (state.downloadUpdate)
        {
            state.downloadUpdate = false;
            state.commandInProgress.progress = 0;
            state.commandInProgress.progressDisabled = false;
            state.commandInProgress.text = "Donwloading update...";
            state.commandInProgress.enabled = true;
            if (state.updater && !state.updater->donwloadUpdate(state))
            {
                state.readyForUpdate = true;
                state.progamShouldExit = true;
            }
            else
            {
                state.showFile.errorLog = true;
                state.showFile.enabled = true;
            }
            state.commandInProgress.enabled = false;
            state.commandInProgress.progress = -1;
        }
        if (state.startCommand)
        {
            Utils::runInteractiveTerminal(state.startCmdline);
            state.startCommand = false;
        }
        if (state.download.start)
        {
            yt_dlp_utils::donwload(state);
            state.download.start = false;
        }
        if (state.beets.import)
        {
            if (state.beets.pickDir)
            {
                auto file_select = pfd::select_folder("Select a directory to import", pfd::path::home());
                state.beets.dir = file_select.result();
                std::cout << "Selected dir: " << state.beets.dir << "\n";
            }
            if (state.beets.dir.empty())
            {
                std::cout << "No dir selected.\n";
            }
            else
            {
                std::cout << "Importing to beets library.\n";
                Utils::runInteractiveTerminal(pythonPath.string() +
                                              " -m beets import \"" + state.beets.dir.string() + "\"");
            }
            state.beets.import = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout.flush();
    }

    renderer.join();

    if (state.readyForUpdate)
    {
        std::cout << "updating.\n";
        state.updater->update(state);
    }

    if (state.version)
    {
        delete state.version;
        state.version = nullptr;
    }

    if (state.updater)
    {
        delete state.updater;
        state.updater = nullptr;
    }

    return 0;
}