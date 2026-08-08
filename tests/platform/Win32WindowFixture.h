#pragma once

#include <windows.h>

#include <gtest/gtest.h>

namespace osk::platform {

// Shared fixture that creates a real, off-screen, non-activating top-level
// HWND for platform tests that need to observe genuine Win32 window state
// (ex-style bits, layered-window attributes, topmost z-order) without
// touching anything visible to the person running the tests.
class Win32WindowFixture : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  HWND Hwnd() const noexcept { return hwnd_; }

  // Attempts to give this fixture's window real OS input focus, so a test
  // can safely exercise SendInput-based code without risking keystrokes
  // landing wherever the person running the tests currently has focused.
  // Returns false if Windows refuses the foreground-switch request (which
  // happens intermittently depending on session/CI configuration) — callers
  // must GTEST_SKIP() rather than proceed to inject input blind, since that
  // would mean sending it to some other, unintended window.
  bool TryTakeForeground() const;

 private:
  HWND hwnd_ = nullptr;
};

}  // namespace osk::platform
