# Cross-compiling for 64-bit Windows with MinGW-w64 (Homebrew's mingw-w64, Debian's
# gcc-mingw-w64-x86-64):
#
#   cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake \
#         -DCMAKE_PREFIX_PATH=/path/to/SDL3-3.2.24/x86_64-w64-mingw32
#   cmake --build build-win --target mapedit
#
# SDL3's MinGW development package is SDL3-devel-<version>-mingw.tar.gz from SDL's releases.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
