#!/usr/bin/env bash
set -e

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

APP=yt-dlp-beets
VERSION=$(cat .version)
BUILD_DIR=build
APPDIR=$BUILD_DIR/AppDir

mkdir -p "$APPDIR/usr/bin"

# Install app files into AppDir
cmake --install "$BUILD_DIR" --prefix "$APPDIR/usr"

# Add desktop entry + icon (needed for AppImage)
mkdir -p "$APPDIR/usr/share/applications"
cat > "$APPDIR/usr/share/applications/$APP.desktop" <<EOF
[Desktop Entry]
Name=yt-dlp-beets
Exec=$APP
Icon=$APP
Type=Application
Categories=Utility;
EOF

# Dummy icon
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp $PWD/icon.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP.png"

# Run linuxdeploy + appimagetool
linuxdeploy --appdir "$APPDIR" \
  -d "$APPDIR/usr/share/applications/$APP.desktop" \
  -i "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP.png" \
  --output appimage
