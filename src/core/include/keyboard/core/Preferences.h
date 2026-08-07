#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "keyboard/core/Panel.h"

namespace osk::core {

enum class Theme : std::uint8_t {
  kSystem,
  kLight,
  kDark,
};

// User-configurable settings, independent of any persistence mechanism.
// Every setter clamps out-of-range input to the nearest valid value rather
// than rejecting it — a bad or corrupted stored value should never leave
// assistive-tech settings in an unusable state.
class Preferences {
 public:
  static constexpr int kMinTransparencyPercent = 0;
  static constexpr int kMaxTransparencyPercent = 100;

  static constexpr float kMinWindowScale = 0.5F;
  static constexpr float kMaxWindowScale = 3.0F;

  static constexpr float kMinKeySize = 0.5F;
  static constexpr float kMaxKeySize = 2.0F;

  static constexpr std::chrono::milliseconds kMinDwellDelay{100};
  static constexpr std::chrono::milliseconds kMaxDwellDelay{5000};

  static constexpr std::chrono::milliseconds kMinKeyRepeatInitialDelay{50};
  static constexpr std::chrono::milliseconds kMaxKeyRepeatInitialDelay{3000};

  static constexpr std::chrono::milliseconds kMinKeyRepeatInterval{16};
  static constexpr std::chrono::milliseconds kMaxKeyRepeatInterval{2000};

  Theme GetTheme() const;
  void SetTheme(Theme theme);

  int GetTransparencyPercent() const;
  void SetTransparencyPercent(int percent);

  float GetWindowScale() const;
  void SetWindowScale(float scale);

  float GetKeySize() const;
  void SetKeySize(float size);

  const std::vector<PanelId>& GetActivePanelOrder() const;
  // Duplicate ids are dropped, keeping each id's first occurrence.
  void SetActivePanelOrder(std::vector<PanelId> order);

  bool IsDwellEnabled() const;
  void SetDwellEnabled(bool enabled);

  std::chrono::milliseconds GetDwellDelay() const;
  void SetDwellDelay(std::chrono::milliseconds delay);

  // nullopt means "use the platform's system keyboard-repeat default".
  std::optional<std::chrono::milliseconds> GetKeyRepeatInitialDelay() const;
  void SetKeyRepeatInitialDelay(std::optional<std::chrono::milliseconds> delay);

  std::optional<std::chrono::milliseconds> GetKeyRepeatInterval() const;
  void SetKeyRepeatInterval(std::optional<std::chrono::milliseconds> interval);

 private:
  Theme theme_ = Theme::kSystem;
  int transparencyPercent_ = kMaxTransparencyPercent;
  float windowScale_ = 1.0F;
  float keySize_ = 1.0F;
  std::vector<PanelId> activePanelOrder_;
  bool dwellEnabled_ = false;
  std::chrono::milliseconds dwellDelay_{800};
  std::optional<std::chrono::milliseconds> keyRepeatInitialDelay_;
  std::optional<std::chrono::milliseconds> keyRepeatInterval_;
};

}  // namespace osk::core
