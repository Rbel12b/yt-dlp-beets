#include "yt-dlp.hpp"
#include <ctype.h>
#include <cstring>
#include <algorithm>
#include "Utils.hpp"
#include "python_setup.hpp"
#include <iostream>
#include <json/json.h>
#include <fstream>
#include <sstream>

void yt_dlp_utils::download(AppState &state)
{
    auto _download = state.download;
    if (!_download.start)
    {
        return;
    }

    createOptionsFile(state);

    std::string yt_dlp_cmd = PythonSetup::getPythonPath().string();

#if _WIN32
    yt_dlp_cmd = "\"" + yt_dlp_cmd + "\"";
#endif

    yt_dlp_cmd += " \"" + Utils::getBundledFilePath("yt-dlp-wrapper.py").string() + "\"";

    yt_dlp_cmd += " " + state.download.optionsFileName;

    std::cout << yt_dlp_cmd << "\n";

    state.commandInProgress.enabled = true;
    state.commandInProgress.progressDisabled = false;
    state.commandInProgress.progress = 0;
    if (state.download.playlist.noPlaylist)
    {
        state.commandInProgress.text = "Downloading";
    }
    else
    {
        state.commandInProgress.text = "Downloading playlist";
        state.download.playlist.displayedIndex = 0;
    }
    Utils::runCommandOutputCallback(yt_dlp_cmd, std::bind(download_callback, &state, std::placeholders::_1));
    state.commandInProgress.enabled = false;
}

void yt_dlp_utils::createOptionsFile(AppState &state)
{
    state.download.optionsFileName = (Utils::getTempDir() / "yt-dlp-options.json").string();
    Json::Value options;

    options["urls"] = std::string(state.download.urlBuffer);

#if _WIN32
    options["ffmpeg_location"] = Utils::getBundledExePath("ffmpeg").parent_path().string();
#endif

    if (state.download.audioOnly)
    {
        options["format"] = "bestaudio/best";
        options["outtmpl"] = state.tempAudioDir.string() + "/%(title)s [%(id)s].%(ext)s";
        options["writethumbnail"] = true;

        Json::Value pp(Json::arrayValue);

        Json::Value pp_extract_audio;
        pp_extract_audio["key"] = "FFmpegExtractAudio";
        pp_extract_audio["preferredcodec"] = "mp3";
        pp.append(pp_extract_audio);

        Json::Value pp_embed_thumb;
        pp_embed_thumb["key"] = "EmbedThumbnail";
        pp.append(pp_embed_thumb);

        Json::Value pp_metadata;
        pp_metadata["key"] = "FFmpegMetadata";
        pp.append(pp_metadata);

        options["postprocessors"] = pp;
    }
    else
    {
        options["format"] = "bestvideo+bestaudio/best";
        options["outtmpl"] = state.videoDir.string() + "/%(title)s [%(id)s].%(ext)s";
        options["writethumbnail"] = true;
        options["merge_output_format"] = "mp4";

        Json::Value pp(Json::arrayValue);

        Json::Value pp_embed_thumb;
        pp_embed_thumb["key"] = "EmbedThumbnail";
        pp.append(pp_embed_thumb);

        Json::Value pp_metadata;
        pp_metadata["key"] = "FFmpegMetadata";
        pp.append(pp_metadata);

        options["postprocessors"] = pp;
    }

    if (state.download.playlist.noPlaylist)
    {
        options["noplaylist"] = true;
    }
    else
    {
        options["yesplaylist"] = true;
        options["playlist_items"] = std::string(state.download.playlist.selectionBuffer);
    }

    std::ofstream file(state.download.optionsFileName);
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // use 2 spaces
    file << Json::writeString(writer, options);
    file.close();
}

void yt_dlp_utils::download_callback(AppState *state, const std::string &lineStr)
{
    std::cout << lineStr;
    std::stringstream line(lineStr);

    std::string type;
    line >> type;
    if (type == "DOWN")
    {
        std::string in_co, percentStr;
        line >> in_co >> percentStr;
        size_t index, count;
        float percent;

        if (in_co.find_first_of('/') == std::string::npos || in_co.find_first_of('/') + 1 > in_co.size())
        {
            std::cout << "invalid output from yt-dlp-wrapper: " << lineStr;
            return;
        }

        try
        {
            index = std::stoull(in_co.substr(0, in_co.find_first_of('/')));
            count = std::stoull(in_co.substr(in_co.find_first_of('/') + 1));
            percent = std::stof(percentStr);
        }
        catch (std::exception &e)
        {
            std::cout << "Error: " << e.what() << "\n";
            return;
        }

        if (index < 1 || count < 1)
        {
            state->commandInProgress.progress = 0;
            return;
        }

        if (!state->download.playlist.noPlaylist && state->download.playlist.displayedIndex != index)
        {
            state->commandInProgress.text = std::string("Downloading playlist item ") + std::to_string(index) + " of " + std::to_string(count);
            state->download.playlist.displayedIndex = index;
        }

        float playlistProgress = (float)(index - 1) / (float)count * 100;
        float progress = playlistProgress + (percent / count);

        state->commandInProgress.progress = progress;
    }
    else if (type == "FINI")
    {
        std::string in_co;
        line >> in_co;
        size_t index, count;

        if (in_co.find_first_of('/') == std::string::npos || in_co.find_first_of('/') + 1 > in_co.size())
        {
            std::cout << "invalid output from yt-dlp-wrapper: " << lineStr;
            return;
        }

        try
        {
            index = std::stoull(in_co.substr(0, in_co.find_first_of('/')));
            count = std::stoull(in_co.substr(in_co.find_first_of('/') + 1));
        }
        catch (std::exception &e)
        {
            std::cout << "Error: " << e.what() << "\n";
            return;
        }

        if (index < 1 || count < 1)
        {
            state->commandInProgress.progress = 0;
            return;
        }

        if (!state->download.playlist.noPlaylist && state->download.playlist.displayedIndex != (index + 1) && count >= (index + 1))
        {
            state->commandInProgress.text = std::string("Downloading playlist item ") + std::to_string(index + 1) + " of " + std::to_string(count);
            state->download.playlist.displayedIndex = (index + 1);
        }

        float playlistProgress = (float)(index) / (float)count;

        state->commandInProgress.progress = playlistProgress * 100;
    }
    else if (type == "ERRO")
    {
        std::cout << "Error [yt-dlp-wrapper]: " << lineStr;
    }
    else
    {
    }
}
