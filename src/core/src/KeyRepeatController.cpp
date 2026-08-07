#include "keyboard/core/KeyRepeatController.h"

#include <algorithm>
#include <utility>

namespace osk::core {

namespace {
std::chrono::milliseconds ClampNonNegative(std::chrono::milliseconds duration) {
  return std::max(duration, std::chrono::milliseconds::zero());
}
}  // namespace

KeyRepeatController::KeyRepeatController(std::chrono::milliseconds initialDelay,
                                          std::chrono::milliseconds repeatInterval)
    : initialDelay_(ClampNonNegative(initialDelay)), repeatInterval_(ClampNonNegative(repeatInterval)) {}

void KeyRepeatController::OnKeyDown(KeyId id, std::chrono::steady_clock::time_point now) {
  heldKey_ = HeldKey{.id = std::move(id), .nextFireAt = now + initialDelay_};
}

void KeyRepeatController::OnKeyUp(const KeyId& id) {
  if (heldKey_.has_value() && heldKey_->id == id) {
    heldKey_.reset();
  }
}

std::optional<KeyId> KeyRepeatController::Update(std::chrono::steady_clock::time_point now) {
  if (!heldKey_.has_value() || now < heldKey_->nextFireAt) {
    return std::nullopt;
  }

  const KeyId firedId = heldKey_->id;
  heldKey_->nextFireAt = now + repeatInterval_;
  return firedId;
}

}  // namespace osk::core
