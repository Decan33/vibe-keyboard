#include "keyboard/platform/Win32InputInjector.h"

#include <array>
#include <type_traits>
#include <variant>

#include "keyboard/platform/VirtualKeyMapping.h"

namespace osk::platform {

namespace {

INPUT MakeKeyboardInput(WORD virtualKey, WORD scanCode, DWORD flags) {
  INPUT input{};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = virtualKey;
  input.ki.wScan = scanCode;
  input.ki.dwFlags = flags;
  return input;
}

INPUT MakeUnicodeKeyInput(WCHAR utf16CodeUnit, bool keyUp) {
  DWORD flags = KEYEVENTF_UNICODE;
  if (keyUp) {
    flags |= KEYEVENTF_KEYUP;
  }
  return MakeKeyboardInput(0, static_cast<WORD>(utf16CodeUnit), flags);
}

INPUT MakeVirtualKeyInput(std::uint16_t virtualKey, bool keyUp) {
  return MakeKeyboardInput(virtualKey, 0, keyUp ? KEYEVENTF_KEYUP : 0);
}

std::size_t BuildCharacterEvents(const core::CharacterEvent& event,
                                  std::array<INPUT, kMaxInputEventsPerKeyEvent>& buffer) {
  std::size_t count = 0;

  if (event.codepoint > 0xFFFFU) {
    // Supplementary-plane: split into a UTF-16 surrogate pair, each unit
    // sent as its own keydown+keyup -- Unicode injection via
    // KEYEVENTF_UNICODE bypasses the keyboard layout entirely, so no
    // modifier keys are involved here.
    const char32_t offset = event.codepoint - 0x10000U;
    const auto high = static_cast<WCHAR>(0xD800U + (offset >> 10U));
    const auto low = static_cast<WCHAR>(0xDC00U + (offset & 0x3FFU));

    buffer[count++] = MakeUnicodeKeyInput(high, false);
    buffer[count++] = MakeUnicodeKeyInput(high, true);
    buffer[count++] = MakeUnicodeKeyInput(low, false);
    buffer[count++] = MakeUnicodeKeyInput(low, true);
  } else {
    const auto unit = static_cast<WCHAR>(event.codepoint);
    buffer[count++] = MakeUnicodeKeyInput(unit, false);
    buffer[count++] = MakeUnicodeKeyInput(unit, true);
  }

  return count;
}

constexpr core::Modifier ToModifierFlag(core::ModifierKind kind) {
  switch (kind) {
    case core::ModifierKind::kShift: return core::Modifier::kShift;
    case core::ModifierKind::kControl: return core::Modifier::kControl;
    case core::ModifierKind::kAlt: return core::Modifier::kAlt;
  }
  return core::Modifier::kNone;
}

// Press order is fixed (Shift, Control, Alt); release happens in the exact
// reverse order of whichever of those were actually pressed, matching
// standard nested-modifier press/release semantics.
constexpr std::array<core::ModifierKind, 3> kModifierPressOrder{
    core::ModifierKind::kShift, core::ModifierKind::kControl, core::ModifierKind::kAlt};

std::size_t BuildVirtualKeyEvents(const core::VirtualKeyEvent& event,
                                   std::array<INPUT, kMaxInputEventsPerKeyEvent>& buffer) {
  std::size_t count = 0;

  for (const core::ModifierKind modifier : kModifierPressOrder) {
    if (core::HasModifier(event.modifiers, ToModifierFlag(modifier))) {
      buffer[count++] = MakeVirtualKeyInput(ToWin32VirtualKey(modifier), false);
    }
  }

  const std::uint16_t vk = ToWin32VirtualKey(event.key);
  buffer[count++] = MakeVirtualKeyInput(vk, false);
  buffer[count++] = MakeVirtualKeyInput(vk, true);

  for (auto it = kModifierPressOrder.rbegin(); it != kModifierPressOrder.rend(); ++it) {
    if (core::HasModifier(event.modifiers, ToModifierFlag(*it))) {
      buffer[count++] = MakeVirtualKeyInput(ToWin32VirtualKey(*it), true);
    }
  }

  return count;
}

}  // namespace

std::size_t BuildInputEvents(const core::KeyEvent& event, std::array<INPUT, kMaxInputEventsPerKeyEvent>& buffer) {
  return std::visit(
      [&buffer](const auto& concreteEvent) -> std::size_t {
        using EventType = std::decay_t<decltype(concreteEvent)>;
        if constexpr (std::is_same_v<EventType, core::CharacterEvent>) {
          return BuildCharacterEvents(concreteEvent, buffer);
        } else {
          static_assert(std::is_same_v<EventType, core::VirtualKeyEvent>);
          return BuildVirtualKeyEvents(concreteEvent, buffer);
        }
      },
      event);
}

void Win32InputInjector::InjectKeyEvent(const core::KeyEvent& event) {
  std::array<INPUT, kMaxInputEventsPerKeyEvent> buffer{};
  const std::size_t count = BuildInputEvents(event, buffer);
  ::SendInput(static_cast<UINT>(count), buffer.data(), sizeof(INPUT));
}

}  // namespace osk::platform
