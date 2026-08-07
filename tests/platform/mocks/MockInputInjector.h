#pragma once

#include <gmock/gmock.h>

#include "keyboard/platform/IInputInjector.h"

namespace osk::platform {

class MockInputInjector : public IInputInjector {
 public:
  MOCK_METHOD(void, InjectKeyEvent, (const core::KeyEvent& event), (override));
};

}  // namespace osk::platform
