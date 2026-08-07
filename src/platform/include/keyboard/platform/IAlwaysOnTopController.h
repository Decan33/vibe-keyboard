#pragma once

namespace osk::platform {

// Keeps the keyboard window above other windows: best-effort, not absolute
// (see CLAUDE.md — true exclusive-fullscreen games are an OS-level
// exception no ordinary window can overlay). Real implementation wraps
// SetWindowPos(HWND_TOPMOST) plus an EVENT_SYSTEM_FOREGROUND watchdog that
// reasserts it whenever the foreground window changes.
class IAlwaysOnTopController {
 public:
  virtual ~IAlwaysOnTopController() = default;

  virtual void Start() = 0;
  virtual void Stop() = 0;
};

}  // namespace osk::platform
