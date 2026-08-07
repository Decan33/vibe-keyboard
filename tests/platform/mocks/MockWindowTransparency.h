#pragma once

#include <gmock/gmock.h>

#include "keyboard/platform/IWindowTransparency.h"

namespace osk::platform {

class MockWindowTransparency : public IWindowTransparency {
 public:
  MOCK_METHOD(void, SetAlpha, (std::uint8_t alpha), (override));
};

}  // namespace osk::platform
