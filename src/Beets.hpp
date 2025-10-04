#pragma once
#include "AppState.hpp"
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <unordered_map>

namespace beets
{
    int ensureConfig(AppState& state);


    // A track item in the Beets library
    struct BeetsTrack {
        std::string artist;
        std::string album;
        std::string title;
        std::string path;
    };

    class BeetsBackend {
    public:
        std::vector<BeetsTrack> tracks;
        std::function<int(const std::string&, std::function<void(const std::string&)>)> runFunc;

        void setRunFunc(const std::function<int(const std::string&, std::function<void(const std::string&)>)>& _runFunc)
        {
            runFunc = _runFunc;
        }

        bool loadLibrary();

        auto groupByArtistAlbum() const {
            std::unordered_map<std::string, std::unordered_map<std::string, std::vector<const BeetsTrack*>>> grouped;
            for (const auto& t : tracks)
                grouped[t.artist][t.album].push_back(&t);
            return grouped;
        }
    };
}