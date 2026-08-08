#include "keyboard/platform/Win32WindowTransparency.h"

#include <windows.h>

#include <gtest/gtest.h>

#include "Win32WindowFixture.h"

namespace osk::platform {
namespace {

using Win32WindowTransparencyTest = Win32WindowFixture;

TEST_F(Win32WindowTransparencyTest, SetAlphaAddsLayeredExStyleOnFirstCall) {
  EXPECT_EQ(GetWindowLongPtrW(Hwnd(), GWL_EXSTYLE) & WS_EX_LAYERED, 0);

  Win32WindowTransparency transparency(Hwnd());
  transparency.SetAlpha(128);

  EXPECT_NE(GetWindowLongPtrW(Hwnd(), GWL_EXSTYLE) & WS_EX_LAYERED, 0);
}

TEST_F(Win32WindowTransparencyTest, SetAlphaAppliesTheExactRequestedValue) {
  Win32WindowTransparency transparency(Hwnd());
  transparency.SetAlpha(77);

  BYTE alpha = 0;
  DWORD flags = 0;
  ASSERT_TRUE(GetLayeredWindowAttributes(Hwnd(), nullptr, &alpha, &flags));
  EXPECT_EQ(alpha, 77);
  EXPECT_NE(flags & LWA_ALPHA, 0);
}

TEST_F(Win32WindowTransparencyTest, FullyOpaqueIsApplied) {
  Win32WindowTransparency transparency(Hwnd());
  transparency.SetAlpha(255);

  BYTE alpha = 0;
  DWORD flags = 0;
  ASSERT_TRUE(GetLayeredWindowAttributes(Hwnd(), nullptr, &alpha, &flags));
  EXPECT_EQ(alpha, 255);
}

TEST_F(Win32WindowTransparencyTest, FullyTransparentIsApplied) {
  Win32WindowTransparency transparency(Hwnd());
  transparency.SetAlpha(0);

  BYTE alpha = 255;
  DWORD flags = 0;
  ASSERT_TRUE(GetLayeredWindowAttributes(Hwnd(), nullptr, &alpha, &flags));
  EXPECT_EQ(alpha, 0);
}

TEST_F(Win32WindowTransparencyTest, SecondCallDoesNotClearLayeredExStyle) {
  Win32WindowTransparency transparency(Hwnd());
  transparency.SetAlpha(200);
  transparency.SetAlpha(50);

  EXPECT_NE(GetWindowLongPtrW(Hwnd(), GWL_EXSTYLE) & WS_EX_LAYERED, 0);
  BYTE alpha = 0;
  ASSERT_TRUE(GetLayeredWindowAttributes(Hwnd(), nullptr, &alpha, nullptr));
  EXPECT_EQ(alpha, 50);
}

}  // namespace
}  // namespace osk::platform
