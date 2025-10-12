#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

class Renderer;

#include <functional>
#include <thread>
#include <atomic>
#include <SDL2/SDL.h>
#include "imgui.h"
#include "AppState.hpp"

class Renderer
{
public:
    Renderer();
    ~Renderer();
    
    void setRenderFunction(std::function<void()> func);
    void setKeyCallback(std::function<void(const SDL_KeyboardEvent&)> func);

    int startRenderLoop(AppState* _state);

    bool isRunning() const { return running; }

    void join() { if (renderThread.joinable()) renderThread.join(); }

    ImVec2 getWindowSize() const;

    void* createTextureFromRGBA(const unsigned char* rgba, int width, int height);

private:
    int init();
    int setup();
    void beginFrame();
    void endFrame();
    void terminate();

    int renderLoop();

    void setupFonts();

private:
    std::function<void()> renderFunction;
    std::function<void(const SDL_KeyboardEvent&)> keyCallBack;
    bool initialized;
    std::atomic<bool> running;
    std::thread renderThread;
    SDL_Window *window;
    SDL_Renderer *renderer;
    AppState* state;
};

#endif // RENDERER_HPP