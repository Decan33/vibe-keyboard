#include "keyboard/core/KeyAction.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

TEST(ModifierMaskTest, OrCombinesFlags) {
  const Modifier mask = Modifier::kShift | Modifier::kControl;
  EXPECT_TRUE(HasModifier(mask, Modifier::kShift));
  EXPECT_TRUE(HasModifier(mask, Modifier::kControl));
  EXPECT_FALSE(HasModifier(mask, Modifier::kAlt));
}

TEST(ModifierMaskTest, NoneHasNoFlags) {
  EXPECT_FALSE(HasModifier(Modifier::kNone, Modifier::kShift));
  EXPECT_FALSE(HasModifier(Modifier::kNone, Modifier::kControl));
  EXPECT_FALSE(HasModifier(Modifier::kNone, Modifier::kAlt));
}

TEST(ModifierMaskTest, OrAssignAccumulates) {
  Modifier mask = Modifier::kNone;
  mask |= Modifier::kShift;
  mask |= Modifier::kAlt;
  EXPECT_TRUE(HasModifier(mask, Modifier::kShift));
  EXPECT_TRUE(HasModifier(mask, Modifier::kAlt));
  EXPECT_FALSE(HasModifier(mask, Modifier::kControl));
}

TEST(ModifierMaskTest, AllThreeFlagsCombine) {
  const Modifier mask = Modifier::kShift | Modifier::kControl | Modifier::kAlt;
  EXPECT_TRUE(HasModifier(mask, Modifier::kShift));
  EXPECT_TRUE(HasModifier(mask, Modifier::kControl));
  EXPECT_TRUE(HasModifier(mask, Modifier::kAlt));
}

TEST(TypeCharacterTest, EqualityComparesAllFields) {
  const TypeCharacter a{.base = U'a', .shifted = U'A', .capsLockApplies = true};
  const TypeCharacter same{.base = U'a', .shifted = U'A', .capsLockApplies = true};
  const TypeCharacter differentBase{.base = U'b', .shifted = U'A', .capsLockApplies = true};
  const TypeCharacter differentCaps{.base = U'a', .shifted = U'A', .capsLockApplies = false};

  EXPECT_EQ(a, same);
  EXPECT_NE(a, differentBase);
  EXPECT_NE(a, differentCaps);
}

TEST(SendVirtualKeyTest, EqualityComparesKeyAndModifiers) {
  const SendVirtualKey a{.key = VirtualKey::kEnter, .modifiers = Modifier::kShift};
  const SendVirtualKey same{.key = VirtualKey::kEnter, .modifiers = Modifier::kShift};
  const SendVirtualKey differentKey{.key = VirtualKey::kTab, .modifiers = Modifier::kShift};
  const SendVirtualKey differentModifiers{.key = VirtualKey::kEnter, .modifiers = Modifier::kNone};

  EXPECT_EQ(a, same);
  EXPECT_NE(a, differentKey);
  EXPECT_NE(a, differentModifiers);
}

TEST(ToggleModifierTest, EqualityComparesModifierKind) {
  EXPECT_EQ(ToggleModifier{.modifier = ModifierKind::kShift}, (ToggleModifier{.modifier = ModifierKind::kShift}));
  EXPECT_NE(ToggleModifier{.modifier = ModifierKind::kShift}, (ToggleModifier{.modifier = ModifierKind::kControl}));
}

TEST(ToggleCapsLockTest, AlwaysEqual) {
  EXPECT_EQ(ToggleCapsLock{}, ToggleCapsLock{});
}

TEST(KeyActionTest, VariantHoldsDistinctAlternatives) {
  const KeyAction typeAction = TypeCharacter{.base = U'a', .shifted = U'A', .capsLockApplies = true};
  const KeyAction sendAction = SendVirtualKey{.key = VirtualKey::kEnter};
  const KeyAction toggleAction = ToggleModifier{.modifier = ModifierKind::kShift};
  const KeyAction capsAction = ToggleCapsLock{};

  EXPECT_TRUE(std::holds_alternative<TypeCharacter>(typeAction));
  EXPECT_TRUE(std::holds_alternative<SendVirtualKey>(sendAction));
  EXPECT_TRUE(std::holds_alternative<ToggleModifier>(toggleAction));
  EXPECT_TRUE(std::holds_alternative<ToggleCapsLock>(capsAction));
}

}  // namespace
}  // namespace osk::core
