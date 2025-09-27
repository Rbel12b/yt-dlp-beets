#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "AppState.hpp"

namespace yt_dlp_utils
{
    std::vector<std::string> parseFlags(const char* flags_str, int n);
    void buildFlagsStr(char* flags_str, size_t n, const std::vector<std::string>& flags);

    void parseDownloadState(AppState& state);
    void saveFlagsFromState(AppState& state);

    void donwload(AppState& state);
};