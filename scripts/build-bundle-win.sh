#!/bin/bash

cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake
cmake --build build-win --config Release
cpack --config build-win/CPackConfig.cmake