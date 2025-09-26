#!/usr/bin/env bash
set -e

APP=yt-dlp-beets
VERSION=$(cat .version)
BUILD_DIR=build
APPDIR=$BUILD_DIR/AppDir

# 1. Configure + build
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR"

# 2. Generate icons and desktop file via CMake
cmake --build "$BUILD_DIR" --target generate_icons

# 3. Create AppDir structure
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# 4. Copy built binaries
cp "$BUILD_DIR/$APP" "$APPDIR/usr/bin/"
chmod +x "$APPDIR/usr/bin/$APP"

# 5. Copy third-party Linux executables
cp "thirdparty/linux/"* "$APPDIR/usr/bin/"
chmod +x "$APPDIR/usr/bin/"*

# 6. Copy generated 256x256 icon to AppDir
ICON_SRC="$BUILD_DIR/icons/${APP}-256.png"
ICON_DST="$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP.png"
cp "$ICON_SRC" "$ICON_DST"

# 7. Copy generated .desktop file
DESKTOP_SRC="$BUILD_DIR/${APP}.desktop"
DESKTOP_DST="$APPDIR/usr/share/applications/${APP}.desktop"
cp "$DESKTOP_SRC" "$DESKTOP_DST"

# 8. Run linuxdeploy + appimagetool
linuxdeploy --appdir "$APPDIR" \
  -d "$DESKTOP_DST" \
  -i "$ICON_DST" \
  --output appimage
