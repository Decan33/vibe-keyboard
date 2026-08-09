========================================================================
    OnScreenKeyboardApp Project Overview
========================================================================

WinUI 3 desktop shell for the accessibility on-screen keyboard. See the
repo root CLAUDE.md for full project context, decisions, and status.

Not part of the CMake build graph: WinUI 3's XAML/MSIX toolchain is
MSBuild-only. Build with:

    msbuild OnScreenKeyboardApp.sln /p:Configuration=Debug /p:Platform=x64

osk::core / osk::platform sources are compiled directly into this project
(see the .vcxproj) rather than linked as a prebuilt library, to avoid any
CRT/ABI mismatch between this MSBuild-invoked MSVC and the CMake-invoked
one used for osk_core/osk_platform's own tests.

========================================================================
Learn more about Windows App SDK here:
https://docs.microsoft.com/windows/apps/windows-app-sdk/
Learn more about WinUI3 here:
https://docs.microsoft.com/windows/apps/winui/winui3/
Learn more about C++/WinRT here:
http://aka.ms/cppwinrt/
========================================================================
