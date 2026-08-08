#include "keyboard/platform/Win32WindowTransparency.h"

#include <windows.h>

namespace osk::platform {

Win32WindowTransparency::Win32WindowTransparency(HWND hwnd) noexcept : hwnd_(hwnd) {}

void Win32WindowTransparency::SetAlpha(std::uint8_t alpha) {
  const LONG_PTR exStyle = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
  if ((exStyle & WS_EX_LAYERED) == 0) {
    SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
  }

  // Best-effort, matching the rest of osk::platform: a failure here (e.g.
  // hwnd_ already destroyed by the time a queued preferences change fires)
  // is silently ignored rather than thrown, since IWindowTransparency's
  // interface is void and there is nothing meaningful to recover into.
  SetLayeredWindowAttributes(hwnd_, 0, alpha, LWA_ALPHA);
}

}  // namespace osk::platform
