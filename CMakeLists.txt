cmake_minimum_required(VERSION 3.25)
project(AFRM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(AFRM_STATIC_RUNTIME "Link the MinGW runtime statically)" ON)
option(AFRM_BUILD_GUI "Build the ImGui desktop application" ON)
option(AFRM_SANITIZE "Build with AddressSanitizer and UBSan (Linux/GCC)" OFF)

# Drop unused code and data from final binary
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-ffunction-sections -fdata-sections)
  add_link_options(-Wl,--gc-sections)
endif()

# MinGW static link
if(MINGW AND AFRM_STATIC_RUNTIME)
  add_link_options(-static -static-libgcc -static-libstdc++)
endif()

# Strict warnings, for app code
add_library(afrm_options INTERFACE)
if(MSVC)
  target_compile_options(afrm_options INTERFACE /W4 /WX /permissive-)
else()
  target_compile_options(afrm_options INTERFACE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()
if(WIN32)
  # Windows 10 API level - also correct for Windows 11.
  target_compile_definitions(afrm_options INTERFACE _WIN32_WINNT=0x0A00)
endif()
if(AFRM_SANITIZE)
  target_compile_options(afrm_options INTERFACE
    -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(afrm_options INTERFACE -fsanitize=address,undefined)
endif()

include(cmake/Dependencies.cmake)

# Calculation core - plain C++ with no GUI 
add_library(afrm_core STATIC src/core/version.cpp)
target_include_directories(afrm_core PUBLIC src)
target_compile_definitions(afrm_core PUBLIC AFRM_VERSION="${PROJECT_VERSION}")
target_link_libraries(afrm_core PRIVATE afrm_options)

if(AFRM_BUILD_GUI)
  include(cmake/GuiDependencies.cmake)
  add_executable(afrm src/gui/main.cpp)
  target_link_libraries(afrm PRIVATE afrm_core afrm_options imgui)
endif()

enable_testing()
add_subdirectory(tests)
