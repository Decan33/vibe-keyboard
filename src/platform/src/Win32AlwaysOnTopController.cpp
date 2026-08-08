#include "keyboard/platform/Win32AlwaysOnTopController.h"

#include <atomic>
#include <cassert>

namespace osk::platform {

namespace {
// At most one instance is ever active, matching reality (one keyboard
// window). Encapsulated entirely in this .cpp -- not a class member, since
// WinEventProc must be routable from a plain callback with no user-data
// passthrough.
std::atomic<Win32AlwaysOnTopController*> g_activeInstance{nullptr};
}  // namespace

Win32AlwaysOnTopController::Win32AlwaysOnTopController(HWND hwnd) noexcept : hwnd_(hwnd) {}

Win32AlwaysOnTopController::~Win32AlwaysOnTopController() { Stop(); }

void Win32AlwaysOnTopController::Start() {
  ReassertTopmost();

  Win32AlwaysOnTopController* expected = nullptr;
  const bool claimed = g_activeInstance.compare_exchange_strong(expected, this);
  // A programmer-error contract violation (two Start()-ed instances at
  // once), not a runtime condition -- caught in debug builds; in release,
  // degrades to "this Start() call still applied topmost once, but the
  // watchdog belongs to whichever instance claimed it first" rather than
  // corrupting that other instance's state.
  assert(claimed && "Win32AlwaysOnTopController::Start() called while another instance is already active");
  if (!claimed) {
    return;
  }

  hook_.reset(SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, &WinEventProc, 0, 0,
                               WINEVENT_OUTOFCONTEXT));
}

void Win32AlwaysOnTopController::Stop() {
  hook_.reset();

  Win32AlwaysOnTopController* self = this;
  g_activeInstance.compare_exchange_strong(self, nullptr);
}

void Win32AlwaysOnTopController::ReassertTopmost() const {
  SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CALLBACK Win32AlwaysOnTopController::WinEventProc(HWINEVENTHOOK /*hook*/, DWORD event, HWND /*hwnd*/,
                                                         LONG /*idObject*/, LONG /*idChild*/, DWORD /*eventThread*/,
                                                         DWORD /*eventTime*/) {
  if (event != EVENT_SYSTEM_FOREGROUND) {
    return;
  }
  if (Win32AlwaysOnTopController* instance = g_activeInstance.load(); instance != nullptr) {
    instance->ReassertTopmost();
  }
}

}  // namespace osk::platform
