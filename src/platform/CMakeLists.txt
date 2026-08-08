find_package(wil CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(osk_platform
  src/PlatformLibrary.cpp
  src/Win32AlwaysOnTopController.cpp
  src/Win32InputInjector.cpp
  src/Win32PreferencesStore.cpp
  src/Win32SystemMetrics.cpp
  src/Win32WindowTransparency.cpp
)
add_library(osk::platform ALIAS osk_platform)

target_include_directories(osk_platform
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# WIN32_LEAN_AND_MEAN / NOMINMAX are load-bearing, not just hygiene: without
# NOMINMAX, windows.h's min/max macros silently break any std::min/std::max
# call textually after the include.
target_compile_definitions(osk_platform
  PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(osk_platform
  PUBLIC
    osk::core
    # WIL is PUBLIC (not PRIVATE): Win32AlwaysOnTopController.h, a public
    # header, includes <wil/resource.h> for wil::unique_hwineventhook, so
    # consumers of osk::platform need its include path too.
    WIL::WIL
  PRIVATE
    osk_project_warnings
    nlohmann_json::nlohmann_json
    User32
    Shell32
    Ole32
)
