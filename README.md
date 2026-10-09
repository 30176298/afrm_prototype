# AFRM – Aircraft Fuel and Range Management (prototype)

Graphical desktop application for the evaluation of flight planning with regards to fuel feasibility.

##Toolchain
C++17
GCC (MSYS2) on development - MSVC and Linux GCC in CI
CMAKE (presets) + Ninja
VS Code + CMake Tools, C/C++ Extensions
Catch2 v3 through CTest
Compiler warnings -Wall -Wextra -Wpedantic -Wconversion -Wshadow, cppcheck
Targetting Windows 10+

##Additional
mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-cppcheck
needed in MSYS2

cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug
