#include "keyboard/platform/SystemMetricsConversion.h"

#include <gtest/gtest.h>

namespace osk::platform {
namespace {

using namespace std::chrono_literals;

TEST(ToKeyboardRepeatDelayTest, ZeroIsShortestDelay) {
  EXPECT_EQ(ToKeyboardRepeatDelay(0), 250ms);
}

TEST(ToKeyboardRepeatDelayTest, OneStepsUp) {
  EXPECT_EQ(ToKeyboardRepeatDelay(1), 500ms);
}

TEST(ToKeyboardRepeatDelayTest, TwoStepsUp) {
  EXPECT_EQ(ToKeyboardRepeatDelay(2), 750ms);
}

TEST(ToKeyboardRepeatDelayTest, ThreeIsLongestDelay) {
  EXPECT_EQ(ToKeyboardRepeatDelay(3), 1000ms);
}

TEST(ToKeyboardRepeatDelayTest, BelowMinIsClampedToZero) {
  EXPECT_EQ(ToKeyboardRepeatDelay(-1), ToKeyboardRepeatDelay(0));
  EXPECT_EQ(ToKeyboardRepeatDelay(-100), ToKeyboardRepeatDelay(0));
}

TEST(ToKeyboardRepeatDelayTest, AboveMaxIsClampedToThree) {
  EXPECT_EQ(ToKeyboardRepeatDelay(4), ToKeyboardRepeatDelay(3));
  EXPECT_EQ(ToKeyboardRepeatDelay(100), ToKeyboardRepeatDelay(3));
}

TEST(ToKeyboardRepeatIntervalTest, ZeroIsSlowestRate) {
  EXPECT_EQ(ToKeyboardRepeatInterval(0), 400ms);
}

TEST(ToKeyboardRepeatIntervalTest, ThirtyOneIsFastestRate) {
  EXPECT_EQ(ToKeyboardRepeatInterval(31), 33ms);
}

TEST(ToKeyboardRepeatIntervalTest, MidpointIsBetweenTheExtremes) {
  const auto interval = ToKeyboardRepeatInterval(15);
  EXPECT_GT(interval, ToKeyboardRepeatInterval(31));
  EXPECT_LT(interval, ToKeyboardRepeatInterval(0));
}

TEST(ToKeyboardRepeatIntervalTest, IsMonotonicallyDecreasingAsSpiValueIncreases) {
  for (int value = 0; value < 31; ++value) {
    EXPECT_GE(ToKeyboardRepeatInterval(value), ToKeyboardRepeatInterval(value + 1))
        << "value=" << value;
  }
}

TEST(ToKeyboardRepeatIntervalTest, BelowMinIsClampedToZero) {
  EXPECT_EQ(ToKeyboardRepeatInterval(-1), ToKeyboardRepeatInterval(0));
  EXPECT_EQ(ToKeyboardRepeatInterval(-100), ToKeyboardRepeatInterval(0));
}

TEST(ToKeyboardRepeatIntervalTest, AboveMaxIsClampedToThirtyOne) {
  EXPECT_EQ(ToKeyboardRepeatInterval(32), ToKeyboardRepeatInterval(31));
  EXPECT_EQ(ToKeyboardRepeatInterval(1000), ToKeyboardRepeatInterval(31));
}

}  // namespace
}  // namespace osk::platform
