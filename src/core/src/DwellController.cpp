#include "keyboard/core/DwellController.h"

#include <algorithm>
#include <utility>

namespace osk::core {

DwellController::DwellController(std::chrono::milliseconds dwellDuration)
    : dwellDuration_(std::max(dwellDuration, std::chrono::milliseconds::zero())) {}

void DwellController::BeginTracking(KeyId id, std::chrono::steady_clock::time_point now) {
  tracked_ = TrackedKey{.id = std::move(id), .enteredAt = now, .fireAt = now + dwellDuration_};
}

void DwellController::OnPointerEnter(KeyId id, std::chrono::steady_clock::time_point now) {
  BeginTracking(std::move(id), now);
}

void DwellController::OnPointerMove(KeyId id, std::chrono::steady_clock::time_point now) {
  if (tracked_.has_value() && tracked_->id == id) {
    return;
  }
  BeginTracking(std::move(id), now);
}

void DwellController::OnPointerLeave(const KeyId& id) {
  if (tracked_.has_value() && tracked_->id == id) {
    tracked_.reset();
  }
}

std::optional<KeyId> DwellController::Update(std::chrono::steady_clock::time_point now) {
  if (!tracked_.has_value() || now < tracked_->fireAt) {
    return std::nullopt;
  }

  const KeyId firedId = tracked_->id;
  tracked_.reset();
  return firedId;
}

float DwellController::ProgressFor(const KeyId& id, std::chrono::steady_clock::time_point now) const {
  if (!tracked_.has_value() || tracked_->id != id) {
    return 0.0F;
  }

  if (dwellDuration_ <= std::chrono::milliseconds::zero()) {
    return now >= tracked_->enteredAt ? 1.0F : 0.0F;
  }

  const std::chrono::duration<float, std::milli> elapsed = now - tracked_->enteredAt;
  const float ratio = elapsed.count() / static_cast<float>(dwellDuration_.count());
  return std::clamp(ratio, 0.0F, 1.0F);
}

}  // namespace osk::core
