#include "Beets.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include "python_setup.hpp"
#include <iostream>
#include <fstream>

#include "beets.config.yaml.str"

int beets::ensureConfig(AppState& state)
{
    std::string configStr(reinterpret_cast<const char*>(___resources_beets_config_yaml),
        static_cast<size_t>(___resources_beets_config_yaml_len));

#ifdef _WIN32
    std::string configBegin = "directory: " + state.audioDir.string() + "\nlibrary: " +
        (Utils::getUserDataDir() / ".." / ".." / "Roaming" / "beets" / "library.db").string() + "\n";
#else
    std::string configBegin = "directory: \"" + state.audioDir.string() + "\"\nlibrary: \"" +
        (Utils::getUserDataDir() / ".." / ".." / ".local" / "share" / "beets" / "library.db").string() + "\"\n";
#endif

    std::filesystem::path configFilePath = "";

    try
    {
        std::string cmd = PythonSetup::getPythonPath().string() + " -m beets config -p";
        std::string output = Utils::runCommandOutput(cmd);
        if (output.size() == 0)
        {
            throw std::runtime_error("command returned nothing: " + cmd);
        }
        while (output.back() == '\n' || output.back() == '\r')
        {
            output.pop_back();
        }
        configFilePath = output;
        std::cout << "beets config file: " << configFilePath << "\n";
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << "\n";
        return 1;
    }

    if (std::filesystem::exists(configFilePath))
    {
        return 0;
    }

    std::ofstream configFile(configFilePath, std::ios_base::out);
    configFile << configBegin << "\n";
    configFile << configStr;
    configFile.close();

    return 0;
}