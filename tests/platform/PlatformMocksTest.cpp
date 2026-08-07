#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mocks/MockAlwaysOnTopController.h"
#include "mocks/MockInputInjector.h"
#include "mocks/MockPreferencesStore.h"
#include "mocks/MockSystemMetrics.h"
#include "mocks/MockWindowTransparency.h"

namespace osk::platform {
namespace {

using ::testing::Eq;
using ::testing::Return;
using ::testing::_;

// These tests don't exercise any behavior of our own — the interfaces have
// none yet, by design (real bodies are deferred to Windows-side work). They
// prove the interface + mock seam compiles, links, and is usable from test
// code, which is what src/app and future osk::core consumers will rely on.

TEST(MockInputInjectorTest, InjectKeyEventCanBeExpectedAndVerified) {
  MockInputInjector injector;
  const core::KeyEvent event = core::CharacterEvent{.codepoint = U'a'};

  EXPECT_CALL(injector, InjectKeyEvent(Eq(event))).Times(1);
  injector.InjectKeyEvent(event);
}

TEST(MockAlwaysOnTopControllerTest, StartAndStopCanBeExpectedAndVerified) {
  MockAlwaysOnTopController controller;

  EXPECT_CALL(controller, Start()).Times(1);
  EXPECT_CALL(controller, Stop()).Times(1);
  controller.Start();
  controller.Stop();
}

TEST(MockWindowTransparencyTest, SetAlphaCanBeExpectedWithASpecificValue) {
  MockWindowTransparency transparency;

  EXPECT_CALL(transparency, SetAlpha(128)).Times(1);
  transparency.SetAlpha(128);
}

TEST(MockPreferencesStoreTest, LoadReturnsTheConfiguredPreferences) {
  MockPreferencesStore store;
  core::Preferences prefs;
  prefs.SetTransparencyPercent(42);

  EXPECT_CALL(store, Load()).WillOnce(Return(prefs));
  const core::Preferences loaded = store.Load();
  EXPECT_EQ(loaded.GetTransparencyPercent(), 42);
}

TEST(MockPreferencesStoreTest, SaveCanBeExpectedAndVerified) {
  MockPreferencesStore store;
  const core::Preferences prefs;

  EXPECT_CALL(store, Save(_)).Times(1);
  store.Save(prefs);
}

TEST(MockSystemMetricsTest, KeyboardRepeatSettingsReturnConfiguredValues) {
  MockSystemMetrics metrics;

  EXPECT_CALL(metrics, GetKeyboardRepeatDelay()).WillOnce(Return(std::chrono::milliseconds(500)));
  EXPECT_CALL(metrics, GetKeyboardRepeatInterval()).WillOnce(Return(std::chrono::milliseconds(30)));

  EXPECT_EQ(metrics.GetKeyboardRepeatDelay(), std::chrono::milliseconds(500));
  EXPECT_EQ(metrics.GetKeyboardRepeatInterval(), std::chrono::milliseconds(30));
}

}  // namespace
}  // namespace osk::platform
