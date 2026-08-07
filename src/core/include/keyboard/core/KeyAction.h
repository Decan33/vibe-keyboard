#pragma once

#include <cstdint>
#include <variant>

namespace osk::core {

enum class VirtualKey : std::uint8_t {
  kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM,
  kN, kO, kP, kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ,

  kDigit0, kDigit1, kDigit2, kDigit3, kDigit4,
  kDigit5, kDigit6, kDigit7, kDigit8, kDigit9,

  kBackspace, kTab, kEnter, kEscape, kSpace, kDelete, kInsert,

  kHome, kEnd, kPageUp, kPageDown, kLeftArrow, kRightArrow, kUpArrow, kDownArrow,

  kF1, kF2, kF3, kF4, kF5, kF6, kF7, kF8, kF9, kF10, kF11, kF12,
};

enum class ModifierKind : std::uint8_t {
  kShift,
  kControl,
  kAlt,
};

enum class Modifier : std::uint8_t {
  kNone = 0,
  kShift = 1U << 0,
  kControl = 1U << 1,
  kAlt = 1U << 2,
};

constexpr Modifier operator|(Modifier lhs, Modifier rhs) {
  return static_cast<Modifier>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

constexpr Modifier operator&(Modifier lhs, Modifier rhs) {
  return static_cast<Modifier>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

constexpr Modifier& operator|=(Modifier& lhs, Modifier rhs) {
  lhs = lhs | rhs;
  return lhs;
}

constexpr bool HasModifier(Modifier mask, Modifier flag) {
  return (mask & flag) != Modifier::kNone;
}

// Emits a raw Unicode codepoint. `base` is sent with no effective shift;
// `shifted` is sent when Shift (or CapsLock, for keys where it applies) is
// active. Storing both explicitly avoids locale-dependent case folding and
// lets symbol keys (e.g. "1" / "!") work the same way as letters.
struct TypeCharacter {
  char32_t base;
  char32_t shifted;
  // Whether CapsLock alone (without Shift) selects `shifted`. True for
  // letters, false for digits/symbols — matches physical keyboard behavior.
  bool capsLockApplies = false;

  bool operator==(const TypeCharacter&) const = default;
};

// Emits a non-character key, optionally with baked-in modifiers (e.g. the
// Shift+Enter shortcut key, or the Copy/Paste presets for Ctrl+C/Ctrl+V).
struct SendVirtualKey {
  VirtualKey key;
  Modifier modifiers = Modifier::kNone;

  bool operator==(const SendVirtualKey&) const = default;
};

// Toggles a one-shot (latching) Shift/Control/Alt modifier. Does not emit
// anything by itself.
struct ToggleModifier {
  ModifierKind modifier;

  bool operator==(const ToggleModifier&) const = default;
};

// Toggles the persistent CapsLock state. Does not emit anything by itself.
struct ToggleCapsLock {
  bool operator==(const ToggleCapsLock&) const = default;
};

using KeyAction = std::variant<TypeCharacter, SendVirtualKey, ToggleModifier, ToggleCapsLock>;

}  // namespace osk::core
