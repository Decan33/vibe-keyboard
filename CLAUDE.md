# On-Screen Keyboard — Project Context

Accessibility-focused on-screen keyboard for Windows. This file is the running
source of truth for what's decided, what's built, and what's next — update it
whenever a decision is made or a milestone lands, don't let it go stale.

## Mission

Increase accessibility: users who can't use a physical keyboard should be able
to type reliably. This is assistive tech some users may depend on as their
only input method, so correctness and robustness are not optional.

## Non-negotiable bar (user-stated, 2026-08-07; extended 2026-08-08)

- Every `if`-branch and corner case gets a test, not just happy paths.
- Code must be understandable, maintainable, and readable by a human first.
- Architecture must stay open for extension (new layouts, languages, input
  backends) without rework.
- UI must stay responsive.
- "Bullet-proof and production-grade" — treat this as shipped assistive
  software, not a prototype.
- **Code must always be memory-safe.** RAII everywhere for OS handles (see
  `wil` in Stack below); no raw `new`/`delete`; fixed-size buffers passed to
  Win32 APIs are value-initialized, never left with uninitialized fields.
- **No security vulnerabilities introduced.** Prefer well-audited
  dependencies over hand-rolled parsing (e.g. `nlohmann-json` instead of a
  hand-rolled JSON parser for preferences); resolve OS paths via typed APIs
  (`SHGetKnownFolderPath`) instead of reading environment variables
  directly; never work around a real OS security boundary (e.g. UIPI
  blocking `SendInput`) even when it would make something more convenient.
- **Move logic to compile-time wherever it genuinely can be.** Enum-to-value
  mappings are `constexpr` `switch` statements with no `default` case, so an
  unhandled new enumerator is a compile error, not a silent runtime gap;
  pure value-conversion math gets `static_assert` regression checks
  alongside its runtime tests.

## Stack (decided 2026-08-07)

- **Language**: C++20.
- **GUI**: WinUI 3 (Windows App SDK) — native perf, built-in UI Automation
  support (important since this *is* an accessibility tool). Chosen over Qt
  (heavier, own accessibility bridge) and raw Win32 (more boilerplate, manual
  a11y wiring).
- **Build**: CMake + presets (`CMakePresets.json`), wired to vcpkg manifest
  mode (`toolchainFile` pointing at `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`).
  Both `configurePresets` set `OSK_WARNINGS_AS_ERRORS=ON`; `buildPresets`/
  `testPresets` explicitly set `configuration` (Debug/RelWithDebInfo) since
  the Visual Studio generator is multi-config and otherwise silently
  defaults to Debug regardless of `CMAKE_BUILD_TYPE` — a real gap found and
  fixed 2026-08-08 once building actually happened on Windows.
