#include "keyboard/platform/Win32SystemMetrics.h"

#include <windows.h>

#include "keyboard/platform/SystemMetricsConversion.h"

namespace osk::platform {

std::chrono::milliseconds Win32SystemMetrics::GetKeyboardRepeatDelay() const {
  int spiValue = 0;
  // A failed call leaves spiValue at 0, which ToKeyboardRepeatDelay still
  // clamps to a valid, sane duration -- never propagated as an error.
  SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &spiValue, 0);
  return ToKeyboardRepeatDelay(spiValue);
}

std::chrono::milliseconds Win32SystemMetrics::GetKeyboardRepeatInterval() const {
  int spiValue = 0;
  SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &spiValue, 0);
  return ToKeyboardRepeatInterval(spiValue);
}

}  // namespace osk::platform
