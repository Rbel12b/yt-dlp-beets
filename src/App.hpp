#pragma once
#ifndef APP_H
#define APP_H
#include "Renderer.hpp"

void render();

class App
{
    friend void render();
public:
    App();
    ~App();

    int run(int argc, char **argv);

private:
    void render();

    Renderer renderer;
};

#endif // APP_H