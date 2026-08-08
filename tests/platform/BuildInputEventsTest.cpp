#include "keyboard/platform/Win32InputInjector.h"

#include <gtest/gtest.h>

#include "keyboard/platform/VirtualKeyMapping.h"

namespace osk::platform {
namespace {

using Buffer = std::array<INPUT, kMaxInputEventsPerKeyEvent>;

TEST(BuildInputEventsTest, BmpCharacterProducesOneKeydownKeyupPair) {
  Buffer buffer{};
  const core::KeyEvent event = core::CharacterEvent{.codepoint = U'a'};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 2U);

  EXPECT_EQ(buffer[0].type, static_cast<DWORD>(INPUT_KEYBOARD));
  EXPECT_EQ(buffer[0].ki.wVk, 0);
  EXPECT_EQ(buffer[0].ki.wScan, static_cast<WORD>(U'a'));
  EXPECT_EQ(buffer[0].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE));

  EXPECT_EQ(buffer[1].ki.wScan, static_cast<WORD>(U'a'));
  EXPECT_EQ(buffer[1].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));
}

TEST(BuildInputEventsTest, SupplementaryPlaneCharacterProducesSurrogatePair) {
  Buffer buffer{};
  // U+1F600 GRINNING FACE -> surrogate pair 0xD83D 0xDE00.
  const core::KeyEvent event = core::CharacterEvent{.codepoint = 0x1F600};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 4U);

  EXPECT_EQ(buffer[0].ki.wScan, 0xD83D);
  EXPECT_EQ(buffer[0].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE));
  EXPECT_EQ(buffer[1].ki.wScan, 0xD83D);
  EXPECT_EQ(buffer[1].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));

  EXPECT_EQ(buffer[2].ki.wScan, 0xDE00);
  EXPECT_EQ(buffer[2].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE));
  EXPECT_EQ(buffer[3].ki.wScan, 0xDE00);
  EXPECT_EQ(buffer[3].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));
}

TEST(BuildInputEventsTest, SupplementaryPlaneBoundaryJustAboveBmpSplitsCorrectly) {
  Buffer buffer{};
  // U+10000, the first supplementary-plane codepoint -> surrogates D800 DC00.
  const core::KeyEvent event = core::CharacterEvent{.codepoint = 0x10000};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 4U);
  EXPECT_EQ(buffer[0].ki.wScan, 0xD800);
  EXPECT_EQ(buffer[2].ki.wScan, 0xDC00);
}

TEST(BuildInputEventsTest, BmpBoundaryAtMaxDoesNotSplit) {
  Buffer buffer{};
  const core::KeyEvent event = core::CharacterEvent{.codepoint = 0xFFFF};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 2U);
  EXPECT_EQ(buffer[0].ki.wScan, 0xFFFF);
}

TEST(BuildInputEventsTest, VirtualKeyWithNoModifiersProducesJustKeydownKeyup) {
  Buffer buffer{};
  const core::KeyEvent event = core::VirtualKeyEvent{.key = core::VirtualKey::kEnter, .modifiers = core::Modifier::kNone};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 2U);
  const WORD enterVk = ToWin32VirtualKey(core::VirtualKey::kEnter);
  EXPECT_EQ(buffer[0].ki.wVk, enterVk);
  EXPECT_EQ(buffer[0].ki.dwFlags, 0U);
  EXPECT_EQ(buffer[1].ki.wVk, enterVk);
  EXPECT_EQ(buffer[1].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));
}

TEST(BuildInputEventsTest, VirtualKeyWithOneModifierWrapsKeyInModifierPressRelease) {
  Buffer buffer{};
  const core::KeyEvent event =
      core::VirtualKeyEvent{.key = core::VirtualKey::kLeftArrow, .modifiers = core::Modifier::kShift};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 4U);
  const WORD shiftVk = ToWin32VirtualKey(core::ModifierKind::kShift);
  const WORD leftVk = ToWin32VirtualKey(core::VirtualKey::kLeftArrow);

  EXPECT_EQ(buffer[0].ki.wVk, shiftVk);
  EXPECT_EQ(buffer[0].ki.dwFlags, 0U);
  EXPECT_EQ(buffer[1].ki.wVk, leftVk);
  EXPECT_EQ(buffer[1].ki.dwFlags, 0U);
  EXPECT_EQ(buffer[2].ki.wVk, leftVk);
  EXPECT_EQ(buffer[2].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));
  EXPECT_EQ(buffer[3].ki.wVk, shiftVk);
  EXPECT_EQ(buffer[3].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));
}

