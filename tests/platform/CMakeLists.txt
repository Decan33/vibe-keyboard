add_executable(osk_platform_tests
  PlatformMocksTest.cpp
  VirtualKeyMappingTest.cpp
  Win32WindowFixture.cpp
  Win32WindowTransparencyTest.cpp
  SystemMetricsConversionTest.cpp
  Win32SystemMetricsTest.cpp
  Win32PreferencesStoreTest.cpp
  BuildInputEventsTest.cpp
  Win32InputInjectorTest.cpp
  Win32AlwaysOnTopControllerTest.cpp
)

target_include_directories(osk_platform_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(osk_platform_tests
  PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(osk_platform_tests
  PRIVATE
    osk::platform
    GTest::gmock_main
    osk_project_warnings
)

gtest_discover_tests(osk_platform_tests)
