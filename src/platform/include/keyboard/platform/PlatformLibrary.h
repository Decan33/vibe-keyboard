#pragma once

// Thin, mockable wrappers around Windows-specific APIs (input injection,
// hotkey registration, window placement, UI Automation) live under this
// namespace, behind interfaces so osk::core never depends on Win32 directly.
// Populated once the functional requirements land.
namespace osk::platform {}
