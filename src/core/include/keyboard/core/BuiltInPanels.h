#pragma once

#include "keyboard/core/Panel.h"

namespace osk::core::panels {

// The mandatory base panel: full QWERTY letter/digit/symbol layout, plus an
// always-available utility row (Shift+Enter shortcut, Copy, Paste).
Panel Qwerty();

// Optional panels, addable independently and in any order via
// KeyboardLayout.
Panel Numeric();
Panel FunctionKeys();
Panel ArrowKeys();

}  // namespace osk::core::panels
