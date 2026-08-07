#pragma once

#include <gmock/gmock.h>

#include "keyboard/platform/IPreferencesStore.h"

namespace osk::platform {

class MockPreferencesStore : public IPreferencesStore {
 public:
  MOCK_METHOD(core::Preferences, Load, (), (const, override));
  MOCK_METHOD(void, Save, (const core::Preferences& preferences), (override));
};

}  // namespace osk::platform
