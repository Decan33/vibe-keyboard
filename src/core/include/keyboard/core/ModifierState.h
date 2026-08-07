#pragma once

#include <cstdint>

#include "keyboard/core/KeyAction.h"

namespace osk::core {

// How a key activation should treat Shift, independent of the latched
// state. Used by right-click-to-capitalize, which forces a shifted
// activation without touching the sticky Shift latch.
enum class ActivationOverride : std::uint8_t {
  kNone,
  kForceShiftOnce,
};

// Tracks the on-screen keyboard's modifier state. Shift/Control/Alt are
// one-shot latches: toggled on by a ToggleModifier activation, and released
// together the next time a non-modifier key (TypeCharacter or
// SendVirtualKey) is activated. CapsLock is a persistent toggle, unaffected
// by latch consumption.
class ModifierState {
 public:
  void ToggleLatch(ModifierKind modifier);
  bool IsLatched(ModifierKind modifier) const;

  // Union of all currently latched modifiers, for combining with a key's
  // own baked-in modifiers.
  Modifier LatchedMask() const;

  void ToggleCapsLock();
  bool IsCapsLockOn() const;

  // Releases all latched Shift/Control/Alt state. CapsLock is unaffected.
  void ConsumeLatches();

 private:
  bool shiftLatched_ = false;
  bool controlLatched_ = false;
  bool altLatched_ = false;
  bool capsLockOn_ = false;
};

}  // namespace osk::core
