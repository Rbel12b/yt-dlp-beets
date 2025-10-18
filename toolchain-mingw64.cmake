set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(tools /usr/bin)
set(CMAKE_C_COMPILER ${tools}/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER ${tools}/x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER ${tools}/x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH
    /usr/x86_64-w64-mingw32
    /usr/local/x86_64-w64-mingw32
)
set(CMAKE_PREFIX_PATH 
    /usr/x86_64-w64-mingw32
    /usr/local/x86_64-w64-mingw32
)

# Adjust search behavior so cmake looks in the cross-compiler sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
