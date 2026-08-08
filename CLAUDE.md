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
  (WinUI 3 shell, not yet scaffolded).

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

src/app/                  NOT YET CREATED — WinUI 3 shell

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
- `src/app` (WinUI 3 shell) — needs Windows App SDK/MSIX project wiring
  only buildable on Windows/Visual Studio; now unblocked since the toolchain
  (VS Build Tools, CMake, vcpkg) is set up on this machine.
- Real word-frequency/bigram dictionary data for `NgramWordPredictor`
  (currently only exercised with tiny synthetic test fixtures) — sourcing
  and licensing not yet done.
- Fullscreen-exclusive detection/warning UX — deferred per user, revisit
  once the app is running and the gap can be felt in practice.

## Next steps

1. Scaffold `src/app` (WinUI 3) and wire it to `osk::core`/`osk::platform`:
   render panels from `KeyboardLayout`, route pointer/mouse events
   (including right-click and press/hold/dwell tracking) into
   `Dispatcher`/`KeyRepeatController`/`DwellController`, apply
   `Preferences` (theme/transparency/size), tick the timing controllers
   from a UI-thread timer, construct the `Win32*` platform classes with the
   app's real `HWND`.
2. Source/license real word-frequency + bigram data for
   `NgramWordPredictor`.
3. Push the platform-layer work and confirm
   `.github/workflows/ci.yml` goes green on `windows-latest` now that its
   branch trigger is fixed.
4. Any further functional requirements beyond the v1 batch above.
