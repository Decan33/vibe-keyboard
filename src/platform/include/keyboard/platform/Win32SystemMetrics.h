#pragma once

#include "keyboard/platform/ISystemMetrics.h"

namespace osk::platform {

// Wraps SystemParametersInfo(SPI_GETKEYBOARDDELAY / SPI_GETKEYBOARDSPEED).
// The actual value math lives in SystemMetricsConversion.h as pure,
// independently-testable functions; this class is just the one-line OS call
// plus that conversion.
class Win32SystemMetrics : public ISystemMetrics {
 public:
  std::chrono::milliseconds GetKeyboardRepeatDelay() const override;
  std::chrono::milliseconds GetKeyboardRepeatInterval() const override;
};

}  // namespace osk::platform
