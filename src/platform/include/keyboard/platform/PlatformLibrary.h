#pragma once

// Thin, mockable wrappers around Windows-specific APIs (input injection,
// always-on-top, window transparency, preferences persistence, system
// keyboard metrics) live under this namespace, behind interfaces so
// osk::core never depends on Win32 directly.
namespace osk::platform {}
