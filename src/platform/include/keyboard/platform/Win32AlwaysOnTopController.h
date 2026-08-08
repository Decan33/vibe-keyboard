#pragma once

#include <windows.h>

#include <wil/resource.h>

#include "keyboard/platform/IAlwaysOnTopController.h"

namespace osk::platform {

// Wraps SetWindowPos(HWND_TOPMOST) plus a SetWinEventHook(EVENT_SYSTEM_FOREGROUND)
// watchdog that reasserts topmost whenever the foreground window changes.
// Best-effort, not absolute -- see CLAUDE.md: true legacy exclusive-fullscreen
// games are an OS-level exception no ordinary window, topmost or not, can
// overlay.
//
// Does not own hwnd -- the caller (src/app) owns the window's lifetime; this
// class must not outlive it. At most one instance may be Start()-ed at a
// time, matching reality (one keyboard window): Win32's hook API has no
// per-instance user-data passthrough, so the callback routes through a
// single file-scope active-instance pointer.
class Win32AlwaysOnTopController : public IAlwaysOnTopController {
 public:
  explicit Win32AlwaysOnTopController(HWND hwnd) noexcept;
  ~Win32AlwaysOnTopController() override;

  Win32AlwaysOnTopController(const Win32AlwaysOnTopController&) = delete;
  Win32AlwaysOnTopController& operator=(const Win32AlwaysOnTopController&) = delete;

  void Start() override;
  void Stop() override;

 private:
  void ReassertTopmost() const;
  static void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                                     DWORD eventThread, DWORD eventTime);

  HWND hwnd_;
  wil::unique_hwineventhook hook_;
};

}  // namespace osk::platform
