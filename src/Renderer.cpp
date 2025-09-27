#include "Renderer.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <windows.h> // SetProcessDPIAware()
#endif

Renderer::Renderer()
    : initialized(false), running(false), renderFunction(nullptr)
{
}

Renderer::~Renderer()
{
    if (running)
    {
        running = false;
        if (renderThread.joinable())
            renderThread.join();
    }
}

int Renderer::init()
{
    if (initialized)
        return 0; // Already initialized

    if (setup() != 0)
        return -1; // Setup failed

    initialized = true;
    return 0;
}

void Renderer::setRenderFunction(std::function<void()> func)
{
    renderFunction = func;
}

int Renderer::setup()
{
    if (initialized)
        return 0; // Already initialized
    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    initialized = true;
    return 0;
}

void Renderer::terminate()
{
    if (!initialized)
        return;

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
    initialized = false;
}

void Renderer::beginFrame()
{
    int display_w, display_h;
    SDL_GetWindowSize(window, &display_w, &display_h);
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Renderer::endFrame()
{
    // Rendering
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

int Renderer::startRenderLoop()
{
    if (running)
        return -1; // Already running

    running = true;
    renderThread = std::thread(&Renderer::renderLoop, this);
    return 0;
}

int Renderer::renderLoop()
{
    if (init() != 0)
        return -1;

    if (!running)
        return -1; // Not running

    bool done = false;
    SDL_Event event;
    while (running && !done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        beginFrame();

        // Call the user-defined render function
        if (renderFunction)
            renderFunction();

        endFrame();
    }

    running = false;
    terminate();
    return 0;
}

ImVec2 Renderer::getWindowSize() const
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return ImVec2((float)w, (float)h);
}
