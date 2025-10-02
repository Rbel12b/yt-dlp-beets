#include "yt-dlp.hpp"
#include <ctype.h>
#include <cstring>
#include <algorithm>
#include "Utils.hpp"
#include "python_setup.hpp"
#include <iostream>
#include <json/json.h>
#include <fstream>

void yt_dlp_utils::donwload(AppState &state)
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

    yt_dlp_cmd += " \"" + Utils::getBundledFilePath("../resources/yt-dlp-wrapper.py").string() + "\"";

    yt_dlp_cmd += " " + state.download.optionsFileName;

    std::cout << yt_dlp_cmd << "\n";

    if (!Utils::runCommand(yt_dlp_cmd))
    {
        return;
    }
}

void yt_dlp_utils::createOptionsFile(AppState &state)
{
    state.download.optionsFileName = (Utils::getTempDir() / "yt-dlp-options.json").string();
    Json::Value options;

    options["urls"] = std::string(state.download.urlBuffer);

#if _WIN32
    options["ffmpeg_location"] = "\"" + Utils::getBundledExePath("ffmpeg").parent_path().string() + "\"";
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

    std::ofstream file(state.download.optionsFileName);
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // use 2 spaces
    file << Json::writeString(writer, options);
    file.close();
}
