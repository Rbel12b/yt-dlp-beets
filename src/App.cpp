#include "App.hpp"
#include "imgui.h"
#include <thread>
#include <chrono>

App::App()
{
}

App::~App()
{
}

void render()
{
    ImGui::ShowDemoWindow(nullptr);
}

int App::run(int argc, char** argv)
{
    renderer.setRenderFunction(render);

    if (renderer.startRenderLoop() != 0)
        return -1; // Failed to start render loop
    
    while (renderer.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}