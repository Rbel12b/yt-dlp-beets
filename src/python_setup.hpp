#pragma once
#ifndef PYTHON_SETUP_HPP
#define PYTHON_SETUP_HPP
#include <string>
#include <filesystem>
#include "AppState.hpp"

namespace PythonSetup
{
    // pythonExe: path to embedded python on Windows, system python on Linux
    int SetupPythonEnv(const std::filesystem::path &pythonExe, AppState& state);
    std::filesystem::path getPythonPath();
    std::filesystem::path getPythonScriptPath(std::string executable);
};

#endif // PYTHON_SETUP_HPP
