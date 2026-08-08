#include "keyboard/platform/Win32InputInjector.h"

#include <windows.h>

#include <chrono>
#include <cstddef>
#include <gtest/gtest.h>

#include "Win32WindowFixture.h"

namespace osk::platform {
namespace {

// SendInput only inserts events into the system input queue; actually
// receiving them as WM_CHAR requires pumping this thread's message queue.
// Bounded so a delivery failure shows up as a (fast) assertion failure
// rather than a hang.
void PumpMessagesUntilTextArrivesOrTimeout(HWND edit, std::chrono::milliseconds timeout) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  MSG msg;
  while (std::chrono::steady_clock::now() < deadline) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    if (GetWindowTextLengthW(edit) > 0) {
      return;
    }
    Sleep(5);
  }
}

using Win32InputInjectorTest = Win32WindowFixture;

// The only test in this file that touches the real SendInput API. To avoid
// ever risking a keystroke landing in whatever window the person running
// these tests actually has focused, it first takes foreground+focus on a
// throwaway EDIT control that lives entirely inside this test's own hidden,
// off-screen fixture window -- and skips outright (rather than proceeding)
// if Windows won't grant that focus switch. Which INPUT events get built
// for a given KeyEvent is already exhaustively covered by
// BuildInputEventsTest.cpp without touching the OS at all; this test only
// proves the real SendInput call actually delivers.
TEST_F(Win32InputInjectorTest, CharacterEventIsDeliveredToTheFocusedControl) {
  const HWND edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE, 0, 0, 100, 20, Hwnd(), nullptr,
                                     GetModuleHandleW(nullptr), nullptr);
  ASSERT_NE(edit, nullptr);

  if (!TryTakeForeground()) {
    GTEST_SKIP() << "Could not take OS foreground focus in this environment; skipping rather than risking "
                     "input landing in an unintended window.";
  }
  SetFocus(edit);

  Win32InputInjector injector;
  injector.InjectKeyEvent(core::CharacterEvent{.codepoint = U'x'});

  PumpMessagesUntilTextArrivesOrTimeout(edit, std::chrono::milliseconds{500});

  wchar_t text[8] = {};
  GetWindowTextW(edit, text, static_cast<int>(std::size(text)));
  EXPECT_STREQ(text, L"x");
}

}  // namespace
}  // namespace osk::platform
