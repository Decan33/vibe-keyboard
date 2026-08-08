#pragma once

#include <windows.h>

#include <array>
#include <cstddef>

#include "keyboard/platform/IInputInjector.h"

namespace osk::platform {

// Largest number of INPUT structs any single osk::core::KeyEvent can expand
// into. A CharacterEvent on a supplementary-plane codepoint needs 4 (two
// UTF-16 surrogate code units, each a keydown+keyup pair). A VirtualKeyEvent
// with all three modifiers (Shift+Control+Alt) needs 8: 3 modifier
// keydowns + 1 key keydown + 1 key keyup + 3 modifier keyups. The larger of
// the two, 8, is the bound used here.
inline constexpr std::size_t kMaxInputEventsPerKeyEvent = 8;

// Builds the raw SendInput event sequence for a KeyEvent without touching
// the OS. Kept separate from Win32InputInjector, and exposed here rather
// than hidden in the .cpp, specifically so this branch-heavy logic
// (surrogate-pair splitting, modifier press/release ordering) is fully
// unit-testable by inspecting the built INPUT structs directly -- with zero
// risk of a test actually injecting a keystroke into whatever window
// happens to have real OS focus.
//
// Returns the number of INPUT entries written into `buffer`, always
// <= kMaxInputEventsPerKeyEvent.
std::size_t BuildInputEvents(const core::KeyEvent& event, std::array<INPUT, kMaxInputEventsPerKeyEvent>& buffer);

// Wraps SendInput. Best-effort by design: SendInput can legitimately
// deliver fewer events than requested when UIPI (User Interface Privilege
// Isolation) blocks input into a higher-privilege foreground window -- a
// real Windows security boundary, not a bug, and must never be worked
// around. Never steals or requires OS focus itself: SendInput always
// targets whatever currently has it, per the project's "never steal focus
// from the app being typed into" requirement.
class Win32InputInjector : public IInputInjector {
 public:
  void InjectKeyEvent(const core::KeyEvent& event) override;
};

}  // namespace osk::platform
