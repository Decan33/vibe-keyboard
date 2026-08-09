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

# src/app (the WinUI 3 shell) is intentionally NOT add_subdirectory'd here.
# WinUI 3's XAML/MSIX toolchain is MSBuild-only -- it can't be driven from
# CMake. Rather than have it link the osk_core/osk_platform static libs
# built by this CMake invocation (real CRT/ABI mismatch risk between a
# CMake-invoked MSVC and an MSBuild-invoked one), src/app's own .vcxproj
# compiles the same osk::core/osk::platform .cpp/.h files directly. Build it
# via `msbuild src/app/OnScreenKeyboardApp.sln`, not through this build.

if(OSK_BUILD_TESTS)
  enable_testing()
  include(CTest)
  add_subdirectory(tests)
endif()
