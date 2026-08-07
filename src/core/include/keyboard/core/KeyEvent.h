#pragma once

#include <variant>

#include "keyboard/core/KeyAction.h"

namespace osk::core {

// What Dispatcher resolves a key activation into: the concrete thing that
// should be injected into whichever app currently holds OS focus.
struct CharacterEvent {
  char32_t codepoint;

  bool operator==(const CharacterEvent&) const = default;
};

struct VirtualKeyEvent {
  VirtualKey key;
  Modifier modifiers;

  bool operator==(const VirtualKeyEvent&) const = default;
};

using KeyEvent = std::variant<CharacterEvent, VirtualKeyEvent>;

}  // namespace osk::core
