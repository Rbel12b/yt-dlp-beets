#include "Beets.hpp"
#include "Utils.hpp"
#include <string>
#include <stdexcept>
#include "python_setup.hpp"
#include <iostream>
#include <fstream>
#include <rapidfuzz/fuzz.hpp>

#include "beets.config.yaml.str"

int beets::ensureConfig(AppState &state)
{
    std::string configStr(reinterpret_cast<const char *>(___resources_beets_config_yaml),
                          static_cast<size_t>(___resources_beets_config_yaml_len));

#ifdef _WIN32
    std::string configBegin = "directory: " + state.settings.audioDir + "\nlibrary: " +
                              (Utils::getUserDataDir() / ".." / ".." / "Roaming" / "beets" / "library.db").string() + "\n";
#else
    std::string configBegin = "directory: \"" + state.settings.audioDir + "\"\nlibrary: \"" +
                              (Utils::getUserDataDir() / ".." / ".." / ".local" / "share" / "beets" / "library.db").string() + "\"\n";
#endif

    std::filesystem::path configFilePath = "";

    try
    {
        if (!Utils::setEnv("FPCALC", Utils::getBundledExePath("fpcalc").string()))
        {
            throw std::runtime_error("Failed to set FPCALC enviroment variable");
        }

        std::vector<std::string> cmd = {"config", "-p"};
        std::string output;

        state.beets.backend->runBeetsCommand(cmd, [&output](const std::string &line)
                                             { output += line; });

        if (output.size() == 0)
        {
            throw std::runtime_error("Command returned nothing: " + cmd[0]);
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

int beets::BeetsBackend::runBeetsCommand(const std::vector<std::string> &cmd, std::function<void(const std::string &)> callback)
{
    return runFunc(cmd, callback);
}

bool beets::BeetsBackend::loadLibrary()
{
    tracks.clear();

    std::vector<std::string> cmd = {"ls", "-f", "$artist|$album|$title|$path"};

    return runBeetsCommand(cmd, [&](const std::string &line)
        {
        std::cout << line;
        if (line.empty()) return;
        std::stringstream ss(line);
        std::string artist, album, title, path;
        if (std::getline(ss, artist, '|') &&
            std::getline(ss, album, '|') &&
            std::getline(ss, title, '|') &&
            std::getline(ss, path, '|')) {
            while (path.back() == '\n' || path.back() == '\r')
            {
                path.pop_back();
            }
            tracks.push_back({artist, album, title, path});
        } }) == 0;
}

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const beets::BeetsTrack *>>> beets::BeetsBackend::search(std::string query)
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const BeetsTrack *>>> results;

    struct SearchResult
    {
        BeetsTrack *track;
        double score;
    };

    std::vector<SearchResult> searchResults;
    if (query.length() == 0)
    {
        return groupByArtistAlbum(tracks);
    }

    for (auto &track : tracks)
    {
        double s_artist = rapidfuzz::fuzz::partial_ratio(query, track.artist);
        double s_album = rapidfuzz::fuzz::partial_ratio(query, track.album);
        double s_title = rapidfuzz::fuzz::partial_ratio(query, track.title);
        double s_max = std::max({s_artist, s_album, s_title});

        if (s_max >= 70.0)
        {
            searchResults.push_back({&track, s_max});
        }
    }

    std::sort(searchResults.begin(), searchResults.end(),
              [](const SearchResult &a, const SearchResult &b)
              {
                  return a.score > b.score;
              });

    for (auto &result : searchResults)
    {
        results[result.track->artist][""].push_back(result.track);
    }

    return results;
}
