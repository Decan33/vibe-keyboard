#pragma once

#include <chrono>

namespace osk::platform {

// Reads OS-level defaults so Preferences can start from values consistent
// with the user's existing system settings. Real implementation wraps
// SystemParametersInfo(SPI_GETKEYBOARDDELAY / SPI_GETKEYBOARDSPEED).
class ISystemMetrics {
 public:
  virtual ~ISystemMetrics() = default;

  virtual std::chrono::milliseconds GetKeyboardRepeatDelay() const = 0;
  virtual std::chrono::milliseconds GetKeyboardRepeatInterval() const = 0;
};

}  // namespace osk::platform
