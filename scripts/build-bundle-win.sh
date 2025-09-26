#!/bin/bash

cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win --config Release

# Create installer using CPack if there is no arguments give to the script
if [ "$#" -ne 0 ]; then
    echo "Skipping installer creation as arguments were provided to the script."
    exit 0
fi

cpack --config build-win/CPackConfig.cmake