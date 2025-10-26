// python_setup.cpp
#include "python_setup.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Utils.hpp"
#include "AppState.hpp"
#include "Rbel12b-cpplib/ProcessUtils/ProcessUtils.hpp"

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
            cpplib::Process proc2;
            proc2.setCommand(pythonExe);
            proc2.appendArgument(getPip.string());
            proc2.appendArgument("--user");

            std::cout << "Installing pip\n";

            proc2.run();

            if (proc2.getExitCode() != 0)
            {
                std::cerr << "Failed to install pip\n";
                return 1;
            }

            // Mark pip as installed
            std::ofstream marker(pipMarker);
            marker << "pip installed";

            cpplib::Process proc3;
            proc3.setCommand(pythonExe);
            proc3.appendArgument("-m");
            proc3.appendArgument("pip");
            proc3.appendArgument("install");
            proc3.appendArgument("--user");
            proc3.appendArgument("virtualenv");

            std::cout << "Installing virtualenv\n";
            proc3.run();
            if (proc3.getExitCode() != 0)
            {
                std::cerr << "Failed to install virtualenv\n";
                return 1;
            }

            cpplib::Process proc;
            proc.setCommand(pythonExe);
            proc.appendArgument("-m");
            proc.appendArgument("virtualenv");
            proc.appendArgument(venvDir.string());
#else
            cpplib::Process proc;
            proc.setCommand(pythonExe);
            proc.appendArgument("-m");
            proc.appendArgument("venv");
            proc.appendArgument(venvDir.string());
#endif

            std::cout << "Creating virtualenv\n";
            proc.run();
            if (proc.getExitCode() != 0)
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
            cpplib::Process proc;
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
            proc.setCommand(venvPython);
            proc.appendArgument("-m");
            proc.appendArgument("ensurepip");
#endif
            std::cout << "Installing pip\n";
            proc.run();
            if (proc.getExitCode() != 0)
            {
                std::cerr << "Failed to install pip\n";
                return 1;
            }
            // Mark pip as installed
            std::ofstream marker(pipMarker);
            marker << "pip installed";
        }

        cpplib::Process proc;

        proc.setCommand(venvPython);
        proc.appendArgument("-m");
        proc.appendArgument("pip");
        proc.appendArgument("install");;
        proc.appendArgument("--upgrade");
        proc.appendArgument("pip");
        proc.appendArgument("beets[fetchart,lyrics,embedart,chroma]");
        proc.appendArgument("yt-dlp");

        std::cout << "Installing beets\n";
        proc.run();
        if (proc.getExitCode() != 0)
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