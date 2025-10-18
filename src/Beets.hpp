#pragma once

namespace beets
{
    class BeetsBackend;
}

#include "AppState.hpp"
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <unordered_map>

namespace beets
{
    int ensureConfig(AppState &state);

    // A track item in the Beets library
    struct BeetsTrack
    {
        std::string artist;
        std::string album;
        std::string title;
        std::string path;
        SDL_Texture *cover = nullptr;  // texture for cover art
    };

    class BeetsBackend
    {
    public:
        using runFuncType = std::function<int(const std::string &, std::function<void(const std::string &)>)>;
        runFuncType runFunc;
        
        std::vector<BeetsTrack> tracks;

        void setRunFunc(const runFuncType &_runFunc)
        {
            runFunc = _runFunc;
        }

        int runBeetsCommand(const std::string &, std::function<void(const std::string &)>);

        bool loadLibrary();

        auto groupByArtistAlbum(const std::vector<BeetsTrack>& _tracks) const
        {
            std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const BeetsTrack *>>> grouped;
            for (const auto &t : _tracks)
                grouped[t.artist][t.album].push_back(&t);
            return grouped;
        }

        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const BeetsTrack *>>>
        search(std::string query);
    };
}