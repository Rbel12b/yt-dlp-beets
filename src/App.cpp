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

App::App()
{
}

App::~App()
{
}

void App::render()
{
    state.mainWindowSize = renderer.getWindowSize();

    if (state.pythonSetupInProgress)
    {
        ImVec2 windowSize = ImVec2(300, 100);
        ImGui::SetNextWindowPos(ImVec2((state.mainWindowSize.x - windowSize.x) * 0.5f, (state.mainWindowSize.y - windowSize.y) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::Begin("Setup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
        const char *text = "Setting up Python environment...";
        auto textSize = ImGui::CalcTextSize(text);
        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f - 10));
        ImGui::Text("Setting up Python environment...");
        ImGui::SetCursorPos(ImVec2((windowSize.x - 30) * 0.5f, (windowSize.y - 30) * 0.5f + 10));
        ImGui::Spinner("##spinner", 15.0f, 4, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        ImGui::End();
        return;
    }
    gui.render(state);
}

void App::keyCallback(const SDL_KeyboardEvent& keyEvent)
{
    if (keyEvent.type == SDL_KEYDOWN)
    {
        if (keyEvent.keysym.sym == SDLK_F4 && (keyEvent.keysym.mod & KMOD_ALT))
        {
            state.progamShouldExit = true;
        }
    }
}

int App::run(int argc, char **argv)
{
    std::filesystem::path exeDir = Utils::GetExecutableDir();
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
            state.pythonSetupInProgress = true;
            PythonSetup::SetupPythonEnv(pythonExe);
            pythonPath = PythonSetup::getPythonPath();
            state.pythonSetupInProgress = false;
            state.pythonSetupComplete = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    renderer.join();

    return 0;
}