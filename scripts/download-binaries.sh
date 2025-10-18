#!/bin/bash

# This script downloads the latest binaries for yt-dlp, FFmpeg, and Python and places them in thirdparty/windows

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="$SCRIPT_DIR/../thirdparty/windows"
THIRDPARTY_DIR_LINUX="$SCRIPT_DIR/../thirdparty/linux"
mkdir -p "$THIRDPARTY_DIR"
mkdir -p "$THIRDPARTY_DIR_LINUX"

# Function to download a file
download_file() {
    local url="$1"
    local output_path="$2"
    echo "Downloading $url..."
    curl -L -o "$output_path" "$url"
}

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
PYTHON_URL="https://www.python.org/ftp/python/3.12.10/python-3.12.10-embed-amd64.zip"
PYTHON_ZIP_PATH="$THIRDPARTY_DIR/python.zip"
download_file "$PYTHON_URL" "$PYTHON_ZIP_PATH"
unzip -o "$PYTHON_ZIP_PATH" -d "$THIRDPARTY_DIR/python"
rm "$PYTHON_ZIP_PATH"
cp "$THIRDPARTY_DIR/python/LICENSE.txt" "$THIRDPARTY_DIR/../licenses/LICENSE.python.txt"
cd "$THIRDPARTY_DIR/python"

# edit python313._pth
sed -i 's/^#\s*import site/import site/' python312._pth

# --- Download Chromaprint (fpcalc)

CHROMAPRINT_ZIP_DIR_LINUX="$THIRDPARTY_DIR_LINUX"
CHROMAPRINT_ZIP_DIR_WIN="$THIRDPARTY_DIR"

CHROMAPRINT_REPO="https://github.com/acoustid/chromaprint"
RELEASE_API="$CHROMAPRINT_REPO/releases/latest"

CHROMAPRINT_TAG="1.6.0"

BASE_URL="$CHROMAPRINT_REPO/releases/download/v$CHROMAPRINT_TAG"

# Files to fetch
LINUX_FILE="chromaprint-fpcalc-$CHROMAPRINT_TAG-linux-x86_64.tar.gz"
WIN_FILE_="chromaprint-fpcalc-$CHROMAPRINT_TAG-windows-x86_64"
WIN_FILE="$WIN_FILE_.zip"

# Download and extract Linux version
echo "Downloading Linux fpcalc..."
echo "$BASE_URL/$LINUX_FILE"
curl -L -o "$CHROMAPRINT_ZIP_DIR_LINUX/$LINUX_FILE" "$BASE_URL/$LINUX_FILE"
tar -xzf "$CHROMAPRINT_ZIP_DIR_LINUX/$LINUX_FILE" -C "$CHROMAPRINT_ZIP_DIR_LINUX" --strip-components=1 || true

# Download and extract Windows version
echo "Downloading Windows fpcalc..."
curl -L -o "$CHROMAPRINT_ZIP_DIR_WIN/$WIN_FILE" "$BASE_URL/$WIN_FILE"
unzip -o "$CHROMAPRINT_ZIP_DIR_WIN/$WIN_FILE" -d "$CHROMAPRINT_ZIP_DIR_WIN" >/dev/null

mv "$CHROMAPRINT_ZIP_DIR_WIN/$WIN_FILE_/fpcalc.exe" "$CHROMAPRINT_ZIP_DIR_WIN/fpcalc.exe" || true
rm -rf "$CHROMAPRINT_ZIP_DIR_WIN/$WIN_FILE_"

# Make Linux binary executable
chmod +x "$CHROMAPRINT_ZIP_DIR_LINUX/fpcalc" || true

# Clean up archives
rm -f "$CHROMAPRINT_ZIP_DIR_LINUX/$LINUX_FILE" "$CHROMAPRINT_ZIP_DIR_WIN/$WIN_FILE"