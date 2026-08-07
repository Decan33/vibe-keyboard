#include "keyboard/core/DwellController.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

using namespace std::chrono_literals;

constexpr auto kEpoch = std::chrono::steady_clock::time_point{};

TEST(DwellControllerTest, UpdateWithNothingTrackedReturnsNullopt) {
  DwellController controller(500ms);
  EXPECT_FALSE(controller.Update(kEpoch).has_value());
}

TEST(DwellControllerTest, DoesNotFireBeforeDwellDurationElapses) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_FALSE(controller.Update(kEpoch + 100ms).has_value());
  EXPECT_FALSE(controller.Update(kEpoch + 499ms).has_value());
}

TEST(DwellControllerTest, FiresOnceDwellDurationElapses) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);

  const auto fired = controller.Update(kEpoch + 500ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "a");
}

TEST(DwellControllerTest, FiringIsOneShotAndDoesNotRepeat) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  ASSERT_TRUE(controller.Update(kEpoch + 500ms).has_value());

  EXPECT_FALSE(controller.Update(kEpoch + 600ms).has_value());
  EXPECT_FALSE(controller.Update(kEpoch + 10s).has_value());
}

TEST(DwellControllerTest, OnPointerLeaveCancelsDwell) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.OnPointerLeave("a");

  EXPECT_FALSE(controller.Update(kEpoch + 500ms).has_value());
}

TEST(DwellControllerTest, OnPointerLeaveForNonTrackedKeyIsNoop) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.OnPointerLeave("b");

  EXPECT_TRUE(controller.Update(kEpoch + 500ms).has_value());
}

TEST(DwellControllerTest, ReEnteringDifferentKeyReplacesTrackedKey) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.OnPointerEnter("b", kEpoch + 200ms);

  EXPECT_FALSE(controller.Update(kEpoch + 500ms).has_value());
  const auto fired = controller.Update(kEpoch + 700ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "b");
}

TEST(DwellControllerTest, ReEnteringSameKeyRestartsProgress) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.OnPointerEnter("a", kEpoch + 400ms);

  EXPECT_FALSE(controller.Update(kEpoch + 500ms).has_value());
  EXPECT_TRUE(controller.Update(kEpoch + 900ms).has_value());
}

TEST(DwellControllerTest, PointerMoveOnSameKeyDoesNotRestartProgress) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.OnPointerMove("a", kEpoch + 400ms);

  const auto fired = controller.Update(kEpoch + 500ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "a");
}

TEST(DwellControllerTest, PointerMoveOnDifferentKeyStartsTrackingIt) {
  DwellController controller(500ms);
  controller.OnPointerMove("a", kEpoch);
  controller.OnPointerMove("b", kEpoch + 100ms);

  EXPECT_FALSE(controller.Update(kEpoch + 500ms).has_value());
  const auto fired = controller.Update(kEpoch + 600ms);
  ASSERT_TRUE(fired.has_value());
  EXPECT_EQ(*fired, "b");
}

TEST(DwellControllerTest, ProgressForUntrackedKeyIsZero) {
  DwellController controller(500ms);
  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch), 0.0F);

  controller.OnPointerEnter("a", kEpoch);
  EXPECT_FLOAT_EQ(controller.ProgressFor("b", kEpoch), 0.0F);
}

TEST(DwellControllerTest, ProgressForTrackedKeyIncreasesTowardOne) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch), 0.0F);
  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch + 250ms), 0.5F);
  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch + 500ms), 1.0F);
}

TEST(DwellControllerTest, ProgressForClampsPastCompletion) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch + 10s), 1.0F);
}

TEST(DwellControllerTest, ProgressForIsZeroAfterFiring) {
  DwellController controller(500ms);
  controller.OnPointerEnter("a", kEpoch);
  controller.Update(kEpoch + 500ms);

  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch + 500ms), 0.0F);
}

TEST(DwellControllerTest, ZeroDwellDurationFiresImmediately) {
  DwellController controller(0ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_TRUE(controller.Update(kEpoch).has_value());
}

TEST(DwellControllerTest, ZeroDwellDurationProgressIsOneImmediately) {
  DwellController controller(0ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_FLOAT_EQ(controller.ProgressFor("a", kEpoch), 1.0F);
}

TEST(DwellControllerTest, NegativeDwellDurationIsClampedToZero) {
  DwellController controller(-100ms);
  controller.OnPointerEnter("a", kEpoch);

  EXPECT_TRUE(controller.Update(kEpoch).has_value());
}

}  // namespace
}  // namespace osk::core