TEST(BuildInputEventsTest, VirtualKeyWithTwoModifiersPressesInFixedOrderAndReleasesInReverse) {
  Buffer buffer{};
  const core::KeyEvent event = core::VirtualKeyEvent{
      .key = core::VirtualKey::kC, .modifiers = core::Modifier::kShift | core::Modifier::kControl};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, 6U);
  const WORD shiftVk = ToWin32VirtualKey(core::ModifierKind::kShift);
  const WORD controlVk = ToWin32VirtualKey(core::ModifierKind::kControl);
  const WORD cVk = ToWin32VirtualKey(core::VirtualKey::kC);

  // Press order: Shift, Control.
  EXPECT_EQ(buffer[0].ki.wVk, shiftVk);
  EXPECT_EQ(buffer[0].ki.dwFlags, 0U);
  EXPECT_EQ(buffer[1].ki.wVk, controlVk);
  EXPECT_EQ(buffer[1].ki.dwFlags, 0U);

  EXPECT_EQ(buffer[2].ki.wVk, cVk);
  EXPECT_EQ(buffer[3].ki.wVk, cVk);
  EXPECT_EQ(buffer[3].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));

  // Release order: Control, then Shift (reverse of press order).
  EXPECT_EQ(buffer[4].ki.wVk, controlVk);
  EXPECT_EQ(buffer[4].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));
  EXPECT_EQ(buffer[5].ki.wVk, shiftVk);
  EXPECT_EQ(buffer[5].ki.dwFlags, static_cast<DWORD>(KEYEVENTF_KEYUP));
}

TEST(BuildInputEventsTest, VirtualKeyWithAllThreeModifiersProducesTheMaximumEventCount) {
  Buffer buffer{};
  const core::KeyEvent event = core::VirtualKeyEvent{
      .key = core::VirtualKey::kDelete,
      .modifiers = core::Modifier::kShift | core::Modifier::kControl | core::Modifier::kAlt};
  const std::size_t count = BuildInputEvents(event, buffer);

  ASSERT_EQ(count, kMaxInputEventsPerKeyEvent);
  const WORD shiftVk = ToWin32VirtualKey(core::ModifierKind::kShift);
  const WORD controlVk = ToWin32VirtualKey(core::ModifierKind::kControl);
  const WORD altVk = ToWin32VirtualKey(core::ModifierKind::kAlt);

  EXPECT_EQ(buffer[0].ki.wVk, shiftVk);
  EXPECT_EQ(buffer[1].ki.wVk, controlVk);
  EXPECT_EQ(buffer[2].ki.wVk, altVk);
  // buffer[3], buffer[4] are the key itself.
  EXPECT_EQ(buffer[5].ki.wVk, altVk);
  EXPECT_EQ(buffer[6].ki.wVk, controlVk);
  EXPECT_EQ(buffer[7].ki.wVk, shiftVk);
}

TEST(BuildInputEventsTest, AllInputStructsAreKeyboardType) {
  Buffer buffer{};
  const core::KeyEvent event = core::VirtualKeyEvent{
      .key = core::VirtualKey::kTab, .modifiers = core::Modifier::kShift | core::Modifier::kAlt};
  const std::size_t count = BuildInputEvents(event, buffer);

  for (std::size_t i = 0; i < count; ++i) {
    EXPECT_EQ(buffer[i].type, static_cast<DWORD>(INPUT_KEYBOARD)) << "index " << i;
  }
}

}  // namespace
}  // namespace osk::platform
