cmake_minimum_required(VERSION 3.24)

project(
  OnScreenKeyboard
  VERSION 0.1.0
  DESCRIPTION "Accessible on-screen keyboard for Windows"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

option(OSK_BUILD_TESTS "Build the OnScreenKeyboard test suite" ${PROJECT_IS_TOP_LEVEL})
option(OSK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(osk_project_warnings INTERFACE)
osk_set_project_warnings(osk_project_warnings ${OSK_WARNINGS_AS_ERRORS})

add_subdirectory(src/core)
add_subdirectory(src/platform)

# The WinUI 3 app shell (src/app) is intentionally not wired up yet: it needs
# concrete UI requirements and Windows App SDK project machinery that can only
# be authored and verified on Windows/Visual Studio.

if(OSK_BUILD_TESTS)
  enable_testing()
  include(CTest)
  add_subdirectory(tests)
endif()
