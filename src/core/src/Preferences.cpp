#include "keyboard/core/Preferences.h"

#include <algorithm>
#include <utility>

namespace osk::core {

Theme Preferences::GetTheme() const {
  return theme_;
}

void Preferences::SetTheme(Theme theme) {
  theme_ = theme;
}

int Preferences::GetTransparencyPercent() const {
  return transparencyPercent_;
}

void Preferences::SetTransparencyPercent(int percent) {
  transparencyPercent_ = std::clamp(percent, kMinTransparencyPercent, kMaxTransparencyPercent);
}

float Preferences::GetWindowScale() const {
  return windowScale_;
}

void Preferences::SetWindowScale(float scale) {
  windowScale_ = std::clamp(scale, kMinWindowScale, kMaxWindowScale);
}

float Preferences::GetKeySize() const {
  return keySize_;
}

void Preferences::SetKeySize(float size) {
  keySize_ = std::clamp(size, kMinKeySize, kMaxKeySize);
}

const std::vector<PanelId>& Preferences::GetActivePanelOrder() const {
  return activePanelOrder_;
}

void Preferences::SetActivePanelOrder(std::vector<PanelId> order) {
  std::vector<PanelId> deduped;
  deduped.reserve(order.size());
  for (auto& id : order) {
    if (std::find(deduped.begin(), deduped.end(), id) == deduped.end()) {
      deduped.push_back(std::move(id));
    }
  }
  activePanelOrder_ = std::move(deduped);
}

bool Preferences::IsDwellEnabled() const {
  return dwellEnabled_;
}

void Preferences::SetDwellEnabled(bool enabled) {
  dwellEnabled_ = enabled;
}

std::chrono::milliseconds Preferences::GetDwellDelay() const {
  return dwellDelay_;
}

void Preferences::SetDwellDelay(std::chrono::milliseconds delay) {
  dwellDelay_ = std::clamp(delay, kMinDwellDelay, kMaxDwellDelay);
}

std::optional<std::chrono::milliseconds> Preferences::GetKeyRepeatInitialDelay() const {
  return keyRepeatInitialDelay_;
}

void Preferences::SetKeyRepeatInitialDelay(std::optional<std::chrono::milliseconds> delay) {
  if (delay.has_value()) {
    delay = std::clamp(*delay, kMinKeyRepeatInitialDelay, kMaxKeyRepeatInitialDelay);
  }
  keyRepeatInitialDelay_ = delay;
}

std::optional<std::chrono::milliseconds> Preferences::GetKeyRepeatInterval() const {
  return keyRepeatInterval_;
}

void Preferences::SetKeyRepeatInterval(std::optional<std::chrono::milliseconds> interval) {
  if (interval.has_value()) {
    interval = std::clamp(*interval, kMinKeyRepeatInterval, kMaxKeyRepeatInterval);
  }
  keyRepeatInterval_ = interval;
}

}  // namespace osk::core
