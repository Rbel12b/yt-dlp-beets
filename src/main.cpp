#include "App.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <filesystem>
#include "Utils.hpp"

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
    std::filesystem::path logFilePath = Utils::getUserDataDir() / "log.txt";
    if (std::filesystem::exists(logFilePath))
    {
        std::filesystem::path oldLogFilePath = logFilePath.string() + ".old";
        if (std::filesystem::exists(oldLogFilePath))
        {
            std::filesystem::remove(oldLogFilePath);
        }
        std::filesystem::rename(logFilePath, oldLogFilePath);
    }
    std::string logFile = logFilePath.string();
    // Redirect C I/O
    FILE* f1 = freopen(logFile.c_str(), "w", stdout);
    FILE* f2 = freopen(logFile.c_str(), "a", stderr);

    // Redirect C++ streams as well
    std::ofstream out(logFile, std::ios::app);
    auto* oldCout = std::cout.rdbuf(out.rdbuf());
    auto* oldCerr = std::cerr.rdbuf(out.rdbuf());

    int ret = app.run(argc, argv, logFilePath);

    std::cout.rdbuf(oldCout);
    std::cerr.rdbuf(oldCerr);

    return ret;
}