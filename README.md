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

##Dependencies
Via MSYS2:
mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-cppcheck

Via CMake - Downloaded at config-time and verified with SHA-256 hash
Dear ImGUI (1.92.9)
GLFW (3.5.1)
Catch2 (3.16.0)

##Structure
src/core/   calculation engine
src/gui/    ImGui front end
tests/      Catch2 tests, named by requirement ID
cmake/      pinned dependencies
scripts/    environment check

##VSCode
Run tasks from command palette:
  1. Check environment
  2. Debug build
  3. Release Build


