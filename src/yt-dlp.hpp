#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "AppState.hpp"

namespace yt_dlp_utils
{
    void download(AppState &state);

    void createOptionsFile(AppState &state);

    void download_callback(AppState *state, const std::string &lineStr);
};