#include "keyboard/core/BuiltInPanels.h"

#include <utility>

namespace osk::core::panels {

namespace {

Key Letter(std::string id, char lower) {
  const auto upper = static_cast<char32_t>('A' + (lower - 'a'));
  return Key{
      .id = std::move(id),
      .label = std::string(1, lower),
      .action = TypeCharacter{.base = static_cast<char32_t>(lower), .shifted = upper, .capsLockApplies = true},
  };
}

Key Symbol(std::string id, std::string label, char32_t base, char32_t shifted) {
  return Key{
      .id = std::move(id),
      .label = std::move(label),
      .action = TypeCharacter{.base = base, .shifted = shifted, .capsLockApplies = false},
  };
}

Key Special(std::string id, std::string label, VirtualKey key, Modifier modifiers = Modifier::kNone) {
  return Key{
      .id = std::move(id),
      .label = std::move(label),
      .action = SendVirtualKey{.key = key, .modifiers = modifiers},
  };
}

Key ModifierToggle(std::string id, std::string label, ModifierKind kind) {
  return Key{.id = std::move(id), .label = std::move(label), .action = ToggleModifier{.modifier = kind}};
}

Key CapsLockKey() {
  return Key{.id = "qwerty.capslock", .label = "Caps Lock", .action = ToggleCapsLock{}};
}

}  // namespace

Panel Qwerty() {
  Panel panel;
  panel.id = "qwerty";
  panel.displayName = "QWERTY";

  panel.rows.push_back(PanelRow{{
      Symbol("qwerty.grave", "`", U'`', U'~'),
      Symbol("qwerty.1", "1", U'1', U'!'),
      Symbol("qwerty.2", "2", U'2', U'@'),
      Symbol("qwerty.3", "3", U'3', U'#'),
      Symbol("qwerty.4", "4", U'4', U'$'),
      Symbol("qwerty.5", "5", U'5', U'%'),
      Symbol("qwerty.6", "6", U'6', U'^'),
      Symbol("qwerty.7", "7", U'7', U'&'),
      Symbol("qwerty.8", "8", U'8', U'*'),
      Symbol("qwerty.9", "9", U'9', U'('),
      Symbol("qwerty.0", "0", U'0', U')'),
      Symbol("qwerty.minus", "-", U'-', U'_'),
      Symbol("qwerty.equals", "=", U'=', U'+'),
      Special("qwerty.backspace", "Backspace", VirtualKey::kBackspace),
  }});

  panel.rows.push_back(PanelRow{{
      Special("qwerty.tab", "Tab", VirtualKey::kTab),
      Letter("qwerty.q", 'q'),
      Letter("qwerty.w", 'w'),
      Letter("qwerty.e", 'e'),
      Letter("qwerty.r", 'r'),
      Letter("qwerty.t", 't'),
      Letter("qwerty.y", 'y'),
      Letter("qwerty.u", 'u'),
      Letter("qwerty.i", 'i'),
      Letter("qwerty.o", 'o'),
      Letter("qwerty.p", 'p'),
      Symbol("qwerty.lbracket", "[", U'[', U'{'),
      Symbol("qwerty.rbracket", "]", U']', U'}'),
      Symbol("qwerty.backslash", "\\", U'\\', U'|'),
  }});

  panel.rows.push_back(PanelRow{{
      CapsLockKey(),
      Letter("qwerty.a", 'a'),
      Letter("qwerty.s", 's'),
      Letter("qwerty.d", 'd'),
      Letter("qwerty.f", 'f'),
      Letter("qwerty.g", 'g'),
      Letter("qwerty.h", 'h'),
      Letter("qwerty.j", 'j'),
      Letter("qwerty.k", 'k'),
      Letter("qwerty.l", 'l'),
      Symbol("qwerty.semicolon", ";", U';', U':'),
      Symbol("qwerty.quote", "'", U'\'', U'"'),
      Special("qwerty.enter", "Enter", VirtualKey::kEnter),
  }});

  panel.rows.push_back(PanelRow{{
      ModifierToggle("qwerty.shift_left", "Shift", ModifierKind::kShift),
      Letter("qwerty.z", 'z'),
      Letter("qwerty.x", 'x'),
      Letter("qwerty.c", 'c'),
      Letter("qwerty.v", 'v'),
      Letter("qwerty.b", 'b'),
      Letter("qwerty.n", 'n'),
      Letter("qwerty.m", 'm'),
      Symbol("qwerty.comma", ",", U',', U'<'),
      Symbol("qwerty.period", ".", U'.', U'>'),
      Symbol("qwerty.slash", "/", U'/', U'?'),
      ModifierToggle("qwerty.shift_right", "Shift", ModifierKind::kShift),
  }});

  panel.rows.push_back(PanelRow{{
      ModifierToggle("qwerty.ctrl", "Ctrl", ModifierKind::kControl),
      ModifierToggle("qwerty.alt", "Alt", ModifierKind::kAlt),
      Special("qwerty.space", "Space", VirtualKey::kSpace),
      Special("qwerty.shift_enter", "Shift+Enter", VirtualKey::kEnter, Modifier::kShift),
      Special("qwerty.copy", "Copy", VirtualKey::kC, Modifier::kControl),
      Special("qwerty.paste", "Paste", VirtualKey::kV, Modifier::kControl),
  }});

  return panel;
}

Panel Numeric() {
  Panel panel;
  panel.id = "numeric";
  panel.displayName = "Numeric";

  auto digit = [](std::string id, char32_t ch) {
    return Symbol(std::move(id), std::string(1, static_cast<char>(ch)), ch, ch);
  };

  panel.rows.push_back(PanelRow{{
      digit("numeric.7", U'7'),
      digit("numeric.8", U'8'),
      digit("numeric.9", U'9'),
      Symbol("numeric.divide", "/", U'/', U'/'),
  }});
  panel.rows.push_back(PanelRow{{
      digit("numeric.4", U'4'),
      digit("numeric.5", U'5'),
      digit("numeric.6", U'6'),
      Symbol("numeric.multiply", "*", U'*', U'*'),
  }});
  panel.rows.push_back(PanelRow{{
      digit("numeric.1", U'1'),
      digit("numeric.2", U'2'),
      digit("numeric.3", U'3'),
      Symbol("numeric.subtract", "-", U'-', U'-'),
  }});
  panel.rows.push_back(PanelRow{{
      digit("numeric.0", U'0'),
      Symbol("numeric.decimal", ".", U'.', U'.'),
      Special("numeric.enter", "Enter", VirtualKey::kEnter),
      Symbol("numeric.add", "+", U'+', U'+'),
  }});

  return panel;
}

Panel FunctionKeys() {
  Panel panel;
  panel.id = "function";
  panel.displayName = "Function Keys";

  panel.rows.push_back(PanelRow{{
      Special("function.f1", "F1", VirtualKey::kF1),
      Special("function.f2", "F2", VirtualKey::kF2),
      Special("function.f3", "F3", VirtualKey::kF3),
      Special("function.f4", "F4", VirtualKey::kF4),
      Special("function.f5", "F5", VirtualKey::kF5),
      Special("function.f6", "F6", VirtualKey::kF6),
  }});
  panel.rows.push_back(PanelRow{{
      Special("function.f7", "F7", VirtualKey::kF7),
      Special("function.f8", "F8", VirtualKey::kF8),
      Special("function.f9", "F9", VirtualKey::kF9),
      Special("function.f10", "F10", VirtualKey::kF10),
      Special("function.f11", "F11", VirtualKey::kF11),
      Special("function.f12", "F12", VirtualKey::kF12),
  }});

  return panel;
}

Panel ArrowKeys() {
  Panel panel;
  panel.id = "arrows";
  panel.displayName = "Arrow Keys";

  panel.rows.push_back(PanelRow{{
      Special("arrows.up", "↑", VirtualKey::kUpArrow),
  }});
  panel.rows.push_back(PanelRow{{
      Special("arrows.left", "←", VirtualKey::kLeftArrow),
      Special("arrows.down", "↓", VirtualKey::kDownArrow),
      Special("arrows.right", "→", VirtualKey::kRightArrow),
  }});

  return panel;
}

}  // namespace osk::core::panels
