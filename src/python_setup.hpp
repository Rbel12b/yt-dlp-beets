#pragma once
#ifndef PYTHON_SETUP_HPP
#define PYTHON_SETUP_HPP
#include <string>
#include <filesystem>

namespace PythonSetup
{
    // pythonExe: path to embedded python on Windows, system python on Linux
    void SetupPythonEnv(const std::filesystem::path &pythonExe);
    std::filesystem::path getPythonPath();
};

#endif // PYTHON_SETUP_HPP
