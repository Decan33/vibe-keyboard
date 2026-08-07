#include "keyboard/core/Preferences.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

using namespace std::chrono_literals;

TEST(PreferencesTest, DefaultsAreSensibleAndWithinRange) {
  Preferences prefs;
  EXPECT_EQ(prefs.GetTheme(), Theme::kSystem);
  EXPECT_EQ(prefs.GetTransparencyPercent(), Preferences::kMaxTransparencyPercent);
  EXPECT_FLOAT_EQ(prefs.GetWindowScale(), 1.0F);
  EXPECT_FLOAT_EQ(prefs.GetKeySize(), 1.0F);
  EXPECT_TRUE(prefs.GetActivePanelOrder().empty());
  EXPECT_FALSE(prefs.IsDwellEnabled());
  EXPECT_FALSE(prefs.GetKeyRepeatInitialDelay().has_value());
  EXPECT_FALSE(prefs.GetKeyRepeatInterval().has_value());
}

TEST(PreferencesTest, SetThemeStoresExactly) {
  Preferences prefs;
  prefs.SetTheme(Theme::kDark);
  EXPECT_EQ(prefs.GetTheme(), Theme::kDark);
  prefs.SetTheme(Theme::kLight);
  EXPECT_EQ(prefs.GetTheme(), Theme::kLight);
}

TEST(PreferencesTest, TransparencyWithinRangeIsUnchanged) {
  Preferences prefs;
  prefs.SetTransparencyPercent(42);
  EXPECT_EQ(prefs.GetTransparencyPercent(), 42);
}

TEST(PreferencesTest, TransparencyAtBoundsIsUnchanged) {
  Preferences prefs;
  prefs.SetTransparencyPercent(Preferences::kMinTransparencyPercent);
  EXPECT_EQ(prefs.GetTransparencyPercent(), Preferences::kMinTransparencyPercent);
  prefs.SetTransparencyPercent(Preferences::kMaxTransparencyPercent);
  EXPECT_EQ(prefs.GetTransparencyPercent(), Preferences::kMaxTransparencyPercent);
}

TEST(PreferencesTest, TransparencyBelowMinIsClamped) {
  Preferences prefs;
  prefs.SetTransparencyPercent(-50);
  EXPECT_EQ(prefs.GetTransparencyPercent(), Preferences::kMinTransparencyPercent);
}

TEST(PreferencesTest, TransparencyAboveMaxIsClamped) {
  Preferences prefs;
  prefs.SetTransparencyPercent(500);
  EXPECT_EQ(prefs.GetTransparencyPercent(), Preferences::kMaxTransparencyPercent);
}

TEST(PreferencesTest, WindowScaleBelowMinIsClamped) {
  Preferences prefs;
  prefs.SetWindowScale(0.0F);
  EXPECT_FLOAT_EQ(prefs.GetWindowScale(), Preferences::kMinWindowScale);
}

TEST(PreferencesTest, WindowScaleAboveMaxIsClamped) {
  Preferences prefs;
  prefs.SetWindowScale(100.0F);
  EXPECT_FLOAT_EQ(prefs.GetWindowScale(), Preferences::kMaxWindowScale);
}

TEST(PreferencesTest, WindowScaleAtBoundsIsUnchanged) {
  Preferences prefs;
  prefs.SetWindowScale(Preferences::kMinWindowScale);
  EXPECT_FLOAT_EQ(prefs.GetWindowScale(), Preferences::kMinWindowScale);
  prefs.SetWindowScale(Preferences::kMaxWindowScale);
  EXPECT_FLOAT_EQ(prefs.GetWindowScale(), Preferences::kMaxWindowScale);
}

TEST(PreferencesTest, KeySizeBelowMinIsClamped) {
  Preferences prefs;
  prefs.SetKeySize(-1.0F);
  EXPECT_FLOAT_EQ(prefs.GetKeySize(), Preferences::kMinKeySize);
}

TEST(PreferencesTest, KeySizeAboveMaxIsClamped) {
  Preferences prefs;
  prefs.SetKeySize(50.0F);
  EXPECT_FLOAT_EQ(prefs.GetKeySize(), Preferences::kMaxKeySize);
}

