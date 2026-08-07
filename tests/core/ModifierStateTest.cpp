#include "keyboard/core/ModifierState.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

TEST(ModifierStateTest, StartsWithNothingLatchedOrToggled) {
  ModifierState state;
  EXPECT_FALSE(state.IsLatched(ModifierKind::kShift));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kControl));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kAlt));
  EXPECT_FALSE(state.IsCapsLockOn());
  EXPECT_EQ(state.LatchedMask(), Modifier::kNone);
}

TEST(ModifierStateTest, ToggleLatchSetsThenClearsShift) {
  ModifierState state;
  state.ToggleLatch(ModifierKind::kShift);
  EXPECT_TRUE(state.IsLatched(ModifierKind::kShift));

  state.ToggleLatch(ModifierKind::kShift);
  EXPECT_FALSE(state.IsLatched(ModifierKind::kShift));
}

TEST(ModifierStateTest, EachModifierLatchesIndependently) {
  ModifierState state;
  state.ToggleLatch(ModifierKind::kControl);

  EXPECT_TRUE(state.IsLatched(ModifierKind::kControl));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kShift));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kAlt));
}

TEST(ModifierStateTest, LatchedMaskUnionsAllLatchedModifiers) {
  ModifierState state;
  state.ToggleLatch(ModifierKind::kShift);
  state.ToggleLatch(ModifierKind::kAlt);

  const Modifier mask = state.LatchedMask();
  EXPECT_TRUE(HasModifier(mask, Modifier::kShift));
  EXPECT_TRUE(HasModifier(mask, Modifier::kAlt));
  EXPECT_FALSE(HasModifier(mask, Modifier::kControl));
}

TEST(ModifierStateTest, ToggleCapsLockFlipsIndependentlyOfLatches) {
  ModifierState state;
  state.ToggleLatch(ModifierKind::kShift);

  state.ToggleCapsLock();
  EXPECT_TRUE(state.IsCapsLockOn());
  EXPECT_TRUE(state.IsLatched(ModifierKind::kShift));

  state.ToggleCapsLock();
  EXPECT_FALSE(state.IsCapsLockOn());
}

TEST(ModifierStateTest, ConsumeLatchesClearsAllLatchesButNotCapsLock) {
  ModifierState state;
  state.ToggleLatch(ModifierKind::kShift);
  state.ToggleLatch(ModifierKind::kControl);
  state.ToggleLatch(ModifierKind::kAlt);
  state.ToggleCapsLock();

  state.ConsumeLatches();

  EXPECT_FALSE(state.IsLatched(ModifierKind::kShift));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kControl));
  EXPECT_FALSE(state.IsLatched(ModifierKind::kAlt));
  EXPECT_TRUE(state.IsCapsLockOn());
}

TEST(ModifierStateTest, ConsumeLatchesOnAlreadyClearStateIsNoop) {
  ModifierState state;
  state.ConsumeLatches();
  EXPECT_FALSE(state.IsLatched(ModifierKind::kShift));
}

}  // namespace
}  // namespace osk::core
