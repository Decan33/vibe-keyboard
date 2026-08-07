add_library(osk_core
  src/BuiltInPanels.cpp
  src/Dispatcher.cpp
  src/DwellController.cpp
  src/KeyboardLayout.cpp
  src/KeyRepeatController.cpp
  src/ModifierState.cpp
  src/NgramWordPredictor.cpp
  src/Preferences.cpp
)
add_library(osk::core ALIAS osk_core)

target_include_directories(osk_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(osk_core PRIVATE osk_project_warnings)
