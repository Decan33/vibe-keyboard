#pragma once

#include <cstdint>

namespace osk::platform {

// Controls the keyboard window's opacity. Real implementation wraps
// WS_EX_LAYERED + SetLayeredWindowAttributes.
class IWindowTransparency {
 public:
  virtual ~IWindowTransparency() = default;

  // 0 = fully transparent, 255 = fully opaque.
  virtual void SetAlpha(std::uint8_t alpha) = 0;
};

}  // namespace osk::platform
