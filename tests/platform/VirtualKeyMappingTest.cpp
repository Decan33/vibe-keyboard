#include "keyboard/platform/VirtualKeyMapping.h"

#include <gtest/gtest.h>

namespace osk::platform {
namespace {

struct VirtualKeyCase {
  core::VirtualKey key;
  std::uint16_t expectedWin32Code;
  std::string name;
};

class VirtualKeyMappingTest : public ::testing::TestWithParam<VirtualKeyCase> {};

TEST_P(VirtualKeyMappingTest, MapsToExpectedWin32VirtualKeyCode) {
  EXPECT_EQ(ToWin32VirtualKey(GetParam().key), GetParam().expectedWin32Code);
}

// One case per osk::core::VirtualKey enumerator, matching the project's
// per-branch testing bar. Expected codes are the documented VK_xx values.
INSTANTIATE_TEST_SUITE_P(
    AllVirtualKeys, VirtualKeyMappingTest,
    ::testing::Values(
        VirtualKeyCase{core::VirtualKey::kA, 0x41, "A"}, VirtualKeyCase{core::VirtualKey::kB, 0x42, "B"},
        VirtualKeyCase{core::VirtualKey::kC, 0x43, "C"}, VirtualKeyCase{core::VirtualKey::kD, 0x44, "D"},
        VirtualKeyCase{core::VirtualKey::kE, 0x45, "E"}, VirtualKeyCase{core::VirtualKey::kF, 0x46, "F"},
        VirtualKeyCase{core::VirtualKey::kG, 0x47, "G"}, VirtualKeyCase{core::VirtualKey::kH, 0x48, "H"},
        VirtualKeyCase{core::VirtualKey::kI, 0x49, "I"}, VirtualKeyCase{core::VirtualKey::kJ, 0x4A, "J"},
        VirtualKeyCase{core::VirtualKey::kK, 0x4B, "K"}, VirtualKeyCase{core::VirtualKey::kL, 0x4C, "L"},
        VirtualKeyCase{core::VirtualKey::kM, 0x4D, "M"}, VirtualKeyCase{core::VirtualKey::kN, 0x4E, "N"},
        VirtualKeyCase{core::VirtualKey::kO, 0x4F, "O"}, VirtualKeyCase{core::VirtualKey::kP, 0x50, "P"},
        VirtualKeyCase{core::VirtualKey::kQ, 0x51, "Q"}, VirtualKeyCase{core::VirtualKey::kR, 0x52, "R"},
        VirtualKeyCase{core::VirtualKey::kS, 0x53, "S"}, VirtualKeyCase{core::VirtualKey::kT, 0x54, "T"},
        VirtualKeyCase{core::VirtualKey::kU, 0x55, "U"}, VirtualKeyCase{core::VirtualKey::kV, 0x56, "V"},
        VirtualKeyCase{core::VirtualKey::kW, 0x57, "W"}, VirtualKeyCase{core::VirtualKey::kX, 0x58, "X"},
        VirtualKeyCase{core::VirtualKey::kY, 0x59, "Y"}, VirtualKeyCase{core::VirtualKey::kZ, 0x5A, "Z"},

        VirtualKeyCase{core::VirtualKey::kDigit0, 0x30, "Digit0"},
        VirtualKeyCase{core::VirtualKey::kDigit1, 0x31, "Digit1"},
        VirtualKeyCase{core::VirtualKey::kDigit2, 0x32, "Digit2"},
        VirtualKeyCase{core::VirtualKey::kDigit3, 0x33, "Digit3"},
        VirtualKeyCase{core::VirtualKey::kDigit4, 0x34, "Digit4"},
        VirtualKeyCase{core::VirtualKey::kDigit5, 0x35, "Digit5"},
        VirtualKeyCase{core::VirtualKey::kDigit6, 0x36, "Digit6"},
        VirtualKeyCase{core::VirtualKey::kDigit7, 0x37, "Digit7"},
        VirtualKeyCase{core::VirtualKey::kDigit8, 0x38, "Digit8"},
        VirtualKeyCase{core::VirtualKey::kDigit9, 0x39, "Digit9"},

        VirtualKeyCase{core::VirtualKey::kBackspace, 0x08, "Backspace"},
        VirtualKeyCase{core::VirtualKey::kTab, 0x09, "Tab"},
        VirtualKeyCase{core::VirtualKey::kEnter, 0x0D, "Enter"},
        VirtualKeyCase{core::VirtualKey::kEscape, 0x1B, "Escape"},
        VirtualKeyCase{core::VirtualKey::kSpace, 0x20, "Space"},
        VirtualKeyCase{core::VirtualKey::kDelete, 0x2E, "Delete"},
        VirtualKeyCase{core::VirtualKey::kInsert, 0x2D, "Insert"},

        VirtualKeyCase{core::VirtualKey::kHome, 0x24, "Home"},
        VirtualKeyCase{core::VirtualKey::kEnd, 0x23, "End"},
        VirtualKeyCase{core::VirtualKey::kPageUp, 0x21, "PageUp"},
        VirtualKeyCase{core::VirtualKey::kPageDown, 0x22, "PageDown"},
        VirtualKeyCase{core::VirtualKey::kLeftArrow, 0x25, "LeftArrow"},
        VirtualKeyCase{core::VirtualKey::kUpArrow, 0x26, "UpArrow"},
        VirtualKeyCase{core::VirtualKey::kRightArrow, 0x27, "RightArrow"},
        VirtualKeyCase{core::VirtualKey::kDownArrow, 0x28, "DownArrow"},

        VirtualKeyCase{core::VirtualKey::kF1, 0x70, "F1"}, VirtualKeyCase{core::VirtualKey::kF2, 0x71, "F2"},
        VirtualKeyCase{core::VirtualKey::kF3, 0x72, "F3"}, VirtualKeyCase{core::VirtualKey::kF4, 0x73, "F4"},
        VirtualKeyCase{core::VirtualKey::kF5, 0x74, "F5"}, VirtualKeyCase{core::VirtualKey::kF6, 0x75, "F6"},
        VirtualKeyCase{core::VirtualKey::kF7, 0x76, "F7"}, VirtualKeyCase{core::VirtualKey::kF8, 0x77, "F8"},
        VirtualKeyCase{core::VirtualKey::kF9, 0x78, "F9"}, VirtualKeyCase{core::VirtualKey::kF10, 0x79, "F10"},
        VirtualKeyCase{core::VirtualKey::kF11, 0x7A, "F11"}, VirtualKeyCase{core::VirtualKey::kF12, 0x7B, "F12"}),
    [](const ::testing::TestParamInfo<VirtualKeyCase>& info) { return info.param.name; });

struct ModifierKindCase {
  core::ModifierKind modifier;
  std::uint16_t expectedWin32Code;
  std::string name;
};

class ModifierKindMappingTest : public ::testing::TestWithParam<ModifierKindCase> {};

TEST_P(ModifierKindMappingTest, MapsToExpectedWin32VirtualKeyCode) {
  EXPECT_EQ(ToWin32VirtualKey(GetParam().modifier), GetParam().expectedWin32Code);
}

INSTANTIATE_TEST_SUITE_P(
    AllModifierKinds, ModifierKindMappingTest,
    ::testing::Values(ModifierKindCase{core::ModifierKind::kShift, 0x10, "Shift"},
                       ModifierKindCase{core::ModifierKind::kControl, 0x11, "Control"},
                       ModifierKindCase{core::ModifierKind::kAlt, 0x12, "Alt"}),
    [](const ::testing::TestParamInfo<ModifierKindCase>& info) { return info.param.name; });

}  // namespace
}  // namespace osk::platform
