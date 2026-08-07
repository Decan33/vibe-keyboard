#pragma once

#include "keyboard/core/Preferences.h"

namespace osk::platform {

// Persists user Preferences across app runs. Real implementation wraps a
// JSON file under %LOCALAPPDATA%.
class IPreferencesStore {
 public:
  virtual ~IPreferencesStore() = default;

  virtual core::Preferences Load() const = 0;
  virtual void Save(const core::Preferences& preferences) = 0;
};

}  // namespace osk::platform
