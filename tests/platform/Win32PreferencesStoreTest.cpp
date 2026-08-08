#include "keyboard/platform/Win32PreferencesStore.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <string>

namespace osk::platform {
namespace {

using namespace std::chrono_literals;

class Win32PreferencesStoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    directory_ = std::filesystem::temp_directory_path() /
                 ("osk_prefs_test_" + std::string(testInfo->test_suite_name()) + "_" +
                  std::string(testInfo->name()) + "_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
    std::filesystem::remove_all(directory_);
  }

  void TearDown() override { std::filesystem::remove_all(directory_); }

  std::filesystem::path FilePath() const { return directory_ / "preferences.json"; }

  void WriteRawFile(const std::string& contents) const {
    std::filesystem::create_directories(directory_);
    std::ofstream file(FilePath(), std::ios::binary | std::ios::trunc);
    file << contents;
  }

  std::filesystem::path directory_;
};

TEST_F(Win32PreferencesStoreTest, LoadReturnsDefaultsWhenFileDoesNotExist) {
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();
  const core::Preferences defaults;

  EXPECT_EQ(loaded.GetTheme(), defaults.GetTheme());
  EXPECT_EQ(loaded.GetTransparencyPercent(), defaults.GetTransparencyPercent());
  EXPECT_FLOAT_EQ(loaded.GetWindowScale(), defaults.GetWindowScale());
}

TEST_F(Win32PreferencesStoreTest, SaveThenLoadRoundTripsAllFields) {
  Win32PreferencesStore store(directory_);

  core::Preferences original;
  original.SetTheme(core::Theme::kDark);
  original.SetTransparencyPercent(42);
  original.SetWindowScale(1.5F);
  original.SetKeySize(0.75F);
  original.SetActivePanelOrder({"qwerty", "numeric"});
  original.SetDwellEnabled(true);
  original.SetDwellDelay(600ms);
  original.SetKeyRepeatInitialDelay(250ms);
  original.SetKeyRepeatInterval(40ms);

  store.Save(original);
  const core::Preferences loaded = store.Load();

  EXPECT_EQ(loaded.GetTheme(), core::Theme::kDark);
  EXPECT_EQ(loaded.GetTransparencyPercent(), 42);
  EXPECT_FLOAT_EQ(loaded.GetWindowScale(), 1.5F);
  EXPECT_FLOAT_EQ(loaded.GetKeySize(), 0.75F);
  EXPECT_EQ(loaded.GetActivePanelOrder(), (std::vector<core::PanelId>{"qwerty", "numeric"}));
  EXPECT_TRUE(loaded.IsDwellEnabled());
  EXPECT_EQ(loaded.GetDwellDelay(), 600ms);
  ASSERT_TRUE(loaded.GetKeyRepeatInitialDelay().has_value());
  EXPECT_EQ(*loaded.GetKeyRepeatInitialDelay(), 250ms);
  ASSERT_TRUE(loaded.GetKeyRepeatInterval().has_value());
  EXPECT_EQ(*loaded.GetKeyRepeatInterval(), 40ms);
}

TEST_F(Win32PreferencesStoreTest, SaveThenLoadRoundTripsUnsetOptionalKeyRepeatFields) {
  Win32PreferencesStore store(directory_);

  core::Preferences original;
  original.SetKeyRepeatInitialDelay(std::nullopt);
  original.SetKeyRepeatInterval(std::nullopt);

  store.Save(original);
  const core::Preferences loaded = store.Load();

  EXPECT_FALSE(loaded.GetKeyRepeatInitialDelay().has_value());
  EXPECT_FALSE(loaded.GetKeyRepeatInterval().has_value());
}

TEST_F(Win32PreferencesStoreTest, LoadReturnsDefaultsWhenFileIsEmpty) {
  WriteRawFile("");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();
  const core::Preferences defaults;

  EXPECT_EQ(loaded.GetTheme(), defaults.GetTheme());
  EXPECT_EQ(loaded.GetTransparencyPercent(), defaults.GetTransparencyPercent());
}

TEST_F(Win32PreferencesStoreTest, LoadReturnsDefaultsWhenFileIsMalformedJson) {
  WriteRawFile("{ this is not valid json");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();
  const core::Preferences defaults;

  EXPECT_EQ(loaded.GetTheme(), defaults.GetTheme());
}

TEST_F(Win32PreferencesStoreTest, LoadReturnsDefaultsWhenAFieldHasTheWrongType) {
  WriteRawFile(R"({"theme": "dark", "transparencyPercent": "not a number"})");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();
  const core::Preferences defaults;

  // The whole load is discarded on any type mismatch, not just the bad
  // field -- so even "theme" (which was well-formed) falls back too.
  EXPECT_EQ(loaded.GetTheme(), defaults.GetTheme());
  EXPECT_EQ(loaded.GetTransparencyPercent(), defaults.GetTransparencyPercent());
}

TEST_F(Win32PreferencesStoreTest, LoadClampsOutOfRangeValuesViaPreferencesOwnSetters) {
  WriteRawFile(R"({"transparencyPercent": 500, "windowScale": 100.0})");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();

  EXPECT_EQ(loaded.GetTransparencyPercent(), core::Preferences::kMaxTransparencyPercent);
  EXPECT_FLOAT_EQ(loaded.GetWindowScale(), core::Preferences::kMaxWindowScale);
}

TEST_F(Win32PreferencesStoreTest, LoadTreatsExplicitNullKeyRepeatFieldsAsUnset) {
  WriteRawFile(R"({"keyRepeatInitialDelayMs": null, "keyRepeatIntervalMs": null})");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();

  EXPECT_FALSE(loaded.GetKeyRepeatInitialDelay().has_value());
  EXPECT_FALSE(loaded.GetKeyRepeatInterval().has_value());
}

TEST_F(Win32PreferencesStoreTest, LoadTreatsMissingOptionalFieldsAsUnset) {
  WriteRawFile(R"({"theme": "light"})");
  const Win32PreferencesStore store(directory_);
  const core::Preferences loaded = store.Load();

  EXPECT_EQ(loaded.GetTheme(), core::Theme::kLight);
  EXPECT_FALSE(loaded.GetKeyRepeatInitialDelay().has_value());
  EXPECT_FALSE(loaded.GetKeyRepeatInterval().has_value());
}

TEST_F(Win32PreferencesStoreTest, SaveCreatesTheDirectoryIfItDoesNotExist) {
  ASSERT_FALSE(std::filesystem::exists(directory_));

  Win32PreferencesStore store(directory_);
  store.Save(core::Preferences{});

  EXPECT_TRUE(std::filesystem::exists(FilePath()));
}

TEST_F(Win32PreferencesStoreTest, SaveOverwritesPreviousContent) {
  Win32PreferencesStore store(directory_);

  core::Preferences first;
  first.SetTransparencyPercent(10);
  store.Save(first);

  core::Preferences second;
  second.SetTransparencyPercent(90);
  store.Save(second);

  EXPECT_EQ(store.Load().GetTransparencyPercent(), 90);
}

TEST_F(Win32PreferencesStoreTest, SaveDoesNotLeaveATempFileBehindOnSuccess) {
  Win32PreferencesStore store(directory_);
  store.Save(core::Preferences{});

  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path(FilePath()).concat(".tmp")));
}

TEST(ResolveDefaultPreferencesDirectoryTest, ReturnsANonEmptyPathEndingInTheAppFolderName) {
  const std::filesystem::path path = ResolveDefaultPreferencesDirectory();
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.filename(), "OnScreenKeyboard");
}

}  // namespace
}  // namespace osk::platform
