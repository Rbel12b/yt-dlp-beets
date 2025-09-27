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

    int run(int argc, char **argv);

private:
    void render();
    void keyCallback(const SDL_KeyboardEvent&);

    Renderer renderer;
    GUI gui;

    AppState state;
};

#endif // APP_H