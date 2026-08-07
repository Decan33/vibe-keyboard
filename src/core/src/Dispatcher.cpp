#include "keyboard/core/Dispatcher.h"

namespace osk::core {

Dispatcher::Dispatcher(ModifierState& modifierState) : modifierState_(&modifierState) {}

std::optional<KeyEvent> Dispatcher::ActivateKey(const Key& key, ActivationOverride override) {
  return std::visit(
      [this, override](const auto& action) -> std::optional<KeyEvent> {
        using T = std::decay_t<decltype(action)>;

        if constexpr (std::is_same_v<T, TypeCharacter>) {
          bool useShifted;
          if (override == ActivationOverride::kForceShiftOnce) {
            useShifted = true;
          } else {
            const bool shiftActive = modifierState_->IsLatched(ModifierKind::kShift);
            const bool capsActive = action.capsLockApplies && modifierState_->IsCapsLockOn();
            useShifted = shiftActive != capsActive;
          }
          modifierState_->ConsumeLatches();
          return CharacterEvent{useShifted ? action.shifted : action.base};

        } else if constexpr (std::is_same_v<T, SendVirtualKey>) {
          const Modifier effectiveModifiers = action.modifiers | modifierState_->LatchedMask();
          modifierState_->ConsumeLatches();
          return VirtualKeyEvent{action.key, effectiveModifiers};

        } else if constexpr (std::is_same_v<T, ToggleModifier>) {
          modifierState_->ToggleLatch(action.modifier);
          return std::nullopt;

        } else {
          static_assert(std::is_same_v<T, ToggleCapsLock>);
          modifierState_->ToggleCapsLock();
          return std::nullopt;
        }
      },
      key.action);
}

}  // namespace osk::core
