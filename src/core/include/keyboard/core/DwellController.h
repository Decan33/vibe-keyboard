#pragma once

#include <chrono>
#include <optional>

#include "keyboard/core/Key.h"

namespace osk::core {

// Drives hover-to-activate ("dwell") typing: rest the pointer over a key for
// a set duration and it activates without a click. Timestamp-driven, like
// KeyRepeatController — the platform/app layer supplies `now` on every call
// so this stays deterministic and testable.
//
// Only one key can be dwelt on at a time, matching a single pointer.
// Activation is one-shot: once a key fires, it must be re-entered before it
// can fire again (unlike key-repeat, dwell doesn't keep firing while held).
class DwellController {
 public:
  // Negative durations are clamped to zero (instant activation).
  explicit DwellController(std::chrono::milliseconds dwellDuration);

  // Starts (or restarts) dwelling on id, discarding any previously tracked
  // key's progress.
  void OnPointerEnter(KeyId id, std::chrono::steady_clock::time_point now);

  // If id is already the tracked key, this is a no-op — dwelling continues
  // uninterrupted. Otherwise behaves like OnPointerEnter, so callers may
  // report hover purely via a stream of move events without needing
  // explicit enter/leave transitions.
  void OnPointerMove(KeyId id, std::chrono::steady_clock::time_point now);

  // No-op if id isn't the currently tracked key.
  void OnPointerLeave(const KeyId& id);

  // Returns the key id that completed its dwell this tick, if any.
  std::optional<KeyId> Update(std::chrono::steady_clock::time_point now);

  // Fraction of the dwell duration elapsed for id, clamped to [0, 1]. 0 if
  // id isn't the currently tracked key.
  float ProgressFor(const KeyId& id, std::chrono::steady_clock::time_point now) const;

 private:
  struct TrackedKey {
    KeyId id;
    std::chrono::steady_clock::time_point enteredAt;
    std::chrono::steady_clock::time_point fireAt;
  };

  void BeginTracking(KeyId id, std::chrono::steady_clock::time_point now);

  std::chrono::milliseconds dwellDuration_;
  std::optional<TrackedKey> tracked_;
};

}  // namespace osk::core
