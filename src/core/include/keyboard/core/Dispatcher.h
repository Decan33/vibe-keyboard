#pragma once

#include <optional>

#include "keyboard/core/Key.h"
#include "keyboard/core/KeyEvent.h"
#include "keyboard/core/ModifierState.h"

namespace osk::core {

// Resolves a key activation, combined with the current modifier state, into
// the concrete event that should be injected — or nullopt for actions that
// only mutate modifier state (ToggleModifier, ToggleCapsLock).
class Dispatcher {
 public:
  explicit Dispatcher(ModifierState& modifierState);

  std::optional<KeyEvent> ActivateKey(const Key& key,
                                       ActivationOverride override = ActivationOverride::kNone);

 private:
  ModifierState* modifierState_;
};

}  // namespace osk::core
