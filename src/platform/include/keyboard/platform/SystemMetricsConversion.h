#pragma once

#include <algorithm>
#include <chrono>

namespace osk::platform {

// Converts SPI_GETKEYBOARDDELAY's raw value (documented range 0-3, each step
// roughly 250ms) into a duration. Out-of-range input is clamped rather than
// treated as an error: a keyboard-repeat delay must always resolve to *some*
// sane value, never fail to start up over an unexpected OS return — same
// philosophy as osk::core::Preferences' own clamping setters.
constexpr std::chrono::milliseconds ToKeyboardRepeatDelay(int spiValue) noexcept {
  constexpr int kMinValue = 0;
  constexpr int kMaxValue = 3;
  constexpr int kMillisecondsPerStep = 250;

  const int clamped = std::clamp(spiValue, kMinValue, kMaxValue);
  return std::chrono::milliseconds{(clamped + 1) * kMillisecondsPerStep};
}

// Converts SPI_GETKEYBOARDSPEED's raw value (documented range 0-31, linear
// from ~2.5 to ~30 characters/second) into a repeat interval.
constexpr std::chrono::milliseconds ToKeyboardRepeatInterval(int spiValue) noexcept {
  constexpr int kMinValue = 0;
  constexpr int kMaxValue = 31;
  constexpr double kMinCharsPerSecond = 2.5;
  constexpr double kMaxCharsPerSecond = 30.0;

  const int clamped = std::clamp(spiValue, kMinValue, kMaxValue);
  const double charsPerSecond =
      kMinCharsPerSecond + (static_cast<double>(clamped) * (kMaxCharsPerSecond - kMinCharsPerSecond) /
                             static_cast<double>(kMaxValue));
  const double intervalMs = 1000.0 / charsPerSecond;
  // Manual round-to-nearest rather than std::round: keeps this usable in a
  // constant expression without relying on <cmath>'s constexpr guarantees.
  return std::chrono::milliseconds{static_cast<long long>(intervalMs + 0.5)};
}

static_assert(ToKeyboardRepeatDelay(0) == std::chrono::milliseconds{250});
static_assert(ToKeyboardRepeatDelay(3) == std::chrono::milliseconds{1000});
static_assert(ToKeyboardRepeatDelay(-5) == ToKeyboardRepeatDelay(0));
static_assert(ToKeyboardRepeatDelay(99) == ToKeyboardRepeatDelay(3));

static_assert(ToKeyboardRepeatInterval(0) == std::chrono::milliseconds{400});
static_assert(ToKeyboardRepeatInterval(31) == std::chrono::milliseconds{33});
static_assert(ToKeyboardRepeatInterval(-5) == ToKeyboardRepeatInterval(0));
static_assert(ToKeyboardRepeatInterval(99) == ToKeyboardRepeatInterval(31));

}  // namespace osk::platform
