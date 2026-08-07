add_executable(osk_platform_tests
  PlatformMocksTest.cpp
)

target_include_directories(osk_platform_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(osk_platform_tests
  PRIVATE
    osk::platform
    GTest::gmock_main
    osk_project_warnings
)

gtest_discover_tests(osk_platform_tests)
