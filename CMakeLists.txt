cmake_minimum_required(VERSION 3.16)

set(APP_NAME        "yt-dlp-beets")
set(APP_EXECUTABLE  "${APP_NAME}")  # name of your binary

file(READ ".version" VERSION_STRING)

project(${APP_NAME} VERSION ${VERSION_STRING} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# --- User-configurable variables ---
set(APP_ICON_PNG    "${CMAKE_SOURCE_DIR}/resources/${APP_NAME}.png")  # master PNG 1024×1024

# Output files
set(ICON_DIR          "${CMAKE_BINARY_DIR}/icons")
set(APP_ICON_ICO      "${ICON_DIR}/${APP_NAME}.ico")
set(APP_ICON_PNG_SET  "${ICON_DIR}/${APP_NAME}.png")   # placeholder for master copy
set(APP_ICON_LINUX_16 "${ICON_DIR}/${APP_NAME}-16.png")
set(APP_ICON_LINUX_32 "${ICON_DIR}/${APP_NAME}-32.png")
set(APP_ICON_LINUX_48 "${ICON_DIR}/${APP_NAME}-48.png")
set(APP_ICON_LINUX_128 "${ICON_DIR}/${APP_NAME}-128.png")
set(APP_ICON_LINUX_256 "${ICON_DIR}/${APP_NAME}-256.png")

set(DESKTOP_TEMPLATE_IN  "${CMAKE_SOURCE_DIR}/resources/${APP_NAME}.desktop.in")
set(DESKTOP_TEMPLATE_OUT "${CMAKE_BINARY_DIR}/${APP_NAME}.desktop")

set(WINDOWS_ICON_RESOURCE_IN "${CMAKE_SOURCE_DIR}/resources/${APP_NAME}.rc.in")
set(WINDOWS_ICON_RESOURCE_OUT "${CMAKE_BINARY_DIR}/${APP_NAME}.rc")

# Create icon output directory
file(MAKE_DIRECTORY ${ICON_DIR})

# --- Custom commands: generate icons ---

# 1. Copy master PNG (so it’s available for desktop entry/Icon= path)
add_custom_command(
  OUTPUT ${APP_ICON_PNG_SET}
  COMMAND ${CMAKE_COMMAND} -E copy ${APP_ICON_PNG} ${APP_ICON_PNG_SET}
  DEPENDS ${APP_ICON_PNG}
  COMMENT "Copy master icon PNG"
)

# 2. Generate PNGs for Linux sizes (using ImageMagick convert)
add_custom_command(
  OUTPUT ${APP_ICON_LINUX_16} ${APP_ICON_LINUX_32} ${APP_ICON_LINUX_48} ${APP_ICON_LINUX_128} ${APP_ICON_LINUX_256}
  COMMAND convert ${APP_ICON_PNG} -resize 16x16  ${APP_ICON_LINUX_16}
  COMMAND convert ${APP_ICON_PNG} -resize 32x32  ${APP_ICON_LINUX_32}
  COMMAND convert ${APP_ICON_PNG} -resize 48x48  ${APP_ICON_LINUX_48}
  COMMAND convert ${APP_ICON_PNG} -resize 128x128 ${APP_ICON_LINUX_128}
  COMMAND convert ${APP_ICON_PNG} -resize 256x256 ${APP_ICON_LINUX_256}
  DEPENDS ${APP_ICON_PNG}
  COMMENT "Generating Linux icon pngs"
)

# 3. Generate .ico for Windows (multi-resolution)
add_custom_command(
  OUTPUT ${APP_ICON_ICO}
  COMMAND convert ${APP_ICON_LINUX_16} ${APP_ICON_LINUX_32} ${APP_ICON_LINUX_48} ${APP_ICON_LINUX_256}
          ${APP_ICON_ICO}
  DEPENDS ${APP_ICON_LINUX_16} ${APP_ICON_LINUX_32} ${APP_ICON_LINUX_48} ${APP_ICON_LINUX_256}
  COMMENT "Generating Windows .ico"
)

# 4. Configure the .desktop template
configure_file(
  ${DESKTOP_TEMPLATE_IN}
  ${DESKTOP_TEMPLATE_OUT}
  @ONLY
)

configure_file(
  ${WINDOWS_ICON_RESOURCE_IN}
  ${WINDOWS_ICON_RESOURCE_OUT}
  @ONLY
)

# --- Add to target executable ---

if(WIN32)
  # For Windows, embed icon via resource script (assuming you have one or embed .ico)
  
  add_executable(${APP_EXECUTABLE} WIN32 ${SOURCES} ${WINDOWS_ICON_RESOURCE_OUT})
else()
  add_executable(${APP_EXECUTABLE} ${SOURCES})
endif()

# --- Ensure icon generation happens before build ---
add_custom_target(generate_icons ALL
  DEPENDS
    ${APP_ICON_PNG_SET}
    ${APP_ICON_LINUX_16}
    ${APP_ICON_LINUX_32}
    ${APP_ICON_LINUX_48}
    ${APP_ICON_LINUX_128}
    ${APP_ICON_LINUX_256}
    ${APP_ICON_ICO}
    ${DESKTOP_TEMPLATE_OUT}
)

add_dependencies(${APP_EXECUTABLE} generate_icons)

# ---- Platform-specific linker flags ----
if(MINGW)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "MinGW Release build → linking statically")
        target_link_options(${APP_EXECUTABLE} PRIVATE -static -static-libgcc -static-libstdc++)
    else()
        message(STATUS "MinGW Debug build → linking dynamically")
        # no special flags → default dynamic libstdc++ & libgcc
    endif()
