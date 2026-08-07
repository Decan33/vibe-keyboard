# Project Memory — On-Screen Keyboard

This file is a full context dump, written so it travels with the repo (via
git) across machines instead of living only in Claude Code's local,
machine-specific state (the auto-memory system under
`~/.claude/projects/.../memory/`, and planning documents under
`~/.claude/plans/`, neither of which follow the repo when you switch
machines).

**Relationship to [`CLAUDE.md`](CLAUDE.md):** `CLAUDE.md` is the actively
maintained living doc — trust it first for current status, decisions, and
next steps, and keep updating it as work progresses. This file is a deeper,
point-in-time archive: the full rationale, research findings, and design
detail behind those decisions, including things `CLAUDE.md` intentionally
keeps concise. It won't be kept in sync automatically — if the two ever
disagree, `CLAUDE.md` is more current.

---

## How this project started

User-stated mission (2026-08-07): an accessibility-focused on-screen
keyboard for Windows. Assistive tech some users may depend on as their only
input method — correctness and robustness are treated as non-negotiable,
not aspirational.

Non-negotiable bar, stated before any code was written:
- Every `if`-branch and corner case gets a test, not just happy paths.
- Code must be understandable, maintainable, and readable by a human first.
- Architecture must stay open for extension (new layouts, languages, input
  backends) without rework.
- UI must stay responsive.
- "Bullet-proof and production-grade" — treat this as shipped assistive
  software, not a prototype.

Library/framework choices were explicitly delegated: "Libraries for GUI,
tests, other operations are on you. You are the expert, and I will accept
them or not." The user also gave feedback that has shaped how work
proceeds since: they'd rather scaffold ahead of detailed requirements than
wait idle, but explicitly did **not** want `git init` run — they're handling
repo creation themselves. They approved the stack proposal below as-is on
first pass, no changes requested.

## Stack decision and why (2026-08-07)

- **Language: C++20** — user's explicit choice, for performance.
- **GUI: WinUI 3 (Windows App SDK)** — chosen over Qt (heavier, brings its
  own accessibility bridge rather than the OS-native one) and raw Win32
  (more boilerplate, fully manual accessibility wiring). WinUI 3 gives
  native perf and built-in UI Automation support, which matters because
  this tool's whole purpose is accessibility.
- **Build: CMake + `CMakePresets.json`.** `vcpkg.json` manifest scaffolded
  for future third-party deps but deliberately left empty — nothing needed
  one yet.
- **Tests: GoogleTest + GoogleMock via CMake `FetchContent`**, not vcpkg —
  this is Google's own currently-recommended setup and avoids a vcpkg
  bootstrap dependency just for the test framework. Thin OS-call wrappers
  (SendInput, hotkeys, window placement, UI Automation) sit behind
  mockable interfaces in `osk::platform` specifically so `osk::core` stays
  pure and fully unit-testable without touching real Win32 APIs — this is
  the architectural move that makes the "every branch tested" bar
  achievable at all for a Windows-native app.
- **Quality gates:** `.clang-format`, `.clang-tidy`, GitHub Actions CI on
  `windows-latest`, Debug + RelWithDebInfo, warnings-as-errors, full CTest
  suite on every push.
- **Layering:** `core` (OS-independent logic) → `platform` (thin Windows
  API wrappers behind interfaces) → `app` (WinUI 3 shell).

## Environment quirk: this Mac's toolchain

Confirmed early and re-hit later: this Mac's Xcode Command Line Tools has a
broken/incomplete libc++ —
`/Library/Developer/CommandLineTools/usr/include/c++/v1/` is missing
`cstddef`, `vector`, `string`, etc. Confirmed as a genuine system issue, not
a project issue: a bare `#include <cstddef>` fails even in a throwaway file
outside the project, with no other C++ toolchain (no Homebrew LLVM/GCC)
installed as a fallback initially.

**Resolved by installing Homebrew GCC 16** (`brew install gcc`, giving
`/opt/homebrew/bin/g++-16`) — a local, additive, non-destructive fix that
doesn't touch the system Xcode CLT install. Build with:
```
CC=/opt/homebrew/bin/gcc-16 CXX=/opt/homebrew/bin/g++-16 \
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOSK_WARNINGS_AS_ERRORS=ON
```
If this repo moves to a different Mac with the same broken-CLT symptom,
the same fix applies. On Windows or in CI this doesn't come up at all
(MSVC has no equivalent issue).

