#include "yt-dlp.hpp"
#include <ctype.h>
#include <cstring>
#include <algorithm>
#include "Utils.hpp"
#include <iostream>

std::vector<std::string> yt_dlp_utils::parseFlags(const char *flags_str, int n)
{
    std::vector<std::string> flags;
    std::string current = "";
    for (int i = 0; flags_str[i] != '\0' && i < n; i++)
    {
        if (isspace(flags_str[i]))
        {
            flags.push_back(current);
            current = "";
            continue;
        }
        current += flags_str[i];
    }
    if (current.length())
    {
        flags.push_back(current);
        current = "";
    }
    return flags;
}

void yt_dlp_utils::buildFlagsStr(char *flags_str, size_t n, const std::vector<std::string> &flags)
{
    for (int i = 0; i < n; i++)
    {
        flags_str[i] = '\0';
    }
    bool first = true;
    for (auto flag : flags)
    {
        if (flag.empty())
        {
            continue;
        }
        if (!first)
        {
            strncat(flags_str, " ", n);
        }
        strncat(flags_str, flag.c_str(), n);
        first = false;
    }
}

void yt_dlp_utils::parseDownloadState(AppState &state)
{
    auto flags = parseFlags(state.download.flagsBuffer, sizeof(state.download.flagsBuffer));
    bool nextPlaylistSelection = false;
    int i = 0;
_restart:
    for (auto flag : flags)
    {
        if (flag == "--no-playlist")
        {
            state.download.playlist.noPlaylist = true;
        }
        else if (flag == "--yes-playlist")
        {
            state.download.playlist.noPlaylist = false;
        }
        else if (flag.substr(0, 2) == "-I" || flag == "--playlist-items")
        {
            nextPlaylistSelection = true;
            if (flag.substr(0, 2) == "-I" && flag.length() > 2)
            {
                strncpy(state.download.playlist.selectionBuffer, flag.substr(2).c_str(),
                        sizeof(state.download.playlist.selectionBuffer));
                nextPlaylistSelection = false;
                flags[i] = "-I";
                flags.insert(flags.begin() + i + 1, std::string(state.download.playlist.selectionBuffer));
                goto _restart;
            }
        }
        else if (nextPlaylistSelection)
        {
            strncpy(state.download.playlist.selectionBuffer, flag.c_str(),
                    sizeof(state.download.playlist.selectionBuffer));
            nextPlaylistSelection = false;
        }
        i++;
    }
    state.download.playlist.flags = flags;
}

void yt_dlp_utils::saveFlagsFromState(AppState &state)
{
    auto playlist = state.download.playlist;
    while (std::find(playlist.flags.begin(), playlist.flags.end(), std::string("--yes-playlist")) != playlist.flags.end())
    {
        playlist.flags.erase(std::find(playlist.flags.begin(), playlist.flags.end(), std::string("--yes-playlist")));
    }
    while (std::find(playlist.flags.begin(), playlist.flags.end(), std::string("--no-playlist")) != playlist.flags.end())
    {
        playlist.flags.erase(std::find(playlist.flags.begin(), playlist.flags.end(), std::string("--no-playlist")));
    }

    if (playlist.noPlaylist)
    {
        playlist.flags.push_back("--no-playlist");
    }
    else
    {
        playlist.flags.push_back("--yes-playlist");
    }

    auto iter1 = std::find(playlist.flags.begin(), playlist.flags.end(), std::string("-I"));
    auto iter2 = std::find(playlist.flags.begin(), playlist.flags.end(), std::string("--playlist-items"));

    if (iter1 != playlist.flags.end() && (iter1 + 1) != playlist.flags.end())
    {
        *(iter1 + 1) = playlist.selectionBuffer;
    }
    else if (iter2 != playlist.flags.end() && (iter2 + 1) != playlist.flags.end())
    {
        *(iter2 + 1) = playlist.selectionBuffer;
    }
    else if (strlen(playlist.selectionBuffer))
    {
        playlist.flags.push_back("-I");
        playlist.flags.push_back(playlist.selectionBuffer);
    }

    buildFlagsStr(state.download.flagsBuffer, sizeof(state.download.flagsBuffer) - 1, playlist.flags);
}

void yt_dlp_utils::donwload(AppState &state)
{
    auto _download = state.download;
    if (!_download.start)
    {
        return;
    }

    std::string yt_dlp_cmd = "\"" + Utils::getBundledExePath("yt-dlp").string() + "\"";

    yt_dlp_cmd += " " + std::string(state.download.flagsBuffer);
    
    if (_download.audioOnly)
    {
        yt_dlp_cmd += " --extract-audio --audio-format mp3 --embed-thumbnail --add-metadata"
            " --metadata-from-title \"%(artist)s - %(title)s\" -o \""
            + (state.tempAudioDir / "%(title)s [%(id)s].%(ext)s").string() + "\"";
    }
    else
    {
        yt_dlp_cmd += " \"" + (state.videoDir / "%(title)s [%(id)s].%(ext)s").string() + "\"";
    }


    yt_dlp_cmd += " \"" + std::string(_download.urlBuffer) + "\"";

    std::cout << yt_dlp_cmd << "\n";

    if (!Utils::runInteractiveTerminal(yt_dlp_cmd))
    {
        return;
    }
}
