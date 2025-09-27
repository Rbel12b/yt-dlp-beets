#include "App.hpp"

App app;

#if WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int argc = __argc;
    char **argv = __argv;
#else
int main(int argc, char **argv)
{
#endif
    return app.run(argc, argv);
}