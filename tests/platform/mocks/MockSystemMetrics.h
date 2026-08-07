#pragma once

#include <gmock/gmock.h>

#include "keyboard/platform/ISystemMetrics.h"

namespace osk::platform {

class MockSystemMetrics : public ISystemMetrics {
 public:
  MOCK_METHOD(std::chrono::milliseconds, GetKeyboardRepeatDelay, (), (const, override));
  MOCK_METHOD(std::chrono::milliseconds, GetKeyboardRepeatInterval, (), (const, override));
};

}  // namespace osk::platform
