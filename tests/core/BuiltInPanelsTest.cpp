#include "keyboard/core/BuiltInPanels.h"

#include <set>

#include <gtest/gtest.h>

namespace osk::core::panels {
namespace {

class BuiltInPanelTest : public ::testing::TestWithParam<Panel (*)()> {};

TEST_P(BuiltInPanelTest, HasAtLeastOneRow) {
  const Panel panel = GetParam()();
  EXPECT_FALSE(panel.rows.empty());
}

TEST_P(BuiltInPanelTest, HasNoEmptyRows) {
  const Panel panel = GetParam()();
  for (const PanelRow& row : panel.rows) {
    EXPECT_FALSE(row.keys.empty());
  }
}

TEST_P(BuiltInPanelTest, AllKeyIdsAreUniqueWithinThePanel) {
  const Panel panel = GetParam()();
  std::set<KeyId> seenIds;
  for (const PanelRow& row : panel.rows) {
    for (const Key& key : row.keys) {
      EXPECT_TRUE(seenIds.insert(key.id).second) << "duplicate key id: " << key.id;
    }
  }
}

TEST_P(BuiltInPanelTest, AllKeysHaveNonEmptyLabels) {
  const Panel panel = GetParam()();
  for (const PanelRow& row : panel.rows) {
    for (const Key& key : row.keys) {
      EXPECT_FALSE(key.label.empty()) << "empty label for key id: " << key.id;
    }
  }
}

TEST_P(BuiltInPanelTest, HasNonEmptyIdAndDisplayName) {
  const Panel panel = GetParam()();
  EXPECT_FALSE(panel.id.empty());
  EXPECT_FALSE(panel.displayName.empty());
}

INSTANTIATE_TEST_SUITE_P(AllBuiltInPanels, BuiltInPanelTest,
                          ::testing::Values(&Qwerty, &Numeric, &FunctionKeys, &ArrowKeys));

const Key* FindKey(const Panel& panel, const KeyId& id) {
  for (const PanelRow& row : panel.rows) {
    for (const Key& key : row.keys) {
      if (key.id == id) {
        return &key;
      }
    }
  }
  return nullptr;
}

TEST(QwertyPanelTest, LetterKeyHasLowerAndUpperVariantsAndAppliesCapsLock) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.q");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<TypeCharacter>(key->action);
  EXPECT_EQ(action.base, U'q');
  EXPECT_EQ(action.shifted, U'Q');
  EXPECT_TRUE(action.capsLockApplies);
}

TEST(QwertyPanelTest, DigitKeyHasShiftSymbolAndDoesNotApplyCapsLock) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.1");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<TypeCharacter>(key->action);
  EXPECT_EQ(action.base, U'1');
  EXPECT_EQ(action.shifted, U'!');
  EXPECT_FALSE(action.capsLockApplies);
}

TEST(QwertyPanelTest, ShiftEnterKeyBakesInShiftModifier) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.shift_enter");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<SendVirtualKey>(key->action);
  EXPECT_EQ(action.key, VirtualKey::kEnter);
  EXPECT_TRUE(HasModifier(action.modifiers, Modifier::kShift));
}

TEST(QwertyPanelTest, CopyKeyBakesInControlPlusC) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.copy");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<SendVirtualKey>(key->action);
  EXPECT_EQ(action.key, VirtualKey::kC);
  EXPECT_TRUE(HasModifier(action.modifiers, Modifier::kControl));
}

TEST(QwertyPanelTest, PasteKeyBakesInControlPlusV) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.paste");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<SendVirtualKey>(key->action);
  EXPECT_EQ(action.key, VirtualKey::kV);
  EXPECT_TRUE(HasModifier(action.modifiers, Modifier::kControl));
}

TEST(QwertyPanelTest, CapsLockKeyTogglesCapsLock) {
  const Panel panel = Qwerty();
  const Key* key = FindKey(panel, "qwerty.capslock");
  ASSERT_NE(key, nullptr);
  EXPECT_TRUE(std::holds_alternative<ToggleCapsLock>(key->action));
}

TEST(NumericPanelTest, DigitKeysHaveEqualBaseAndShifted) {
  const Panel panel = Numeric();
  const Key* key = FindKey(panel, "numeric.7");
  ASSERT_NE(key, nullptr);
  const auto& action = std::get<TypeCharacter>(key->action);
  EXPECT_EQ(action.base, U'7');
  EXPECT_EQ(action.shifted, U'7');
}

TEST(FunctionKeysPanelTest, ContainsAllTwelveFunctionKeys) {
  const Panel panel = FunctionKeys();
  for (int i = 1; i <= 12; ++i) {
    const KeyId id = "function.f" + std::to_string(i);
    EXPECT_NE(FindKey(panel, id), nullptr) << "missing key id: " << id;
  }
}

TEST(ArrowKeysPanelTest, ContainsAllFourDirections) {
  const Panel panel = ArrowKeys();
  EXPECT_NE(FindKey(panel, "arrows.up"), nullptr);
  EXPECT_NE(FindKey(panel, "arrows.down"), nullptr);
  EXPECT_NE(FindKey(panel, "arrows.left"), nullptr);
  EXPECT_NE(FindKey(panel, "arrows.right"), nullptr);
}

}  // namespace
}  // namespace osk::core::panels
