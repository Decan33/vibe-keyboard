#include "keyboard/core/KeyRepeatController.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

using namespace std::chrono_literals;

constexpr auto kEpoch = std::chrono::steady_clock::time_point{};

TEST(KeyRepeatControllerTest, UpdateWithNoKeyDownReturnsNullopt) {
  KeyRepeatController controller(300ms, 50ms);
  EXPECT_FALSE(controller.Update(kEpoch).has_value());
}

TEST(KeyRepeatControllerTest, NoRepeatBeforeInitialDelayElapses) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);

  EXPECT_FALSE(controller.Update(kEpoch + 100ms).has_value());
  EXPECT_FALSE(controller.Update(kEpoch + 299ms).has_value());
}

TEST(KeyRepeatControllerTest, FiresOnceInitialDelayElapses) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);

  const auto fired = controller.Update(kEpoch + 300ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "a");
}

TEST(KeyRepeatControllerTest, FiresAgainAfterEachRepeatInterval) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);

  ASSERT_TRUE(controller.Update(kEpoch + 300ms).has_value());
  EXPECT_FALSE(controller.Update(kEpoch + 320ms).has_value());
  EXPECT_TRUE(controller.Update(kEpoch + 350ms).has_value());
  EXPECT_FALSE(controller.Update(kEpoch + 370ms).has_value());
  EXPECT_TRUE(controller.Update(kEpoch + 400ms).has_value());
}

TEST(KeyRepeatControllerTest, FiresAtMostOnceEvenIfMultipleIntervalsElapsed) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);

  const auto fired = controller.Update(kEpoch + 10s);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "a");

  // Next fire is scheduled relative to the call above, not the original
  // schedule, so it should not immediately fire again.
  EXPECT_FALSE(controller.Update(kEpoch + 10s + 10ms).has_value());
}

TEST(KeyRepeatControllerTest, OnKeyUpStopsRepeating) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);
  controller.OnKeyUp("a");

  EXPECT_FALSE(controller.Update(kEpoch + 1s).has_value());
}

TEST(KeyRepeatControllerTest, OnKeyUpForNonHeldKeyIsNoop) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);
  controller.OnKeyUp("b");

  EXPECT_TRUE(controller.Update(kEpoch + 300ms).has_value());
}

TEST(KeyRepeatControllerTest, SecondKeyDownReplacesFirstHeldKey) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);
  controller.OnKeyDown("b", kEpoch + 100ms);

  // "a"'s original schedule (would have fired at kEpoch + 300ms) is gone;
  // "b" now owns the held slot with its own delay from kEpoch + 100ms.
  EXPECT_FALSE(controller.Update(kEpoch + 300ms).has_value());
  const auto fired = controller.Update(kEpoch + 400ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "b");
}

TEST(KeyRepeatControllerTest, OnKeyUpAfterBeingSupersededDoesNotAffectNewHeldKey) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);
  controller.OnKeyDown("b", kEpoch + 100ms);
  controller.OnKeyUp("a");

  EXPECT_TRUE(controller.Update(kEpoch + 400ms).has_value());
}

TEST(KeyRepeatControllerTest, RepeatedOnKeyDownForSameKeyRestartsInitialDelay) {
  KeyRepeatController controller(300ms, 50ms);
  controller.OnKeyDown("a", kEpoch);
  controller.OnKeyDown("a", kEpoch + 200ms);

  EXPECT_FALSE(controller.Update(kEpoch + 300ms).has_value());
  EXPECT_TRUE(controller.Update(kEpoch + 500ms).has_value());
}

TEST(KeyRepeatControllerTest, ZeroInitialDelayFiresImmediately) {
  KeyRepeatController controller(0ms, 50ms);
  controller.OnKeyDown("a", kEpoch);

  EXPECT_TRUE(controller.Update(kEpoch).has_value());
}

TEST(KeyRepeatControllerTest, ZeroRepeatIntervalFiresOnEveryTick) {
  KeyRepeatController controller(0ms, 0ms);
  controller.OnKeyDown("a", kEpoch);

  EXPECT_TRUE(controller.Update(kEpoch).has_value());
  EXPECT_TRUE(controller.Update(kEpoch).has_value());
  EXPECT_TRUE(controller.Update(kEpoch + 1ms).has_value());
}

TEST(KeyRepeatControllerTest, NegativeDelaysAreClampedToZero) {
  KeyRepeatController controller(-100ms, -50ms);
  controller.OnKeyDown("a", kEpoch);

  EXPECT_TRUE(controller.Update(kEpoch).has_value());
}

}  // namespace
}  // namespace osk::core
