#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "AppState.hpp"

namespace yt_dlp_utils
{
    void donwload(AppState &state);

    void createOptionsFile(AppState &state);

    void donwload_callback(AppState *state, const std::string &lineStr);
};