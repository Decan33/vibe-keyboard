#pragma once

#include "keyboard/core/KeyEvent.h"

namespace osk::platform {

// Injects synthetic input into whatever application currently holds OS
// focus. Never assumes the keyboard's own window is focused — our window
// must not steal focus from the app being typed into. Real implementation
// wraps SendInput.
class IInputInjector {
 public:
  virtual ~IInputInjector() = default;

  virtual void InjectKeyEvent(const core::KeyEvent& event) = 0;
};

}  // namespace osk::platform
