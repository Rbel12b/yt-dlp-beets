#!/bin/bash

# This script downloads the latest binaries for yt-dlp and FFmpeg and places them in thirdparty/linux

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="$SCRIPT_DIR/../thirdparty/linux"
mkdir -p "$THIRDPARTY_DIR"

# Function to download a file
download_file() {
    local url="$1"
    local output_path="$2"
    echo "Downloading $url..."
    curl -L -o "$output_path" "$url"
}

# Download yt-dlp
YTDLP_URL="https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_linux"
YTDLP_PATH="$THIRDPARTY_DIR/yt-dlp"
download_file "$YTDLP_URL" "$YTDLP_PATH"
chmod +x "$YTDLP_PATH"

# Download FFmpeg
FFMPEG_URL="https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz"
FFMPEG_TAR_PATH="$THIRDPARTY_DIR/ffmpeg.tar.xz"
download_file "$FFMPEG_URL" "$FFMPEG_TAR_PATH"
tar -xf "$FFMPEG_TAR_PATH" -C "$THIRDPARTY_DIR"
rm "$FFMPEG_TAR_PATH"

FFMPEG_FOLDER=$(find "$THIRDPARTY_DIR" -maxdepth 1 -type d -name "ffmpeg-*")
cp "$FFMPEG_FOLDER/ffmpeg" "$THIRDPARTY_DIR/ffmpeg"
cp "$FFMPEG_FOLDER/ffprobe" "$THIRDPARTY_DIR/ffprobe"
cp "$FFMPEG_FOLDER/GPLv3.txt" "$THIRDPARTY_DIR/../licenses/LICENSE.ffmpeg.txt"
rm -rf "$FFMPEG_FOLDER"