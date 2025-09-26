cmake_minimum_required(VERSION 3.16)
file(READ ".version" VERSION_STRING)
project(yt-dlp-beets VERSION ${VERSION_STRING} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    # Statically link libgcc + libstdc++
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static -static-libgcc -static-libstdc++"
    )
elseif(UNIX AND NOT APPLE)
    # Statically link libstdc++ and libgcc, keep glibc dynamic
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static-libstdc++ -static-libgcc"
    )
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(yt-dlp-beets ${SOURCES})

# --- Install rules ---
install(TARGETS yt-dlp-beets RUNTIME DESTINATION .)

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
set(CPACK_PACKAGE_NAME "yt-dlp-beets")
set(CPACK_PACKAGE_VENDOR "Rbel12b")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI wrapper for yt-dlp integrated with beets")
set(CPACK_PACKAGE_CONTACT "")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# Show a combined NOTICE as license page
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/thirdparty/licenses/NOTICE.txt")

# Install all license files to a "licenses" folder inside installation dir
install(DIRECTORY "${CMAKE_SOURCE_DIR}/thirdparty/licenses/"
        DESTINATION "licenses")

if(WIN32)
    # NSIS installer
    set(CPACK_GENERATOR "NSIS")

    # Where to install
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "yt-dlp-beets")

    # Display name & shortcuts
    set(CPACK_NSIS_DISPLAY_NAME "yt-dlp-beets")
    set(CPACK_NSIS_MODIFY_PATH ON)

    # Create a Start Menu entry and Desktop shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\yt-dlp-beets.lnk\\\" \\\"$INSTDIR\\\\yt-dlp-beets.exe\\\""
    )
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "${CPACK_NSIS_CREATE_ICONS_EXTRA}\nCreateShortCut \\\"$DESKTOP\\\\yt-dlp-beets.lnk\\\" \\\"$INSTDIR\\\\yt-dlp-beets.exe\\\""
    )
endif()

include(CPack)