## v1 functional requirements (received 2026-08-07)

**Priority:** stays on top of other apps/windows including games (see
fullscreen research below for what's actually achievable); QWERTY layout.

**Important:** right-click on a letter capitalizes it (one-off); holding a
key down repeats/spams it; panels (numeric, F1–F12, arrow keys) addable
independently, any order; adjustable transparency; customizable window/key
size.

**Features:** dark/light theme switch; Shift+Enter as a direct shortcut
key.

**Accessibility features:** Copy/Paste buttons without needing a key
combination; word suggestions (a few, contextually logical for the
sentence so far); a dwell/hover-to-activate typing mode (no click needed,
similar in spirit to Android swipe typing).

## Research that shaped the design

**Always-on-top over fullscreen games.** Windows lets any window go
always-on-top via `SetWindowPos(HWND_TOPMOST)`. This reliably works over
windowed apps, borderless-windowed games, and — importantly — the
*majority* of modern fullscreen games, because Windows 10 22H2+/11
auto-converts most exclusive-fullscreen DX9–12 titles to borderless behind
the scenes via "Fullscreen Optimizations," even though the game believes
it's still in exclusive fullscreen. True legacy exclusive-fullscreen with
optimizations disabled is a hard OS/driver-level exception: the game takes
direct control of the display output, and no ordinary window — topmost or
not — can be composited above it. Confirmed via search that this is the
*exact same limitation* Microsoft's own PowerToys "Always on Top" utility
has (its docs say so directly). The only workaround for true exclusive
fullscreen is DirectX hooking, which is fragile (breaks across DX
versions, no working solution beyond DX9) and risks anti-cheat systems
flagging it — rejected as inappropriate for accessibility software people
depend on.

**User's explicit decision on this:** best-effort only, ship it, don't
build fullscreen-exclusive detection/warning UX yet — "we can think about
it later, when the product is ran." Revisit once the app exists and the
gap (if any) is actually felt.

**WinUI 3 transparency/click-through/always-on-top.** Confirmed feasible:
WinUI 3 apps have an underlying HWND reachable via Win32 interop
(`IWindowNative`/`GetWindowHandle`), so standard Win32 techniques apply —
`WS_EX_LAYERED` + `SetLayeredWindowAttributes` for opacity,
`WS_EX_LAYERED | WS_EX_TRANSPARENT` for click-through regions,
`SetWinEventHook(EVENT_SYSTEM_FOREGROUND, ...)` to reassert topmost
immediately when the foreground window changes (more responsive than
blind polling). This is the real-implementation plan for
`IWindowTransparency` and `IAlwaysOnTopController`.

**Word prediction library research.** Looked at `predict4all` (an
open-source, Apache-2.0, purpose-built next-word-prediction library
explicitly designed for virtual keyboards/AAC systems, low memory
footprint). Rejected: it's Java/JVM-only with no C++ binding — not usable
from this codebase. This is *why* `NgramWordPredictor` is a hand-written
statistical model rather than a pulled-in dependency: nothing suitable
existed for C++, and it wasn't worth introducing a JNI-style bridge for
one feature. The user separately confirmed a statistical n-gram approach
was the right scope (see below) rather than a full offline language model.

**User's explicit decision on word prediction:** wanted an abstract
interface with a statistical n-gram model behind it now, but structured so
"if necessary, [a] language model can be plugged in" later — this is
exactly what `IWordPredictor` / `NgramWordPredictor` give: the interface
is the only thing call sites depend on.

## Architecture, in more detail than `CLAUDE.md` keeps

**Requirement → design element mapping:**

| Requirement | Design element |
|---|---|
| Stays on top, even over games | `platform::IAlwaysOnTopController` — topmost + foreground-change watchdog. Interface only; real body needs Windows. |
| QWERTY layout | `core::panels::Qwerty()` — mandatory base panel of every `KeyboardLayout` |
| Right-click capitalizes | `Dispatcher::ActivateKey(key, ActivationOverride::kForceShiftOnce)` — one-off, doesn't touch sticky Shift state |
| Hold spams a key | `core::KeyRepeatController` — timestamp-driven (`Update(now)`), not timer-owning |
| Addable panels, any order | `core::Panel` + `core::KeyboardLayout::AddPanel/RemovePanel/ReorderPanel` |
| Transparency adjustment | `Preferences::transparencyPercent_` (clamped 0–100) + `platform::IWindowTransparency::SetAlpha` |
| Window/key size | `Preferences::windowScale_` (0.5–3.0) / `keySize_` (0.5–2.0), both clamped |
| Dark/light theme | `Preferences::theme_` (`Theme::kSystem/kLight/kDark`); actual rendering is app-layer, not built yet |
| Shift+Enter | Just a `Key` whose action is `SendVirtualKey{kEnter, Modifier::kShift}` — no new mechanism needed |
| Copy/Paste buttons | Same pattern: `SendVirtualKey{kC, Modifier::kControl}` / `{kV, Modifier::kControl}` — no clipboard API touched, the focused app handles the shortcut itself |
| Word suggestions | `core::IWordPredictor` (interface) + `core::NgramWordPredictor` (statistical implementation) |
| Dwell-click mode | `core::DwellController` — same timestamp-driven shape as `KeyRepeatController`, but one-shot instead of repeating |

**Key design decisions and their reasoning** (all shipped in code as of
2026-08-07, all covered by tests):

- **Modifiers are sticky/latching, not held.** Tapping Shift arms the next
  non-modifier key activation, then auto-releases; CapsLock is a separate
  persistent toggle. This is standard virtual/on-screen-keyboard UX (same
  as mobile keyboards) and was chosen because holding a physical button
  down isn't really meaningful when the "keyboard" itself is mouse/dwell
  driven.
- **`TypeCharacter` stores both the unshifted and shifted codepoint
  explicitly** (`base`, `shifted`, plus a `capsLockApplies` bool), rather
  than doing runtime Unicode case-folding. Chosen deliberately to avoid
  locale-dependent case-folding bugs entirely — correctness over cleverness
  for something as foundational as "what character does this key send."
  `capsLockApplies` exists because CapsLock affects letters but not
  digits/symbols on real keyboards (CapsLock+1 still gives "1", not "!") —
  this is explicit, tested data rather than inferred behavior.
- **`SendVirtualKey`'s baked-in modifiers union with whatever's currently
  latched**, rather than override it. So latching Shift then clicking the
  Shift+Enter preset key is still just Shift+Enter (idempotent), but
  latching Shift then clicking a plain Left-Arrow key correctly produces
  Shift+Left (useful for text selection) even though the arrow key's own
  baked-in modifiers are `kNone`.
- **`ActivationOverride::kForceShiftOnce` (right-click) is ignored for
  non-`TypeCharacter` actions**, not an error — defined, tested behavior,
  since right-click-to-capitalize is conceptually a letter-only gesture and
  the app layer is expected to only offer it on letter keys, but core
  shouldn't misbehave if called anyway.
- **`KeyRepeatController` / `DwellController` are timestamp-driven, not
  timer-owning** — they take an explicit `steady_clock::time_point` on
  every `Update()` call rather than starting their own OS timer. The
  platform/app layer is responsible for ticking them (e.g. from a UI-thread
  timer at ~60fps). This was the single biggest lever for hitting the
  "every corner case tested" bar on timing-sensitive logic: tests just feed
  synthetic timestamps, no fake-clock injection or real sleeping needed,
  and behavior at exact boundaries is exactly reproducible.
- **`DwellController` firing is one-shot**; `KeyRepeatController` firing
  repeats on an interval. Different by design: dwelling should not keep
  re-activating every tick once it fires (the user must leave and re-enter
  the key), but holding a key down for repeat obviously should keep firing.
- **Only one key can be tracked at a time by either controller** — matches
  a single mouse/pointer input device. A second `OnKeyDown`/`OnPointerEnter`
  always replaces whatever was previously tracked; a stale `OnKeyUp`/
  `OnPointerLeave` for a since-superseded key is a defined no-op, not an
  error.
- **`NgramWordPredictor`'s two ranking modes differ on purpose:**
  - *Next-word mode* (prefix is empty — user just finished a word): shows
    **only** bigram continuations of the last completed word if any exist,
    falling back to raw unigram frequency only if that word was never seen
    in training data. Chosen because showing frequency-only suggestions
    here would ignore context entirely and defeat the point of "logical for
    the sentence."
  - *Word-completion mode* (prefix is non-empty): prefix is the primary
    filter (correctness first — don't suggest words that don't match what's
    being typed), and bigram-plausible candidates are boosted **ahead of**
    same-prefix candidates with higher raw frequency but no contextual
    support. Verified explicitly in tests: after training data where "the"
    is followed by "cat"/"dog"/"car", completing prefix "ca" ranks "cat"
    then "car" ahead of "cats"/"can" even though the latter have higher raw
    frequency in the synthetic corpus.
  - Prefix matching uses a sorted-by-word vector + binary search
    (`lower_bound` then linear scan while `starts_with(prefix)` holds)
    rather than a hand-rolled trie class — same practical effect, less code
    to get wrong, plenty fast for a bounded vocabulary. Ties (equal
    frequency) always break alphabetically, so ranking is fully
    deterministic and testable regardless of input order.
- **`Preferences` setters clamp out-of-range input rather than rejecting
  it** — a corrupted or malformed stored preferences file should never
  leave assistive-tech settings unusable. `SetActivePanelOrder` similarly
  de-duplicates rather than erroring, keeping each id's first occurrence.
- **`KeyAction` deliberately excludes theme/transparency/panel-arrangement
  actions.** `KeyAction` only ever represents "things injected into
  whichever app currently has OS focus" (`TypeCharacter`,
  `SendVirtualKey`, `ToggleModifier`). Settings-style UI (theme switch,
  transparency slider, add-panel button) is app-layer and writes directly
  to `Preferences` — it never flows through `Dispatcher`/key injection at
  all. Keeps the two concerns (typing vs. configuring the keyboard) from
  bleeding into one mechanism.
