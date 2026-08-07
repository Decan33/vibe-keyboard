#include "keyboard/core/ModifierState.h"

namespace osk::core {

void ModifierState::ToggleLatch(ModifierKind modifier) {
  switch (modifier) {
    case ModifierKind::kShift:
      shiftLatched_ = !shiftLatched_;
      return;
    case ModifierKind::kControl:
      controlLatched_ = !controlLatched_;
      return;
    case ModifierKind::kAlt:
      altLatched_ = !altLatched_;
      return;
  }
}

bool ModifierState::IsLatched(ModifierKind modifier) const {
  switch (modifier) {
    case ModifierKind::kShift:
      return shiftLatched_;
    case ModifierKind::kControl:
      return controlLatched_;
    case ModifierKind::kAlt:
      return altLatched_;
  }
  return false;
}

Modifier ModifierState::LatchedMask() const {
  Modifier mask = Modifier::kNone;
  if (shiftLatched_) {
    mask |= Modifier::kShift;
  }
  if (controlLatched_) {
    mask |= Modifier::kControl;
  }
  if (altLatched_) {
    mask |= Modifier::kAlt;
  }
  return mask;
}

void ModifierState::ToggleCapsLock() {
  capsLockOn_ = !capsLockOn_;
}

bool ModifierState::IsCapsLockOn() const {
  return capsLockOn_;
}

void ModifierState::ConsumeLatches() {
  shiftLatched_ = false;
  controlLatched_ = false;
  altLatched_ = false;
}

}  // namespace osk::core
