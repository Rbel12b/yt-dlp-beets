#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <functional>
#include <thread>
#include <atomic>
#include <SDL2/SDL.h>
#include "imgui.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();
    void setRenderFunction(std::function<void()> func);
    int startRenderLoop();
    bool isRunning() const { return running; }

    ImVec2 getWindowSize() const;

private:
    int init();
    int setup();
    void beginFrame();
    void endFrame();
    void terminate();

    int renderLoop();

private:
    std::function<void()> renderFunction;
    bool initialized;
    std::atomic<bool> running;
    std::thread renderThread;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

#endif // RENDERER_HPP