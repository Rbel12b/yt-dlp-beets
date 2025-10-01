cmake_minimum_required(VERSION 3.16)

set(APP_NAME        "yt-dlp-beets")
set(APP_EXECUTABLE  "${APP_NAME}")
set(VERSION_FILE    "yt-dlp-beets.version")
set(VERSION_FILE_OUT    "${CMAKE_BINARY_DIR}/version.def")

file(READ ${VERSION_FILE} VERSION_STRING)

project(${APP_NAME} VERSION ${VERSION_STRING} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


set(BEETS_CONFIG_IN "../resources/beets.config.yaml")
set(BEETS_CONFIG_OUT "${CMAKE_BINARY_DIR}/beets.config.yaml.str")

set(PROJECT_LICENSES_COMBINED "${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSES_COMBINED.txt")

# --- Find dependencies ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# --- Add to target executable ---
if(WIN32)
  add_executable(${APP_EXECUTABLE} ${SOURCES} ${WINDOWS_ICON_RESOURCE_OUT})
else()
  add_executable(${APP_EXECUTABLE} ${SOURCES})
endif()

target_include_directories(${APP_EXECUTABLE} PRIVATE src ${CMAKE_SOURCE_DIR}/thirdparty/lib/include ${CMAKE_BINARY_DIR})

add_custom_command(
  OUTPUT ${BEETS_CONFIG_OUT}
  COMMAND xxd -i "${BEETS_CONFIG_IN}" > ${BEETS_CONFIG_OUT}
  COMMENT "Generating beets default config"
  DEPENDS ${BEETS_CONFIG_IN}
)

add_custom_command(
  OUTPUT ${VERSION_FILE_OUT}
  COMMAND xxd -i "../${VERSION_FILE}" > ${VERSION_FILE_OUT}
  COMMENT "Generating version.def"
  DEPENDS ${VERSION_FILE}
)

add_custom_target(generate_embedded_files ALL
  DEPENDS ${BEETS_CONFIG_OUT} ${VERSION_FILE_OUT}
)

add_dependencies(${APP_EXECUTABLE} generate_embedded_files)

# --- Libraries ---

set(IMGUI_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/lib/imgui")
set(DearImGui_Backend_SDLRenderer2 ON)
set(DearImGui_Backend_SDL2 ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/lib/CMakeForImGui EXCLUDE_FROM_ALL)

find_package(SDL2 CONFIG REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(${APP_EXECUTABLE} PRIVATE imgui_core imgui_impl_sdl2 imgui_impl_sdlrenderer2 SDL2::SDL2 CURL::libcurl)

# --- Custom commands: generate icons ---

# Create icon output directory
file(MAKE_DIRECTORY ${ICON_DIR})

# 1. Copy master PNG (so it’s available for desktop entry/Icon= path)
add_custom_command(
  OUTPUT ${APP_ICON_PNG_SET}
  COMMAND ${CMAKE_COMMAND} -E copy ${APP_ICON_PNG} ${APP_ICON_PNG_SET}
  DEPENDS ${APP_ICON_PNG}
  COMMENT "Copy master icon PNG"
)

# 2. Generate PNGs for Linux sizes
add_custom_command(
  OUTPUT ${APP_ICON_LINUX_16} ${APP_ICON_LINUX_32} ${APP_ICON_LINUX_48} ${APP_ICON_LINUX_128} ${APP_ICON_LINUX_256}
  COMMAND magick ${APP_ICON_PNG} -resize 16x16  ${APP_ICON_LINUX_16}
  COMMAND magick ${APP_ICON_PNG} -resize 32x32  ${APP_ICON_LINUX_32}
  COMMAND magick ${APP_ICON_PNG} -resize 48x48  ${APP_ICON_LINUX_48}
  COMMAND magick ${APP_ICON_PNG} -resize 128x128 ${APP_ICON_LINUX_128}
  COMMAND magick ${APP_ICON_PNG} -resize 256x256 ${APP_ICON_LINUX_256}
  DEPENDS ${APP_ICON_PNG}
  COMMENT "Generating Linux icon pngs"
)

# 3. Generate .ico for Windows (multi-resolution)
add_custom_command(
  OUTPUT ${APP_ICON_ICO}
  COMMAND magick ${APP_ICON_LINUX_16} ${APP_ICON_LINUX_32} ${APP_ICON_LINUX_48} ${APP_ICON_LINUX_256}
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

add_custom_command(
  OUTPUT ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "yt-dlp-beets License" > ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "===================" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND cat ${CMAKE_SOURCE_DIR}/LICENSE >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "Third-party licenses" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "====================" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "ffmpeg License:" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "-------------" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND cat ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.ffmpeg.txt >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "yt-dlp License:" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "------------" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND cat ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.yt-dlp.txt >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "python License:" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "-------------" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND echo "" >> ${PROJECT_LICENSES_COMBINED}
  COMMAND cat ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.python.txt >> ${PROJECT_LICENSES_COMBINED}
  DEPENDS
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.ffmpeg.txt
    ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.yt-dlp.txt
    ${CMAKE_SOURCE_DIR}/thirdparty/licenses/LICENSE.python.txt
  COMMENT "Combining license files"
)

add_custom_target(generate_licenses ALL
  DEPENDS ${PROJECT_LICENSES_COMBINED}
)

add_dependencies(${APP_EXECUTABLE} generate_licenses)

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

    set(LDD_EXECUTABLE "mingw-ldd")

    install(CODE "
        message(STATUS \"Scanning dependencies for: ${CMAKE_BINARY_DIR}/${APP_EXECUTABLE}.exe\")
        execute_process(
            COMMAND \"${LDD_EXECUTABLE}\" --dll-lookup-dir /usr/x86_64-w64-mingw32/bin -- \"${CMAKE_BINARY_DIR}/${APP_EXECUTABLE}.exe\"
            OUTPUT_VARIABLE ldd_out
            RESULT_VARIABLE ldd_res
        )
        if (NOT \${ldd_res} EQUAL 0)
            message(FATAL_ERROR \"ntldd failed: \${ldd_res}\")
        endif()

        # Parse ntldd output: each line looks like 'libcurl-4.dll => /usr/.../libcurl-4.dll'
        string(REPLACE \"\\n\" \";\" ldd_lines \"\${ldd_out}\")
        foreach(line \${ldd_lines})
            if (line MATCHES \".*=>[ ]+([^ ]+)\")
                set(dll \"\${CMAKE_MATCH_1}\")
                if (EXISTS \"\${dll}\")
                    file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"\${dll}\")
                    message(STATUS \"Installed runtime DLL: \${dll}\")
                endif()
            endif()
        endforeach()
    ")
endif()

# --- CPack configuration ---
set(CPACK_PACKAGE_NAME "${APP_NAME}")
set(CPACK_PACKAGE_VENDOR "Rbel12b")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI wrapper for yt-dlp integrated with beets")
set(CPACK_PACKAGE_CONTACT "")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_LICENSES_COMBINED}")
set(CPACK_THREADS 0)

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
