#include "Win32WindowFixture.h"

namespace osk::platform {
namespace {

constexpr wchar_t kWindowClassName[] = L"OskPlatformTestFixtureWindow";

LRESULT CALLBACK FixtureWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Thread-safe (magic static) and idempotent: every test's SetUp() calls
// this, but RegisterClassExW only actually runs once per process. A false
// return here always surfaces anyway, via CreateWindowExW failing right
// after in SetUp() — ASSERT_* is deliberately not used in this non-test
// function, since a fatal assertion here would only unwind out of this
// helper, not out of the calling SetUp().
bool EnsureWindowClassRegistered() {
  static const bool registered = [] {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = &FixtureWndProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = kWindowClassName;
    return RegisterClassExW(&windowClass) != 0;
  }();
  return registered;
}

}  // namespace

void Win32WindowFixture::SetUp() {
  ASSERT_TRUE(EnsureWindowClassRegistered()) << "RegisterClassExW failed with error " << GetLastError();

  // Positioned far off-screen rather than WS_VISIBLE-less: a hidden window
  // can't reliably take foreground/focus (needed by TryTakeForeground), but
  // a real, off-screen, visible window can, while still never being seen.
  hwnd_ = CreateWindowExW(0, kWindowClassName, L"osk platform test window", WS_OVERLAPPEDWINDOW, -32000, -32000, 200,
                           100, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ASSERT_NE(hwnd_, nullptr) << "CreateWindowExW failed with error " << GetLastError();
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
}

void Win32WindowFixture::TearDown() {
  if (hwnd_ != nullptr) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}

bool Win32WindowFixture::TryTakeForeground() const {
  return SetForegroundWindow(hwnd_) != 0 && SetFocus(hwnd_) != nullptr;
}

}  // namespace osk::platform
