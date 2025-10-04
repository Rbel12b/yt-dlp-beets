#pragma once
#ifndef APP_H
#define APP_H
#include "Renderer.hpp"
#include "GUI.hpp"
#include "AppState.hpp"

class App
{
public:
    App();
    ~App();

    int run(int argc, char **argv, std::filesystem::path logFile);

private:
    void init();
    void render();
    void keyCallback(const SDL_KeyboardEvent&);

    Renderer renderer;
    GUI gui;

    AppState state;
};

#endif // APP_H