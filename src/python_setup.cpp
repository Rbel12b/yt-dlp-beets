// python_setup.cpp
#include "python_setup.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Utils.hpp"

#ifdef _WIN32
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#endif

namespace fs = std::filesystem;

namespace PythonSetup
{

    void SetupPythonEnv(const fs::path &pythonExe)
    {
        fs::path dataDir = Utils::GetUserDataDir();
        fs::path venvDir = dataDir / "venv";
        fs::path pipMarker = dataDir / ".pip_installed";

        // 1. Create venv if it doesn't exist
        if (!fs::exists(venvDir))
        {
            std::string cmd;
#ifdef _WIN32
            // Wrap both the python path and the venv path in quotes using cmd /C
            cmd = "cmd /C \"\\\"" + pythonExe.string() + "\\\" -m venv \\\"" + venvDir.string() + "\\\"\"";
#else
            // Linux/macOS: normal quotes work
            cmd = "\"" + pythonExe.string() + "\" -m venv \"" + venvDir.string() + "\"";
#endif

            std::cout << "Creating virtualenv: " << cmd << std::endl;
            if (system(cmd.c_str()) != 0)
            {
                std::cerr << "Failed to create virtual environment\n";
                return;
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
                if (!Utils::DownloadFile(url, getPip.string()))
                {
                    std::cerr << "Failed to download get-pip.py\n";
                    return;
                }
            }
            std::string cmd = "\"" + venvPython.string() + "\" \"" + getPip.string() + "\" --user";
#else
            // Linux: system python venv already has ensurepip
            std::string cmd = "\"" + venvPython.string() + "\" -m ensurepip";
#endif
            std::cout << "Installing pip: " << cmd << std::endl;
            if (system(cmd.c_str()) != 0)
            {
                std::cerr << "Failed to install pip\n";
                return;
            }

            // Mark pip as installed
            std::ofstream marker(pipMarker);
            marker << "pip installed";
        }

        // 3. Install beets with extras
        std::string beetsCmd = "\"" + venvPython.string() + "\" -m pip install --upgrade pip "
                                                            "beets[fetchart,lyrics,mbsubmit,embedart,chroma]";
        std::cout << "Installing beets: " << beetsCmd << std::endl;
        if (system(beetsCmd.c_str()) != 0)
        {
            std::cerr << "Failed to install beets\n";
            return;
        }

        std::cout << "Python environment is ready in: " << venvDir << std::endl;
    }

    fs::path getPythonPath()
    {
        fs::path dataDir = Utils::GetUserDataDir();
        fs::path venvDir = dataDir / "venv";
#ifdef _WIN32
        fs::path venvPython = venvDir / "Scripts" / "python.exe";
#else
        fs::path venvPython = venvDir / "bin" / "python3";
#endif
        return venvPython;
    }

} // namespace PythonSetup