TEST(PreferencesTest, ActivePanelOrderPreservesInputOrder) {
  Preferences prefs;
  prefs.SetActivePanelOrder({"numeric", "arrows", "function"});
  const std::vector<PanelId> expected = {"numeric", "arrows", "function"};
  EXPECT_EQ(prefs.GetActivePanelOrder(), expected);
}

TEST(PreferencesTest, ActivePanelOrderDropsDuplicatesKeepingFirstOccurrence) {
  Preferences prefs;
  prefs.SetActivePanelOrder({"numeric", "arrows", "numeric", "function", "arrows"});
  const std::vector<PanelId> expected = {"numeric", "arrows", "function"};
  EXPECT_EQ(prefs.GetActivePanelOrder(), expected);
}

TEST(PreferencesTest, ActivePanelOrderAcceptsEmptyList) {
  Preferences prefs;
  prefs.SetActivePanelOrder({"numeric"});
  prefs.SetActivePanelOrder({});
  EXPECT_TRUE(prefs.GetActivePanelOrder().empty());
}

TEST(PreferencesTest, DwellEnabledToggles) {
  Preferences prefs;
  prefs.SetDwellEnabled(true);
  EXPECT_TRUE(prefs.IsDwellEnabled());
  prefs.SetDwellEnabled(false);
  EXPECT_FALSE(prefs.IsDwellEnabled());
}

TEST(PreferencesTest, DwellDelayWithinRangeIsUnchanged) {
  Preferences prefs;
  prefs.SetDwellDelay(1000ms);
  EXPECT_EQ(prefs.GetDwellDelay(), 1000ms);
}

TEST(PreferencesTest, DwellDelayBelowMinIsClamped) {
  Preferences prefs;
  prefs.SetDwellDelay(1ms);
  EXPECT_EQ(prefs.GetDwellDelay(), Preferences::kMinDwellDelay);
}

TEST(PreferencesTest, DwellDelayAboveMaxIsClamped) {
  Preferences prefs;
  prefs.SetDwellDelay(60s);
  EXPECT_EQ(prefs.GetDwellDelay(), Preferences::kMaxDwellDelay);
}

TEST(PreferencesTest, KeyRepeatInitialDelayNulloptMeansUseSystemDefault) {
  Preferences prefs;
  prefs.SetKeyRepeatInitialDelay(300ms);
  ASSERT_TRUE(prefs.GetKeyRepeatInitialDelay().has_value());

  prefs.SetKeyRepeatInitialDelay(std::nullopt);
  EXPECT_FALSE(prefs.GetKeyRepeatInitialDelay().has_value());
}

TEST(PreferencesTest, KeyRepeatInitialDelayIsClampedWhenSet) {
  Preferences prefs;
  prefs.SetKeyRepeatInitialDelay(1ms);
  EXPECT_EQ(prefs.GetKeyRepeatInitialDelay(), Preferences::kMinKeyRepeatInitialDelay);

  prefs.SetKeyRepeatInitialDelay(10s);
  EXPECT_EQ(prefs.GetKeyRepeatInitialDelay(), Preferences::kMaxKeyRepeatInitialDelay);
}

TEST(PreferencesTest, KeyRepeatIntervalNulloptMeansUseSystemDefault) {
  Preferences prefs;
  prefs.SetKeyRepeatInterval(50ms);
  ASSERT_TRUE(prefs.GetKeyRepeatInterval().has_value());

  prefs.SetKeyRepeatInterval(std::nullopt);
  EXPECT_FALSE(prefs.GetKeyRepeatInterval().has_value());
}

TEST(PreferencesTest, KeyRepeatIntervalIsClampedWhenSet) {
  Preferences prefs;
  prefs.SetKeyRepeatInterval(0ms);
  EXPECT_EQ(prefs.GetKeyRepeatInterval(), Preferences::kMinKeyRepeatInterval);

  prefs.SetKeyRepeatInterval(10s);
  EXPECT_EQ(prefs.GetKeyRepeatInterval(), Preferences::kMaxKeyRepeatInterval);
}

}  // namespace
}  // namespace osk::core
