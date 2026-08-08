#pragma once

#include <cstdint>

#include "keyboard/core/KeyAction.h"

namespace osk::platform {

// Maps osk::core's platform-independent key vocabulary onto Win32 virtual-key
// codes (the values documented for VK_xx in winuser.h). Returned as a plain
// std::uint16_t (binary-compatible with Win32's WORD) rather than pulling
// <windows.h> into this public header just for the VK_xx macros — callers
// that actually touch SendInput/keybd_event already include <windows.h> and
// can cast freely.
//
// Both overloads switch over every enumerator with no `default` case: adding
// a new osk::core::VirtualKey or osk::core::ModifierKind without updating the
// mapping here is a compile error (-Wswitch, already enabled via
// osk_project_warnings), not a silent runtime gap.

constexpr std::uint16_t ToWin32VirtualKey(core::VirtualKey key) {
  switch (key) {
    case core::VirtualKey::kA: return 0x41;  // VK_A
    case core::VirtualKey::kB: return 0x42;  // VK_B
    case core::VirtualKey::kC: return 0x43;  // VK_C
    case core::VirtualKey::kD: return 0x44;  // VK_D
    case core::VirtualKey::kE: return 0x45;  // VK_E
    case core::VirtualKey::kF: return 0x46;  // VK_F
    case core::VirtualKey::kG: return 0x47;  // VK_G
    case core::VirtualKey::kH: return 0x48;  // VK_H
    case core::VirtualKey::kI: return 0x49;  // VK_I
    case core::VirtualKey::kJ: return 0x4A;  // VK_J
    case core::VirtualKey::kK: return 0x4B;  // VK_K
    case core::VirtualKey::kL: return 0x4C;  // VK_L
    case core::VirtualKey::kM: return 0x4D;  // VK_M
    case core::VirtualKey::kN: return 0x4E;  // VK_N
    case core::VirtualKey::kO: return 0x4F;  // VK_O
    case core::VirtualKey::kP: return 0x50;  // VK_P
    case core::VirtualKey::kQ: return 0x51;  // VK_Q
    case core::VirtualKey::kR: return 0x52;  // VK_R
    case core::VirtualKey::kS: return 0x53;  // VK_S
    case core::VirtualKey::kT: return 0x54;  // VK_T
    case core::VirtualKey::kU: return 0x55;  // VK_U
    case core::VirtualKey::kV: return 0x56;  // VK_V
    case core::VirtualKey::kW: return 0x57;  // VK_W
    case core::VirtualKey::kX: return 0x58;  // VK_X
    case core::VirtualKey::kY: return 0x59;  // VK_Y
    case core::VirtualKey::kZ: return 0x5A;  // VK_Z

    case core::VirtualKey::kDigit0: return 0x30;  // VK_0
    case core::VirtualKey::kDigit1: return 0x31;  // VK_1
    case core::VirtualKey::kDigit2: return 0x32;  // VK_2
    case core::VirtualKey::kDigit3: return 0x33;  // VK_3
    case core::VirtualKey::kDigit4: return 0x34;  // VK_4
    case core::VirtualKey::kDigit5: return 0x35;  // VK_5
    case core::VirtualKey::kDigit6: return 0x36;  // VK_6
    case core::VirtualKey::kDigit7: return 0x37;  // VK_7
    case core::VirtualKey::kDigit8: return 0x38;  // VK_8
    case core::VirtualKey::kDigit9: return 0x39;  // VK_9

    case core::VirtualKey::kBackspace: return 0x08;  // VK_BACK
    case core::VirtualKey::kTab: return 0x09;        // VK_TAB
    case core::VirtualKey::kEnter: return 0x0D;       // VK_RETURN
    case core::VirtualKey::kEscape: return 0x1B;      // VK_ESCAPE
    case core::VirtualKey::kSpace: return 0x20;       // VK_SPACE
    case core::VirtualKey::kDelete: return 0x2E;      // VK_DELETE
    case core::VirtualKey::kInsert: return 0x2D;      // VK_INSERT

    case core::VirtualKey::kHome: return 0x24;        // VK_HOME
    case core::VirtualKey::kEnd: return 0x23;         // VK_END
    case core::VirtualKey::kPageUp: return 0x21;      // VK_PRIOR
    case core::VirtualKey::kPageDown: return 0x22;    // VK_NEXT
    case core::VirtualKey::kLeftArrow: return 0x25;   // VK_LEFT
    case core::VirtualKey::kUpArrow: return 0x26;     // VK_UP
    case core::VirtualKey::kRightArrow: return 0x27;  // VK_RIGHT
    case core::VirtualKey::kDownArrow: return 0x28;   // VK_DOWN

    case core::VirtualKey::kF1: return 0x70;   // VK_F1
    case core::VirtualKey::kF2: return 0x71;   // VK_F2
    case core::VirtualKey::kF3: return 0x72;   // VK_F3
    case core::VirtualKey::kF4: return 0x73;   // VK_F4
    case core::VirtualKey::kF5: return 0x74;   // VK_F5
    case core::VirtualKey::kF6: return 0x75;   // VK_F6
    case core::VirtualKey::kF7: return 0x76;   // VK_F7
    case core::VirtualKey::kF8: return 0x77;   // VK_F8
    case core::VirtualKey::kF9: return 0x78;   // VK_F9
    case core::VirtualKey::kF10: return 0x79;  // VK_F10
    case core::VirtualKey::kF11: return 0x7A;  // VK_F11
    case core::VirtualKey::kF12: return 0x7B;  // VK_F12
  }
  // Unreachable while every enumerator above is handled; kept out of the
  // switch (no `default`) so an unhandled new enumerator fails to compile.
  return 0;
}

constexpr std::uint16_t ToWin32VirtualKey(core::ModifierKind modifier) {
  switch (modifier) {
    case core::ModifierKind::kShift: return 0x10;    // VK_SHIFT
    case core::ModifierKind::kControl: return 0x11;  // VK_CONTROL
    case core::ModifierKind::kAlt: return 0x12;      // VK_MENU
  }
  return 0;
}

static_assert(ToWin32VirtualKey(core::VirtualKey::kA) == 0x41);
static_assert(ToWin32VirtualKey(core::VirtualKey::kZ) == 0x5A);
static_assert(ToWin32VirtualKey(core::VirtualKey::kDigit0) == 0x30);
static_assert(ToWin32VirtualKey(core::VirtualKey::kEnter) == 0x0D);
static_assert(ToWin32VirtualKey(core::VirtualKey::kF12) == 0x7B);
static_assert(ToWin32VirtualKey(core::ModifierKind::kShift) == 0x10);

}  // namespace osk::platform
