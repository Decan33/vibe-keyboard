#pragma once

#include <string>

#include "keyboard/core/KeyAction.h"

namespace osk::core {

// Stable identifier for a key, unique within the panel that owns it (e.g.
// "qwerty.q", "numeric.7"). Plain string rather than a distinct type: keys
// are authored data, not performance-sensitive handles.
using KeyId = std::string;

struct Key {
  KeyId id;
  std::string label;  // resting (unshifted) display text, e.g. "q", "Enter"
  KeyAction action;

  bool operator==(const Key&) const = default;
};

}  // namespace osk::core
