#pragma once

#include <chrono>
#include <optional>

#include "keyboard/core/Key.h"

namespace osk::core {

// Drives "hold a key to spam it": call OnKeyDown when a key is pressed,
// Update on every UI tick to find out whether a repeat should fire this
// tick, and OnKeyUp when released. Timestamp-driven rather than
// timer-owning, so it stays deterministic and testable with synthetic
// clocks — the platform/app layer supplies `now` on every call.
//
// Only one key can be held for repeat purposes at a time, matching a single
// mouse/touch pointer: a new OnKeyDown always replaces whatever was
// previously held.
class KeyRepeatController {
 public:
  // Negative delays/intervals are clamped to zero (immediate/continuous
  // repeat) rather than treated as invalid.
  KeyRepeatController(std::chrono::milliseconds initialDelay, std::chrono::milliseconds repeatInterval);

  void OnKeyDown(KeyId id, std::chrono::steady_clock::time_point now);

  // No-op if id isn't the currently held key (e.g. a stale/out-of-order
  // event for a key that was already superseded).
  void OnKeyUp(const KeyId& id);

  // Returns the key id that should fire a repeat activation this tick, if
  // any. Fires at most once per call, even if multiple intervals have
  // elapsed since the last call.
  std::optional<KeyId> Update(std::chrono::steady_clock::time_point now);

 private:
  struct HeldKey {
    KeyId id;
    std::chrono::steady_clock::time_point nextFireAt;
  };

  std::chrono::milliseconds initialDelay_;
  std::chrono::milliseconds repeatInterval_;
  std::optional<HeldKey> heldKey_;
};

}  // namespace osk::core
