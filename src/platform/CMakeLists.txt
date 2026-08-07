add_library(osk_platform
  src/PlatformLibrary.cpp
)
add_library(osk::platform ALIAS osk_platform)

target_include_directories(osk_platform
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(osk_platform
  PUBLIC
    osk::core
  PRIVATE
    osk_project_warnings
)
