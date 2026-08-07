#include "keyboard/core/Dispatcher.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

Key MakeLetterKey() {
  return Key{
      .id = "test.a",
      .label = "a",
      .action = TypeCharacter{.base = U'a', .shifted = U'A', .capsLockApplies = true},
  };
}

Key MakeSymbolKey() {
  return Key{
      .id = "test.1",
      .label = "1",
      .action = TypeCharacter{.base = U'1', .shifted = U'!', .capsLockApplies = false},
  };
}

Key MakeEnterKey() {
  return Key{.id = "test.enter", .label = "Enter", .action = SendVirtualKey{.key = VirtualKey::kEnter}};
}

Key MakeShiftEnterKey() {
  return Key{
      .id = "test.shift_enter",
      .label = "Shift+Enter",
      .action = SendVirtualKey{.key = VirtualKey::kEnter, .modifiers = Modifier::kShift},
  };
}

Key MakeShiftToggleKey() {
  return Key{.id = "test.shift", .label = "Shift", .action = ToggleModifier{.modifier = ModifierKind::kShift}};
}

Key MakeCapsLockKey() {
  return Key{.id = "test.capslock", .label = "Caps Lock", .action = ToggleCapsLock{}};
}

class DispatcherTest : public ::testing::Test {
 protected:
  ModifierState modifierState_;
  Dispatcher dispatcher_{modifierState_};
};

TEST_F(DispatcherTest, TypeCharacterWithNoModifiersSendsBase) {
  const auto event = dispatcher_.ActivateKey(MakeLetterKey());
  ASSERT_TRUE(event.has_value());
  ASSERT_TRUE(std::holds_alternative<CharacterEvent>(*event));
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'a');
}

TEST_F(DispatcherTest, TypeCharacterWithLatchedShiftSendsShiftedAndConsumesLatch) {
  modifierState_.ToggleLatch(ModifierKind::kShift);

  const auto event = dispatcher_.ActivateKey(MakeLetterKey());
  ASSERT_TRUE(event.has_value());
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'A');
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kShift));
}

TEST_F(DispatcherTest, TypeCharacterWithForceShiftOnceSendsShiftedWithoutLatching) {
  const auto event = dispatcher_.ActivateKey(MakeLetterKey(), ActivationOverride::kForceShiftOnce);
  ASSERT_TRUE(event.has_value());
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'A');
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kShift));
}

TEST_F(DispatcherTest, ForceShiftOnceAlongsideLatchedShiftStillConsumesLatch) {
  modifierState_.ToggleLatch(ModifierKind::kShift);

  const auto event = dispatcher_.ActivateKey(MakeLetterKey(), ActivationOverride::kForceShiftOnce);
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'A');
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kShift));
}

TEST_F(DispatcherTest, CapsLockAloneAppliesToLettersOnly) {
  modifierState_.ToggleCapsLock();

  const auto letterEvent = dispatcher_.ActivateKey(MakeLetterKey());
  EXPECT_EQ(std::get<CharacterEvent>(*letterEvent).codepoint, U'A');

  const auto symbolEvent = dispatcher_.ActivateKey(MakeSymbolKey());
  EXPECT_EQ(std::get<CharacterEvent>(*symbolEvent).codepoint, U'1');
}

TEST_F(DispatcherTest, ShiftAlwaysAppliesEvenToSymbols) {
  modifierState_.ToggleLatch(ModifierKind::kShift);

  const auto event = dispatcher_.ActivateKey(MakeSymbolKey());
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'!');
}

TEST_F(DispatcherTest, CapsLockAndShiftTogetherCancelOutForLetters) {
  modifierState_.ToggleCapsLock();
  modifierState_.ToggleLatch(ModifierKind::kShift);

  const auto event = dispatcher_.ActivateKey(MakeLetterKey());
  EXPECT_EQ(std::get<CharacterEvent>(*event).codepoint, U'a');
  EXPECT_TRUE(modifierState_.IsCapsLockOn());
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kShift));
}

TEST_F(DispatcherTest, SendVirtualKeyWithNoModifiersSendsBareKey) {
  const auto event = dispatcher_.ActivateKey(MakeEnterKey());
  ASSERT_TRUE(event.has_value());
  const auto& virtualKeyEvent = std::get<VirtualKeyEvent>(*event);
  EXPECT_EQ(virtualKeyEvent.key, VirtualKey::kEnter);
  EXPECT_EQ(virtualKeyEvent.modifiers, Modifier::kNone);
}

TEST_F(DispatcherTest, SendVirtualKeyBakedInModifiersApplyWithoutLatch) {
  const auto event = dispatcher_.ActivateKey(MakeShiftEnterKey());
  const auto& virtualKeyEvent = std::get<VirtualKeyEvent>(*event);
  EXPECT_EQ(virtualKeyEvent.key, VirtualKey::kEnter);
  EXPECT_TRUE(HasModifier(virtualKeyEvent.modifiers, Modifier::kShift));
}

TEST_F(DispatcherTest, SendVirtualKeyUnionsBakedInAndLatchedModifiers) {
  modifierState_.ToggleLatch(ModifierKind::kControl);

  const auto event = dispatcher_.ActivateKey(MakeShiftEnterKey());
  const auto& virtualKeyEvent = std::get<VirtualKeyEvent>(*event);
  EXPECT_TRUE(HasModifier(virtualKeyEvent.modifiers, Modifier::kShift));
  EXPECT_TRUE(HasModifier(virtualKeyEvent.modifiers, Modifier::kControl));
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kControl));
}

TEST_F(DispatcherTest, ForceShiftOnceIsIgnoredForSendVirtualKey) {
  const auto withOverride = dispatcher_.ActivateKey(MakeEnterKey(), ActivationOverride::kForceShiftOnce);
  const auto& virtualKeyEvent = std::get<VirtualKeyEvent>(*withOverride);
  EXPECT_EQ(virtualKeyEvent.modifiers, Modifier::kNone);
}

TEST_F(DispatcherTest, ToggleModifierReturnsNulloptAndLatchesWithoutConsuming) {
  modifierState_.ToggleLatch(ModifierKind::kControl);

  const auto event = dispatcher_.ActivateKey(MakeShiftToggleKey());
  EXPECT_FALSE(event.has_value());
  EXPECT_TRUE(modifierState_.IsLatched(ModifierKind::kShift));
  EXPECT_TRUE(modifierState_.IsLatched(ModifierKind::kControl));
}

TEST_F(DispatcherTest, ToggleCapsLockReturnsNulloptAndFlipsCapsLock) {
  const auto event = dispatcher_.ActivateKey(MakeCapsLockKey());
  EXPECT_FALSE(event.has_value());
  EXPECT_TRUE(modifierState_.IsCapsLockOn());
}

TEST_F(DispatcherTest, ActivatingNonModifierKeyReleasesAllLatchesAtOnce) {
  modifierState_.ToggleLatch(ModifierKind::kShift);
  modifierState_.ToggleLatch(ModifierKind::kControl);
  modifierState_.ToggleLatch(ModifierKind::kAlt);

  dispatcher_.ActivateKey(MakeLetterKey());

  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kShift));
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kControl));
  EXPECT_FALSE(modifierState_.IsLatched(ModifierKind::kAlt));
}

}  // namespace
}  // namespace osk::core
