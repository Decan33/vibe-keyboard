#pragma once

#include <string>
#include <vector>

#include "keyboard/core/Key.h"

namespace osk::core {

using PanelId = std::string;

struct PanelRow {
  std::vector<Key> keys;

  bool operator==(const PanelRow&) const = default;
};

// A self-contained, independently addable group of keys (e.g. the QWERTY
// letters, or a numeric pad). KeyboardLayout composes a mandatory base
// panel with zero or more optional panels.
struct Panel {
  PanelId id;
  std::string displayName;
  std::vector<PanelRow> rows;

  bool operator==(const Panel&) const = default;
};

}  // namespace osk::core
