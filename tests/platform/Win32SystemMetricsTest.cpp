#include "keyboard/platform/Win32SystemMetrics.h"

#include <gtest/gtest.h>

namespace osk::platform {
namespace {

using namespace std::chrono_literals;

// These are thin smoke tests: the live OS-configured keyboard repeat
// delay/speed can't be controlled from a test, so we only assert the result
// falls within the range ToKeyboardRepeatDelay/ToKeyboardRepeatInterval can
// ever produce (see SystemMetricsConversionTest.cpp for the exhaustively
// tested value math).

TEST(Win32SystemMetricsTest, GetKeyboardRepeatDelayIsWithinValidRange) {
  const Win32SystemMetrics metrics;
  const auto delay = metrics.GetKeyboardRepeatDelay();
  EXPECT_GE(delay, 250ms);
  EXPECT_LE(delay, 1000ms);
}

TEST(Win32SystemMetricsTest, GetKeyboardRepeatIntervalIsWithinValidRange) {
  const Win32SystemMetrics metrics;
  const auto interval = metrics.GetKeyboardRepeatInterval();
  EXPECT_GE(interval, 33ms);
  EXPECT_LE(interval, 400ms);
}

}  // namespace
}  // namespace osk::platform