elseif(UNIX AND NOT APPLE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Linux Release build → static libstdc++/libgcc")
        target_link_options(${APP_EXECUTABLE} PRIVATE -static-libstdc++ -static-libgcc)
    endif()
endif()

# --- Install rules ---
install(TARGETS ${APP_EXECUTABLE} RUNTIME DESTINATION .)

# Bundle FFmpeg + Python
if(WIN32)
    # Assume you have ffmpeg + python inside "thirdparty/windows"
    install(FILES
        ${CMAKE_SOURCE_DIR}/thirdparty/windows/ffmpeg.exe
        ${CMAKE_SOURCE_DIR}/thirdparty/windows/ffprobe.exe
        ${CMAKE_SOURCE_DIR}/thirdparty/windows/yt-dlp.exe
        DESTINATION bin
    )
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/thirdparty/windows/python/ DESTINATION python)
endif()

# --- CPack configuration ---
set(CPACK_PACKAGE_NAME "${APP_NAME}")
set(CPACK_PACKAGE_VENDOR "Rbel12b")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI wrapper for yt-dlp integrated with beets")
set(CPACK_PACKAGE_CONTACT "")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# Install all license files to a "licenses" folder inside installation dir
install(DIRECTORY "${CMAKE_SOURCE_DIR}/thirdparty/licenses/"
        DESTINATION "licenses")

if(WIN32)
    # NSIS installer
    set(CPACK_GENERATOR "NSIS")

    # Where to install
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${APP_NAME}")

    # Display name & shortcuts
    set(CPACK_NSIS_DISPLAY_NAME "${APP_NAME}")
    set(CPACK_NSIS_MODIFY_PATH ON)

    # Create a Start Menu entry and Desktop shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${APP_NAME}.lnk\\\" \\\"$INSTDIR\\\\${APP_EXECUTABLE}.exe\\\""
    )
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "${CPACK_NSIS_CREATE_ICONS_EXTRA}\nCreateShortCut \\\"$DESKTOP\\\\${APP_NAME}.lnk\\\" \\\"$INSTDIR\\\\${APP_EXECUTABLE}.exe\\\""
    )
endif()

include(CPack)