- **The keyboard's own window must never steal OS focus** from whatever
  the user is typing into. `IInputInjector::InjectKeyEvent` is specified to
  inject via `SendInput` targeting whatever currently has focus,
  independent of whether the keyboard's own window is focused — this is
  why the real Win32 implementation is expected to use
  `WS_EX_NOACTIVATE` on the keyboard window (noted as a risk to validate
  early once building `src/app`, since WinUI 3's internal pointer-routing
  behavior under `WS_EX_NOACTIVATE` hasn't been prototyped yet).

## What's built vs. deferred (snapshot as of 2026-08-07)

Built, tested (138 tests), verified compiling clean (zero warnings,
warnings-as-errors, Debug + RelWithDebInfo, via the Homebrew GCC 16
workaround above):
- All of `osk::core`: `KeyAction`/`Key`/`Panel`/`KeyboardLayout` +
  built-in panels, `ModifierState`/`Dispatcher`, `KeyRepeatController`,
  `DwellController`, `Preferences`, `IWordPredictor`/`NgramWordPredictor`.
- `osk::platform` interface *shapes* only (`IInputInjector`,
  `IAlwaysOnTopController`, `IWindowTransparency`, `IPreferencesStore`,
  `ISystemMetrics`) plus GoogleMock fakes under `tests/platform/mocks/`
  proving the seam works.

Deliberately not built yet (see `CLAUDE.md` Next Steps for the live list):
- Real Win32/WinRT bodies behind the five platform interfaces — needs
  Windows to author/verify.
- `src/app` (WinUI 3 shell) — needs the platform bodies above, plus
  Windows App SDK/MSIX project wiring only buildable on Windows/Visual
  Studio.
- Real word-frequency/bigram dictionary data (currently only tiny
  synthetic fixtures in tests) — sourcing and licensing not yet decided.
- Fullscreen-exclusive detection/warning UX — deferred per user's explicit
  choice, revisit once the app is running.
- `git init` — the user is doing this themselves, on purpose; nothing in
  this repo should assume git is initialized.
