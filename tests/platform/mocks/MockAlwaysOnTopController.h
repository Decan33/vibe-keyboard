#pragma once

#include <gmock/gmock.h>

#include "keyboard/platform/IAlwaysOnTopController.h"

namespace osk::platform {

class MockAlwaysOnTopController : public IAlwaysOnTopController {
 public:
  MOCK_METHOD(void, Start, (), (override));
  MOCK_METHOD(void, Stop, (), (override));
};

}  // namespace osk::platform
