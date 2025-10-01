// python_setup.cpp
#include "python_setup.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "AppState.hpp"

#ifdef _WIN32
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#endif

namespace fs = std::filesystem;

namespace PythonSetup
{

    int SetupPythonEnv(const fs::path &pythonExe, AppState& state)
    {
        fs::path dataDir = Utils::getUserDataDir();
        fs::path venvDir = dataDir / "venv";
        fs::path pipMarker = dataDir / ".pip_installed";

        // 1. Create venv if it doesn't exist
        if (!fs::exists(venvDir))
        {
            std::string cmd;
#ifdef _WIN32
            fs::path getPip = dataDir / "get-pip.py";
            if (!fs::exists(getPip))
            {
                std::string url = "https://bootstrap.pypa.io/get-pip.py";
                if (!Utils::downloadFile(url, getPip.string(), state))
                {
                    std::cerr << "Failed to download get-pip.py\n";
                    return 1;
                }
            }

            cmd = "\"" + pythonExe.string() + "\" \"" + getPip.string() + "\" --user";
            std::cout << "Installing pip: " << cmd << std::endl;
            if (Utils::runCommand(cmd) != 0)
            {
                std::cerr << "Failed to install pip\n";
                return 1;
            }

            // Mark pip as installed
            std::ofstream marker(pipMarker);
            marker << "pip installed";

            cmd = "\"" + pythonExe.string() + "\" -m pip install --user virtualenv";
            std::cout << "Installing virtualenv: " << cmd << std::endl;
            if (Utils::runCommand(cmd) != 0)
            {
                std::cerr << "Failed to install virtualenv\n";
                return 1;
            }

            // Wrap both the python path and the venv path in quotes using cmd /C
            cmd = "\"" + pythonExe.string() + "\" -m virtualenv \"" + venvDir.string() + "\"";
#else
            // Linux/macOS: normal quotes work
            cmd = "\"" + pythonExe.string() + "\" -m venv \"" + venvDir.string() + "\"";
#endif

            std::cout << "Creating virtualenv: " << cmd << std::endl;
            if (Utils::runCommand(cmd) != 0)
            {
                std::cerr << "Failed to create virtual environment\n";
                return 1;
            }
        }

        // Determine python executable inside venv
#ifdef _WIN32
        fs::path venvPython = venvDir / "Scripts" / "python.exe";
#else
        fs::path venvPython = venvDir / "bin" / "python3";
#endif

        // 2. Install pip if needed
        if (!fs::exists(pipMarker))
        {
#ifdef _WIN32
            fs::path getPip = dataDir / "get-pip.py";
            if (!fs::exists(getPip))
            {
                std::string url = "https://bootstrap.pypa.io/get-pip.py";
                if (!Utils::downloadFile(url, getPip.string(), state))
                {
                    std::cerr << "Failed to download get-pip.py\n";
                    return 1;
                }
            }
            std::string cmd = "\"" + venvPython.string() + "\" \"" + getPip.string() + "\"";
#else
            // Linux: system python venv already has ensurepip
            std::string cmd = "\"" + venvPython.string() + "\" -m ensurepip";
#endif
            std::cout << "Installing pip: " << cmd << std::endl;
            if (Utils::runCommand(cmd) != 0)
            {
                std::cerr << "Failed to install pip\n";
                return 1;
            }
            // Mark pip as installed
            std::ofstream marker(pipMarker);
            marker << "pip installed";
        }

        // 3. Install beets with extras
        std::string beetsCmd = "\"" + venvPython.string() + "\" -m pip install --upgrade pip "
                                                            "beets[fetchart,lyrics,embedart,chroma] yt-dlp";
        std::cout << "Installing beets: " << beetsCmd << std::endl;
        if (Utils::runCommand(beetsCmd) != 0)
        {
            std::cerr << "Failed to install beets\n";
            return 1;
        }

        std::cout << "Python environment is ready in: " << venvDir << std::endl;
        return 0;
    }

    fs::path getPythonPath()
    {
        fs::path dataDir = Utils::getUserDataDir();
        fs::path venvDir = dataDir / "venv";
#ifdef _WIN32
        fs::path venvPython = venvDir / "Scripts" / "python.exe";
#else
        fs::path venvPython = venvDir / "bin" / "python3";
#endif
        return venvPython;
    }

    fs::path getPythonScriptPath(std::string executable)
    {
        fs::path dataDir = Utils::getUserDataDir();
        fs::path venvDir = dataDir / "venv";
#ifdef _WIN32
        fs::path path = venvDir / "Scripts" / (executable + ".exe");
#else
        fs::path path = venvDir / "bin" / executable;
#endif
        return path;
    }

} // namespace PythonSetup