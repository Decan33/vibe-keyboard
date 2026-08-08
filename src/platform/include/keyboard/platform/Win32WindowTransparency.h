#pragma once

#include "keyboard/platform/IWindowTransparency.h"

struct HWND__;
using HWND = HWND__*;

namespace osk::platform {

// Wraps WS_EX_LAYERED + SetLayeredWindowAttributes. Does not own hwnd — the
// caller (src/app) owns the window's lifetime; this class must not outlive
// it.
class Win32WindowTransparency : public IWindowTransparency {
 public:
  explicit Win32WindowTransparency(HWND hwnd) noexcept;

  void SetAlpha(std::uint8_t alpha) override;

 private:
  HWND hwnd_;
};

}  // namespace osk::platform
