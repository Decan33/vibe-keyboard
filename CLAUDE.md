# On-Screen Keyboard — Project Context

Accessibility-focused on-screen keyboard for Windows. This file is the running
source of truth for what's decided, what's built, and what's next — update it
whenever a decision is made or a milestone lands, don't let it go stale.

## Mission

Increase accessibility: users who can't use a physical keyboard should be able
to type reliably. This is assistive tech some users may depend on as their
only input method, so correctness and robustness are not optional.

## Non-negotiable bar (user-stated, 2026-08-07)

- Every `if`-branch and corner case gets a test, not just happy paths.
- Code must be understandable, maintainable, and readable by a human first.
- Architecture must stay open for extension (new layouts, languages, input
  backends) without rework.
- UI must stay responsive.
- "Bullet-proof and production-grade" — treat this as shipped assistive
  software, not a prototype.

## Stack (decided 2026-08-07)

- **Language**: C++20.
- **GUI**: WinUI 3 (Windows App SDK) — native perf, built-in UI Automation
  support (important since this *is* an accessibility tool). Chosen over Qt
  (heavier, own accessibility bridge) and raw Win32 (more boilerplate, manual
  a11y wiring).
- **Build**: CMake + presets (`CMakePresets.json`). `vcpkg.json` manifest
  exists for future third-party deps but is currently empty.
- **Tests**: GoogleTest + GoogleMock, pulled via CMake `FetchContent` (not
  vcpkg — simpler, matches Google's own recommended setup). Thin OS-call
  wrappers (SendInput, hotkeys, window placement, UI Automation) are meant to
  sit behind mockable interfaces in `osk::platform` so `osk::core` logic
  stays pure and fully unit-testable without touching real Win32 APIs.
- **Quality gates**: `.clang-format`, `.clang-tidy`, CI (GitHub Actions) on
  `windows-latest` building Debug + RelWithDebInfo with warnings-as-errors,
  running the full CTest suite.
- **Layering**: `core` (OS-independent logic) → `platform` (thin Windows API
  wrappers behind interfaces) → `app` (WinUI 3 shell, not yet scaffolded).

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
CMakePresets.json         debug / release presets
vcpkg.json                dependency manifest (currently empty — GoogleTest comes via FetchContent instead)
cmake/CompilerWarnings.cmake   per-compiler warning flags, shared via osk_project_warnings
.clang-format / .clang-tidy / .editorconfig / .gitignore
.github/workflows/ci.yml  build + test on windows-latest (Debug, RelWithDebInfo)

src/core/                 osk::core — OS-independent domain logic, no Windows dependency
  KeyAction / Key / Panel / KeyboardLayout   key & layout data model, built-in panels (BuiltInPanels.*)
  ModifierState / Dispatcher                 sticky-modifier resolution, right-click-to-capitalize override
  KeyRepeatController / DwellController      timestamp-driven hold-to-spam / hover-to-activate state machines
  Preferences                                validated user settings (theme, transparency, size, panels, dwell, key-repeat)
  IWordPredictor / NgramWordPredictor        pluggable word-suggestion interface + statistical n-gram implementation

src/platform/             osk::platform — thin Windows API wrappers, interfaces only so far
  IInputInjector / IAlwaysOnTopController / IWindowTransparency / IPreferencesStore / ISystemMetrics
  Real Win32/WinRT bodies NOT YET WRITTEN — deferred to Windows-side work (see Status below)

src/app/                  NOT YET CREATED — WinUI 3 shell

tests/                    mirrors src/, GoogleTest + GoogleMock via FetchContent, wired into CTest
tests/core/                one test file per core component above, every branch/corner case covered
tests/platform/mocks/      GoogleMock fakes for each osk::platform interface, test-only (not shipped headers)
tests/platform/PlatformMocksTest.cpp   proves the interface+mock seam compiles and is usable
```

## Status

**Done:**
- Full build/test/CI scaffolding, committed to disk (git repo not yet
  initialized — user is doing that themselves).
- `osk::core` domain model implemented end-to-end for the v1 requirements
  below: key/panel/layout composition, modifier dispatch (including
  right-click-to-capitalize and Shift+Enter/Copy/Paste presets), hold-to-
  repeat and dwell-click state machines, validated Preferences, and a
  pluggable n-gram word predictor. 138 tests, one per branch/corner case
  per the project's testing bar (see design rationale above).
- `osk::platform` interface shapes + GoogleMock fakes for all five
  interfaces the current requirements need. Real Win32 bodies not written
  yet (see below).
- **Verified locally**: this Mac's system Xcode Command Line Tools has a
  broken/incomplete libc++ (confirmed system issue, not project — even a
  bare `#include <cstddef>` fails outside the project). Worked around by
  installing Homebrew GCC 16 (`brew install gcc`) as a local-only
  alternative toolchain — doesn't touch the system CLT install. With it,
  full clean builds + all 138 tests pass in both Debug and RelWithDebInfo
  with `-DOSK_WARNINGS_AS_ERRORS=ON`, zero warnings:
  ```
  CC=/opt/homebrew/bin/gcc-16 CXX=/opt/homebrew/bin/g++-16 \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOSK_WARNINGS_AS_ERRORS=ON
  cmake --build build -j
  ctest --test-dir build --output-on-failure
  ```
  CI (`windows-latest`, MSVC) hasn't run yet — no remote pushed. Should be
  verified once the repo is pushed.

**Deliberately deferred (not a gap, a decision):**
- Real Win32/WinRT bodies behind the five `osk::platform` interfaces
  (SendInput, topmost+foreground-watchdog, layered-window transparency,
  JSON preferences file, SystemParametersInfo) — need Windows to author and
  verify.
- `src/app` (WinUI 3 shell) — needs the platform bodies above plus Windows
  App SDK/MSIX project wiring only buildable on Windows/Visual Studio.
- Real word-frequency/bigram dictionary data for `NgramWordPredictor`
  (currently only exercised with tiny synthetic test fixtures) — sourcing
  and licensing not yet done.
- Fullscreen-exclusive detection/warning UX — deferred per user, revisit
  once the app is running and the gap can be felt in practice.

## Next steps

1. Real `osk::platform` Win32/WinRT implementations behind the five
   interfaces, on Windows.
2. Scaffold `src/app` (WinUI 3) and wire it to `osk::core`/`osk::platform`:
   render panels from `KeyboardLayout`, route pointer/mouse events
   (including right-click and press/hold/dwell tracking) into
   `Dispatcher`/`KeyRepeatController`/`DwellController`, apply
   `Preferences` (theme/transparency/size), tick the timing controllers
   from a UI-thread timer.
3. Source/license real word-frequency + bigram data for
   `NgramWordPredictor`.
4. User runs `git init` and pushes; confirm `.github/workflows/ci.yml` goes
   green on `windows-latest`.
5. Any further functional requirements beyond the v1 batch above.
