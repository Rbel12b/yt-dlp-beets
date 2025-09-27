#pragma once
#ifndef APP_H
#define APP_H
#include "Renderer.hpp"

class App
{
public:
    App();
    ~App();

    int run(int argc, char** argv);

private:
    Renderer renderer;
};

#endif // APP_H