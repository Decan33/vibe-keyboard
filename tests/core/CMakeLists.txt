add_executable(osk_core_tests
  BuiltInPanelsTest.cpp
  DispatcherTest.cpp
  DwellControllerTest.cpp
  KeyActionTest.cpp
  KeyboardLayoutTest.cpp
  KeyRepeatControllerTest.cpp
  ModifierStateTest.cpp
  NgramWordPredictorTest.cpp
  PreferencesTest.cpp
)

target_link_libraries(osk_core_tests
  PRIVATE
    osk::core
    GTest::gtest_main
    osk_project_warnings
)

gtest_discover_tests(osk_core_tests)
