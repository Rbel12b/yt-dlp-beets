#!/bin/bash

# This script downloads the latest binaries for yt-dlp, FFmpeg, and Python and places them in thirdparty/windows

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="$SCRIPT_DIR/../thirdparty/windows"
mkdir -p "$THIRDPARTY_DIR"

# Function to download a file
download_file() {
    local url="$1"
    local output_path="$2"
    echo "Downloading $url..."
    curl -L -o "$output_path" "$url"
}

# Download yt-dlp
YTDLP_URL="https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe"
YTDLP_PATH="$THIRDPARTY_DIR/yt-dlp.exe"
download_file "$YTDLP_URL" "$YTDLP_PATH"

# Download FFmpeg
FFMPEG_URL="https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip"
FFMPEG_ZIP_PATH="$THIRDPARTY_DIR/ffmpeg.zip"

FFMPEG_FOLDER=$(find "$THIRDPARTY_DIR" -maxdepth 1 -type d -name "ffmpeg-*")

if [ -d "$FFMPEG_FOLDER" ]; then
    echo "FFmpeg already downloaded."
else
    download_file "$FFMPEG_URL" "$FFMPEG_ZIP_PATH"
    unzip -o "$FFMPEG_ZIP_PATH" -d "$THIRDPARTY_DIR"
    rm "$FFMPEG_ZIP_PATH"
fi

FFMPEG_FOLDER=$(find "$THIRDPARTY_DIR" -maxdepth 1 -type d -name "ffmpeg-*")

# Move ffmpeg.exe to the thirdparty/windows directory
cp "$FFMPEG_FOLDER/bin/ffmpeg.exe" "$THIRDPARTY_DIR/ffmpeg.exe"
cp "$FFMPEG_FOLDER/bin/ffprobe.exe" "$THIRDPARTY_DIR/ffprobe.exe"
cp "$FFMPEG_FOLDER/LICENSE" "$THIRDPARTY_DIR/../licenses/LICENSE.ffmpeg.txt"
rm -rf "$FFMPEG_FOLDER"

# Download Python
PYTHON_URL="https://www.python.org/ftp/python/3.13.7/python-3.13.7-embed-amd64.zip"
PYTHON_ZIP_PATH="$THIRDPARTY_DIR/python.zip"
download_file "$PYTHON_URL" "$PYTHON_ZIP_PATH"
unzip -o "$PYTHON_ZIP_PATH" -d "$THIRDPARTY_DIR/python"
rm "$PYTHON_ZIP_PATH"
cp "$THIRDPARTY_DIR/python/LICENSE.txt" "$THIRDPARTY_DIR/../licenses/LICENSE.python.txt"
cd "$THIRDPARTY_DIR/python"

# edit python313._pth
sed -i 's/^#\s*import site/import site/' python313._pth