- **Third-party deps (added 2026-08-08, via vcpkg)**: `wil` (Microsoft's
  Windows Implementation Library) for RAII-wrapping raw Win32 handles
  (`HWINEVENTHOOK`, `CoTaskMemFree`'d strings) — direct answer to the
  memory-safety bar for handle lifetimes, Microsoft's own tool for exactly
  this, used throughout Windows Terminal/Sysinternals. `nlohmann-json` for
  `IPreferencesStore`'s file format — avoids hand-rolling a JSON parser,
  which is exactly the kind of code most likely to introduce a
  malformed-input parsing bug.
- **Tests**: GoogleTest + GoogleMock, pulled via CMake `FetchContent` (not
  vcpkg — simpler, matches Google's own recommended setup). Thin OS-call
  wrappers (SendInput, hotkeys, window placement, UI Automation) sit behind
  mockable interfaces in `osk::platform` so `osk::core` logic stays pure and
  fully unit-testable without touching real Win32 APIs; the real Win32
  bodies additionally get their own tests (see Status below) using a shared
  hidden-`HWND` fixture (`tests/platform/Win32WindowFixture.h`) for the ones
  that need real OS window state.
- **Quality gates**: `.clang-format`, `.clang-tidy`, CI (GitHub Actions) on
  `windows-latest` building Debug + RelWithDebInfo with warnings-as-errors,
  running the full CTest suite. CI triggers on `master` (fixed 2026-08-08 —
  it was configured for a `main` branch that never existed in this repo, so
  it had never actually run since the repo was pushed).
- **Layering**: `core` (OS-independent logic) → `platform` (thin Windows API
  wrappers behind interfaces, real Win32 bodies now implemented) → `app`
  (WinUI 3 shell, v1 slice scaffolded 2026-08-08).
- **Two independent build systems (since `src/app`, 2026-08-08).** WinUI 3's
  XAML/MSIX toolchain is MSBuild-only — it can't be driven from CMake.
  `cmake --build`/`ctest` still own `osk_core`/`osk_platform` + their tests,
  unchanged. `src/app/OnScreenKeyboardApp.vcxproj` is a separate,
  hand-authored MSBuild project (`msbuild src/app/OnScreenKeyboardApp.sln`)
  that compiles the *same* `osk::core`/`osk::platform` `.cpp`/`.h` files
  directly (not a linked `osk_core`/`osk_platform.lib`) — avoids any
  CRT/ABI mismatch risk between a CMake-invoked MSVC and an
  MSBuild-invoked one. NuGet packages (`Microsoft.WindowsAppSDK` 1.8.x,
  `Microsoft.Windows.CppWinRT`, `Microsoft.Windows.SDK.BuildTools`,
  `Microsoft.Windows.ImplementationLibrary`) are acquired via
  `<PackageReference>` in that vcxproj, independent of vcpkg.

## Functional requirements (received 2026-08-07)

**Priority:**
- Stays on top of other apps/windows — nothing should be able to cover it,
  including games. (Scope decision below: best-effort, not absolute.)
- QWERTY layout.

**Important:**
- Right-click on a letter capitalizes it (one-off, without touching Shift).
- Holding a key down repeats/spams it.
- Panels (numeric, F1–F12, arrow keys) can be added independently, in any
  order.
- Transparency is adjustable, for when the keyboard shouldn't block the view.
- Window size and key size are customizable.

**Features:**
- Dark/light theme switch.
- Shift+Enter as a direct shortcut key.

**Accessibility features:**
- Copy/Paste buttons that don't require a key combination.
- Word suggestions — a few, contextually logical for the sentence so far.
- A dwell/hover-to-activate typing mode (no click needed), similar in spirit
  to Android's swipe typing.

## Architecture decisions for the requirements above (2026-08-07)

Full design lives in the plan this was implemented from:
`~/.claude/plans/bubbly-purring-moon.md` (local to this machine, not part of
the repo — the decisions that matter long-term are captured here instead).

- **Always-on-top over games — best-effort, explicitly not absolute.**
  `SetWindowPos(HWND_TOPMOST)` + a foreground-change watchdog
  (`EVENT_SYSTEM_FOREGROUND`) covers windowed apps, borderless games, and
  most modern fullscreen games (Windows auto-converts most exclusive-
  fullscreen DX9-12 titles to borderless via "Fullscreen Optimizations"
  since Win10 22H2). True legacy exclusive-fullscreen with optimizations
  disabled is an OS/driver-level exception no ordinary window can overlay —
  confirmed this is the same limitation Microsoft's own PowerToys "Always on
  Top" utility has. DirectX-hooking was considered and rejected: fragile,
  and risks anti-cheat false-flags — inappropriate for accessibility
  software people depend on. No detection/warning UX for the exclusive-
  fullscreen edge case yet, by user's choice — revisit once the app is
  actually running and the gap can be felt in practice.
- **Word suggestions — pluggable interface, statistical model for now.**
  `core::IWordPredictor` is the abstraction; `core::NgramWordPredictor`
  (word-frequency trie for current-word completion + bigram/trigram
  frequency table for next-word ranking by preceding context) is the only
  implementation. Same approach Windows' own touch keyboard uses — fast,
  small, fully offline. The interface exists specifically so a heavier
  language model could be substituted later without touching call sites.
  Sourcing/licensing the real shipped word-frequency data is a separate
  follow-up, not yet done.
- **Copy/Paste buttons are not a clipboard-API integration.** They're just
  `KeyAction::SendVirtualKey` presets for Ctrl+C / Ctrl+V, injected the same
  way as any other key — the focused app handles the actual clipboard
  semantics once it receives the shortcut. Simpler, and consistent with
  "no clipboard API dependency needed."
- **Modifiers are sticky/latching**, standard virtual-keyboard UX: tapping
  Shift capitalizes the next key then releases; CapsLock is a persistent
  toggle. Right-click-to-capitalize is a one-off `ForceShiftOnce` override
  passed to `Dispatcher::ActivateKey`, not a mutation of the sticky state.
- **Timing-based controllers (`KeyRepeatController`, `DwellController`) are
  timestamp-driven, not timer-owning.** They take an explicit
  `steady_clock::time_point` on every `Update()` call; the platform/app
  layer drives the ticks. Keeps them deterministic and unit-testable with
  synthetic timestamps, no fake-clock injection machinery needed.
- **Theme/transparency/size/panel-arrangement are `Preferences`, not
  `KeyAction`s.** `KeyAction` is strictly "things injected into whichever
  app currently has OS focus"; settings-style buttons in the app UI write
  directly to `Preferences` instead of going through key dispatch.
- Our window must never steal OS focus from whatever the user is typing
  into — `IInputInjector` injects via `SendInput` to the current focus
  target regardless of whether our own window is focused.

## Repo layout

```
CMakeLists.txt            top-level build, wires everything together
CMakePresets.json         debug / release presets, vcpkg toolchain wiring
vcpkg.json                dependency manifest: wil, nlohmann-json (GoogleTest still comes via FetchContent)
cmake/CompilerWarnings.cmake   per-compiler warning flags, shared via osk_project_warnings
.clang-format / .clang-tidy / .editorconfig / .gitignore
.github/workflows/ci.yml  build + test on windows-latest (Debug, RelWithDebInfo), triggers on master

src/core/                 osk::core — OS-independent domain logic, no Windows dependency
  KeyAction / Key / Panel / KeyboardLayout   key & layout data model, built-in panels (BuiltInPanels.*)
  ModifierState / Dispatcher                 sticky-modifier resolution, right-click-to-capitalize override
  KeyRepeatController / DwellController      timestamp-driven hold-to-spam / hover-to-activate state machines
  Preferences                                validated user settings (theme, transparency, size, panels, dwell, key-repeat)
  IWordPredictor / NgramWordPredictor        pluggable word-suggestion interface + statistical n-gram implementation

src/platform/             osk::platform — thin Windows API wrappers behind interfaces, real Win32 bodies implemented
  IInputInjector / IAlwaysOnTopController / IWindowTransparency / IPreferencesStore / ISystemMetrics   interfaces
  Win32InputInjector                         SendInput; pure BuildInputEvents() builder kept separate + testable
  Win32AlwaysOnTopController                 SetWindowPos(HWND_TOPMOST) + SetWinEventHook foreground watchdog
  Win32WindowTransparency                    WS_EX_LAYERED + SetLayeredWindowAttributes
  Win32PreferencesStore                      JSON (nlohmann-json) under a caller-supplied directory, temp+rename writes
  Win32SystemMetrics                         SystemParametersInfo; pure SystemMetricsConversion.h math kept separate
  VirtualKeyMapping.h                        constexpr core::VirtualKey/ModifierKind -> Win32 VK_xx, no default case

src/app/                  osk::app — WinUI 3 shell, v1 slice (separate MSBuild project, see Stack above)
  OnScreenKeyboardApp.vcxproj/.sln   hand-authored single-project-MSIX C++ WinUI3 project, not in the CMake graph
  App.xaml(.h/.cpp)          Application entry point, activates MainWindow
  MainWindow.xaml(.h/.cpp)   thin XAML shell + code-behind: builds the QWERTY button grid from KeyboardLayout,
                             wires Click/RightTapped through Dispatcher+Win32InputInjector, owns Win32AlwaysOnTopController
  Package.appxmanifest       MSIX identity/manifest; hand-adds the InProcessServer/ActivatableClass entry for
                             OnScreenKeyboardApp.App (not auto-generated on this toolchain — see Status)

tests/                    mirrors src/, GoogleTest + GoogleMock via FetchContent, wired into CTest
tests/core/                one test file per core component above, every branch/corner case covered
tests/platform/mocks/      GoogleMock fakes for each osk::platform interface, test-only (not shipped headers)
tests/platform/PlatformMocksTest.cpp   proves the interface+mock seam compiles and is usable
tests/platform/Win32WindowFixture.h    shared hidden off-screen HWND fixture for the real-Win32-state tests
tests/platform/Win32*Test.cpp          one test file per real Win32Xxx class above
```

## Status

**Done:**
- Repo is git-initialized and pushed: `origin` is
  `https://github.com/Decan33/vibe-keyboard`, branch `master`.
- `osk::core` domain model implemented end-to-end for the v1 requirements
  below: key/panel/layout composition, modifier dispatch (including
  right-click-to-capitalize and Shift+Enter/Copy/Paste presets), hold-to-
  repeat and dwell-click state machines, validated Preferences, and a
  pluggable n-gram word predictor. 138 tests, one per branch/corner case
  per the project's testing bar.
- **`osk::platform` real Win32 bodies for all five interfaces (2026-08-08),
  built and tested on Windows** (this was the last major deferred item —
  work moved from a Mac, which can't build/verify Win32 code, to a Windows
  11 machine specifically for this):
  - `Win32InputInjector` — `SendInput`; `CharacterEvent` via
    `KEYEVENTF_UNICODE` (including UTF-16 surrogate-pair splitting for
    supplementary-plane codepoints — Unicode injection bypasses keyboard
    layout entirely, so no modifier keys involved); `VirtualKeyEvent` via
    modifier press → key press/release → modifier release in reverse order.
    Best-effort by design: `SendInput` can legitimately deliver fewer
    events under UIPI (User Interface Privilege Isolation) blocking input
    into a higher-privilege foreground window — a real security boundary,
    never worked around. The branch-heavy event-building logic is a pure,
    separately-tested `BuildInputEvents()` function with zero OS calls;
    only one test (`Win32InputInjectorTest`) touches real `SendInput`, and
    it first takes foreground+focus on its own throwaway off-screen control
    (skipping rather than proceeding if that fails) specifically so a test
    run can never inject a keystroke into whatever the person running the
    tests actually has focused.
  - `Win32AlwaysOnTopController` — `SetWindowPos(HWND_TOPMOST)` +
    `SetWinEventHook(EVENT_SYSTEM_FOREGROUND)` watchdog. The hook's
    `HWINEVENTHOOK` lives in a `wil::unique_hwineventhook` member so
    `Stop()`/the destructor always unhooks, even if a caller forgets —
    prevents a dangling hook callback pointing at a destroyed controller.
    Win32's hook API has no per-instance user-data passthrough, so the
    static callback routes through a single file-scope
    `std::atomic<Win32AlwaysOnTopController*>`; at most one instance may be
    `Start()`-ed at a time (matches reality — one keyboard window),
    enforced via `assert` in debug and a safe no-op degrade in release.
  - `Win32WindowTransparency` — `WS_EX_LAYERED` +
    `SetLayeredWindowAttributes`.
  - `Win32PreferencesStore` — JSON via `nlohmann-json`. Constructor takes
    the target directory explicitly (never hardcodes `%LOCALAPPDATA%` at
    the type level), so tests inject a throwaway temp directory; a separate
    `ResolveDefaultPreferencesDirectory()` free function (via
    `SHGetKnownFolderPath`, safer than reading the env var directly) is
    what `src/app` will actually call. `Save()` writes to a temp file then
    renames over the real one, so a crash mid-write can never leave a
    half-written file. `Load()` catches any parse/type failure narrowly
    (`nlohmann::json::exception`, not a blanket `catch (...)`) and falls
    back to full default `Preferences` — extending `Preferences`' own
    "never leave settings unusable" philosophy down to the file layer.
  - `Win32SystemMetrics` — `SystemParametersInfo`. The
    SPI-value-to-duration math lives in pure, `constexpr`-friendly
    functions (`SystemMetricsConversion.h`) with full boundary-value test
    coverage plus `static_assert` compile-time checks, isolated from the
    one-line OS call itself.
  - `VirtualKeyMapping.h` — `constexpr core::VirtualKey`/`ModifierKind` →
    Win32 `VK_xx` code, as `switch` statements with no `default` case: an
    unhandled new enumerator is a compile error, not a silent runtime gap.
  - Shared `tests/platform/Win32WindowFixture.h` creates a real,
    off-screen, non-activating `HWND` for the tests that need genuine Win32
    window state (ex-style bits, layered-window attributes, topmost
    z-order) without ever touching anything visible to whoever's running
    the tests.
  - **251 platform+core tests total, 100% passing, in both Debug and
    RelWithDebInfo with `-DOSK_WARNINGS_AS_ERRORS=ON`, zero warnings** —
    verified on this Windows 11 machine via VS Build Tools 2022 (Desktop
    C++ workload) + CMake + vcpkg.
- **`src/app` v1 slice (2026-08-08): a real, running WinUI 3 window that
  types into other apps.** Scope deliberately kept to a reviewable first
  cut — see Deliberately deferred below for what's intentionally not in it
  yet. Hand-authored from the actual VS-shipped
  `WinUI.Desktop.CppWinRT.SingleProjectPackagedApp` project template (found
  under the VS install's `Extensions` folder) rather than reconstructed
  from memory, since WinUI3+CMake integration turned out to be genuinely
  unsupported (researched and rejected — see below).
  - `MainWindow` builds the QWERTY button grid from
    `core::panels::Qwerty()`/`KeyboardLayout` in code-behind (XAML itself
    stays a single empty root `Grid`, since the layout is inherently
    data-driven). `Click` → `Dispatcher::ActivateKey(key)`; `RightTapped`
    (marked handled) → same call with `ActivationOverride::kForceShiftOnce`
    — fulfills the "Important" right-click-to-capitalize requirement.
    `ToggleModifier`/`ToggleCapsLock` naturally return `nullopt` from
    `ActivateKey` and inject nothing, so the QWERTY panel's Shift/Ctrl/Alt/
    CapsLock keys work through the exact same generic handler as every
    other key, no special-casing needed.
  - `Win32AlwaysOnTopController` is constructed with the window's real
    `HWND` (via the standard `IWindowNative::get_WindowHandle` interop
    call) and started unconditionally — fulfills the "Priority"
    stays-on-top requirement (no settings UI yet to make it optional).
  - **`WS_EX_NOACTIVATE` risk resolved.** Flagged in this file since
    2026-08-07 as unverified ("WinUI 3's internal pointer-routing behavior
    under `WS_EX_NOACTIVATE` hasn't been prototyped yet"). Now set on the
    real `HWND` and empirically confirmed: button clicks (including via UI
    Automation's `InvokePattern`, and a real click-to-type test into
    Notepad) still register correctly with the flag set.
  - **Verified against a real program**: with the app packaged, installed,
    and running, clicking its on-screen buttons typed real characters into
    a separate Notepad window (confirmed by reading Notepad's UI Automation
    `ValuePattern` value directly, not just visually) — the full pipeline
    (WinUI3 Button → `Dispatcher` → `Win32InputInjector` → `SendInput`) works
    end to end on this machine.
  - **Real toolchain problems hit and fixed** (VS Community 2026/MSBuild
    18.x is very new — several of these are version-skew issues, not
    project bugs): (1) `ResolveNuGetPackageAssets` failed with "does not
    reference UAP,Version=v10.0 framework" — a restore-vs-build
    `NuGetTargetMoniker` mismatch specific to this MSBuild version (traced
    to the actual `Microsoft.NuGet.targets`/`Microsoft.DesktopBridge.props`
    source to confirm); fixed by setting
    `<ResolveNuGetPackages>false</ResolveNuGetPackages>` directly (that
    target is a .NET-assembly-reference mechanism anyway — this project's
    native NuGet packages are consumed via the separately-generated
    `obj\*.vcxproj.nuget.g.props/targets` imports, unaffected by this).
    (2) The generated `AppxManifest.xml` never got an
    `<Extension Category="windows.activatableClass.inProcessServer">`
    registration for our own `OnScreenKeyboardApp.App` WinRT class (only
    WebView2's classes got auto-registered, from that NuGet package's own
    build props) — added explicitly in `Package.appxmanifest`, without
    which activation fails at the "COM ActivateExtension" phase. (3) WIL's
    `cppwinrt_helpers.h` pulls in a global `::Microsoft` (WRL) namespace
    that collides with `using namespace winrt;` + `using namespace
    Microsoft::UI::Xaml;` — fixed by qualifying as
    `winrt::Microsoft::UI::Xaml` everywhere. (4) The shared
    `osk::core`/`osk::platform` `.cpp` files needed
    `<PrecompiledHeader>NotUsing</PrecompiledHeader>` since they don't (and
    shouldn't) include this app's `pch.h`. (5) `pch.h` was missing
    `winrt/Microsoft.UI.Xaml.Input.h` (needed for
    `RightTappedRoutedEventArgs`).
  - **Known, unresolved gap: normal Start Menu / `shell:AppsFolder`
    activation currently fails** with a DCOM registration timeout ("COM
    ActivateExtension" phase), for a reason not yet root-caused. Testing
    instead used `Invoke-CommandInDesktopPackage` (which supplies package
    identity directly) — that path works reliably and is what was used for
    the Notepad verification above. **This needs to be revisited before
    the app is distributed to an actual end user**, since normal launch
    (Start Menu tile, taskbar pin, double-click) goes through the
    `shell:AppsFolder` path that currently fails.
  - **2026-08-08, later same day: this gap got worse and is now better
    understood.** After a layout/styling pass (see below), both the
    previously-working `Invoke-CommandInDesktopPackage` test path *and*
    taskbar/Start-Menu launch stopped merely hanging and started crashing
    outright — `Application Error` events for `OnScreenKeyboardApp.exe`
    faulting in `ucrtbase.dll` with exception code `0xc0000409`
    (`STATUS_STACK_BUFFER_OVERRUN`, the generic Windows `__fastfail` exit
    code — decoded via the WER event's `P10` parameter, which was `7` =
    `FAST_FAIL_FATAL_APP_EXIT`, i.e. an unhandled-exception-style fail-fast,
    not real memory corruption). **Root-caused as far as it can be from
    inside the app**: added raw-Win32 (`CreateFileW`/`WriteFile`, no
    WinRT/CRT exception machinery that could itself be swallowing an error)
    checkpoint logging at the literal first line of `App::App()` — before
    `InitializeComponent`, before anything — and it *never fires*. That
    proves the crash happens before any of this project's own code runs at
    all, which rules out the layout/styling changes (or any app code) as
    the cause and confirms this is purely a WindowsAppRuntime
    bootstrap/MSIX-activation-layer failure. The diagnostic logging was
    temporary and has been removed from `App.xaml.cpp`/`MainWindow.xaml.cpp`
    now that it's served its purpose. **Next actual diagnostic step needs
    an attached debugger** (e.g. Visual Studio's "Debug Installed App
    Package" for a packaged WinUI3 app) to see what's failing inside the
    bootstrap layer itself, since no in-process logging can run early
    enough to catch it.
  - Local sideload testing on this dev machine also needed: Developer Mode
    enabled (`Settings → Privacy & security → For developers` — a system
    setting, done by the user, not automated); the
    `Microsoft.VCLibs.x64.Debug.14.00.Desktop.appx` redistributable
    installed from the Windows SDK's `ExtensionSDKs` folder (Debug-config
    builds only — this is why final verification used a Release build
    instead, sidestepping the debug-CRT redistributable question
    entirely, which is more representative of real usage anyway).
- **2026-08-08 UX pass on user feedback from hands-on testing** ("packed
  like Windows' own OSK", proportional key sizes, Shift needs a pressed/
  latched visual indicator). `MainWindow::BuildKeyboardUi()` rewritten from
  nested fixed-size `StackPanel`s to a `Grid`-of-`Grid`s: rows and columns
  are star-sized so the layout stretches to fill the window edge-to-edge,
  and each key's column gets a relative weight (`KeyWidthWeight()`, keyed
  off the stable `Key::id` strings) proportional to its real-world
  importance — Space is `6×` a letter key's width, Enter/Backspace `2×`,
  Shift `2.25×`, etc. Modifier-toggle keys (`ToggleModifier`/
  `ToggleCapsLock` actions) are now tracked in a `modifierButtons_` list and
  repainted by `UpdateModifierVisuals()` after every key activation,
  reading straight from `ModifierState::IsLatched()`/`IsCapsLockOn()` —
  latched Shift now gets a highlighted background. Both changes compile
  clean (0 warnings) but **could not be visually verified this session**
  due to the activation-gap regression documented above; the code itself
  was ruled out as the cause via the checkpoint-logging investigation.
- Fixed two build-infra gaps found while doing the above, neither
  previously catchable without a real Windows/MSVC build: CI's
  `.github/workflows/ci.yml` triggered on a `main` branch that never
  existed (repo's default is `master` — CI had never actually run since
  the repo was pushed); `CMakePresets.json`'s `buildPresets`/`testPresets`
  didn't set `configuration`, so `cmake --build --preset release` silently
  built Debug binaries (the Visual Studio generator is multi-config and
  ignores `CMAKE_BUILD_TYPE`) — both fixed, both presets now build/test
  their actual intended configuration.
- **Earlier Mac-side verification** (superseded by the Windows build above,
  kept for context): this Mac's system Xcode Command Line Tools had a
  broken/incomplete libc++, worked around via Homebrew GCC 16, all 138
  `osk::core`-only tests passing there too.

**Deliberately deferred (not a gap, a decision):**
- From the `src/app` v1 slice specifically (kept small on purpose — see
  Status above): hold-to-repeat (`KeyRepeatController`) and dwell/
  hover-to-activate (`DwellController`), both of which need a UI-thread
  timer tick wired up; the Numeric/FunctionKeys/ArrowKeys panels and UI to
  add/remove/reorder them; `Preferences` persistence
  (`Win32PreferencesStore`) plus theme switch, transparency slider, and
  window/key size controls; word suggestions (`NgramWordPredictor`) UI;
  `Win32SystemMetrics` (feeds default repeat timing once repeat is wired).
- Real word-frequency/bigram dictionary data for `NgramWordPredictor`
  (currently only exercised with tiny synthetic test fixtures) — sourcing
  and licensing not yet done.
- Fullscreen-exclusive detection/warning UX — deferred per user, revisit
  once the app is running and the gap can be felt in practice.

## Next steps

1. Root-cause the `shell:AppsFolder`/Start Menu activation failure (see
   Status above) — **now blocks all launch paths, not just distribution**,
   since even the `Invoke-CommandInDesktopPackage` dev-loop workaround
   started fail-fasting the same way on 2026-08-08. Confirmed (via
   checkpoint logging, since removed) that the crash happens before any of
   this project's own code runs, so the fix has to come from attaching a
   real debugger to the activation/bootstrap phase (Visual Studio's "Debug
   Installed App Package"), not from more app-side logging. **This is now
   the single blocker on verifying the 2026-08-08 layout/highlight UX
   changes at all.**
2. Wire up hold-to-repeat and dwell-click: a UI-thread timer ticking
   `KeyRepeatController`/`DwellController` with real timestamps, plus
   per-key pointer press/hold and enter/leave tracking in `MainWindow`.
3. Add the Numeric/FunctionKeys/ArrowKeys panels and UI to add/remove/
   reorder active panels via `KeyboardLayout`.
4. Wire `Win32PreferencesStore` (load on startup, save on change) and add
   UI for theme, transparency, window/key size.
5. Source/license real word-frequency + bigram data for
   `NgramWordPredictor`, then build the suggestion-strip UI.
6. Confirm `.github/workflows/ci.yml` goes green on `windows-latest` now
   that its branch trigger is fixed (CI only covers the CMake
   `osk_core`/`osk_platform` build — `src/app`'s MSBuild build isn't wired
   into CI yet, worth adding once the activation gap above is fixed).
7. Any further functional requirements beyond the v1 batch above.
