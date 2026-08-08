#include "keyboard/platform/Win32AlwaysOnTopController.h"

#include <windows.h>

#include <chrono>
#include <gtest/gtest.h>

#include "Win32WindowFixture.h"

namespace osk::platform {
namespace {

bool IsTopmost(HWND hwnd) { return (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0; }

void PumpMessagesFor(std::chrono::milliseconds duration) {
  const auto deadline = std::chrono::steady_clock::now() + duration;
  MSG msg;
  while (std::chrono::steady_clock::now() < deadline) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    Sleep(5);
  }
}

struct ScopedWindow {
  HWND hwnd = nullptr;
  ~ScopedWindow() {
    if (hwnd != nullptr) {
      DestroyWindow(hwnd);
    }
  }
};

using Win32AlwaysOnTopControllerTest = Win32WindowFixture;

TEST_F(Win32AlwaysOnTopControllerTest, StartMakesTheWindowTopmost) {
  ASSERT_FALSE(IsTopmost(Hwnd()));

  Win32AlwaysOnTopController controller(Hwnd());
  controller.Start();

  EXPECT_TRUE(IsTopmost(Hwnd()));
  controller.Stop();
}

TEST_F(Win32AlwaysOnTopControllerTest, StopIsSafeToCallWithoutStart) {
  Win32AlwaysOnTopController controller(Hwnd());
  controller.Stop();
  SUCCEED();
}

TEST_F(Win32AlwaysOnTopControllerTest, StopIsIdempotent) {
  Win32AlwaysOnTopController controller(Hwnd());
  controller.Start();
  controller.Stop();
  controller.Stop();
  SUCCEED();
}

TEST_F(Win32AlwaysOnTopControllerTest, DestructorStopsWithoutAnExplicitStopCall) {
  {
    Win32AlwaysOnTopController controller(Hwnd());
    controller.Start();
  }
  // No crash, and a fresh controller can immediately claim the
  // single-active-instance slot again -- proves the destructor released it.
  Win32AlwaysOnTopController another(Hwnd());
  another.Start();
  EXPECT_TRUE(IsTopmost(Hwnd()));
  another.Stop();
}

// The one test in this file that exercises the real foreground-change
// watchdog end to end. Foreground-switching is inherently
// environment-sensitive (session type, CI runner configuration) -- this
// skips rather than flaking if Windows won't grant the switch, same
// approach as Win32InputInjectorTest's safety skip.
TEST_F(Win32AlwaysOnTopControllerTest, ReassertsTopmostWhenForegroundChangesAway) {
  ScopedWindow other{CreateWindowExW(0, L"STATIC", L"osk second dummy window", WS_OVERLAPPEDWINDOW, -32500, -32500,
                                      100, 100, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr)};
  ASSERT_NE(other.hwnd, nullptr);
  ShowWindow(other.hwnd, SW_SHOWNOACTIVATE);

  if (!TryTakeForeground()) {
    GTEST_SKIP() << "Could not take OS foreground focus in this environment.";
  }

  Win32AlwaysOnTopController controller(Hwnd());
  controller.Start();
  ASSERT_TRUE(IsTopmost(Hwnd()));

  // Simulate something else knocking our window down from topmost.
  SetWindowPos(Hwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  ASSERT_FALSE(IsTopmost(Hwnd()));

  if (SetForegroundWindow(other.hwnd) == 0) {
    controller.Stop();
    GTEST_SKIP() << "Could not switch foreground to the second window in this environment.";
  }
  PumpMessagesFor(std::chrono::milliseconds{50});

  // Switching foreground back to our window is what should fire
  // EVENT_SYSTEM_FOREGROUND and trigger the watchdog's reassertion.
  SetForegroundWindow(Hwnd());
  PumpMessagesFor(std::chrono::milliseconds{300});

  EXPECT_TRUE(IsTopmost(Hwnd()));
  controller.Stop();
}

}  // namespace
}  // namespace osk::platform
