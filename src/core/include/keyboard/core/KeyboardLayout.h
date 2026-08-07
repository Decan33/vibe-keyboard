#pragma once

#include <vector>

#include "keyboard/core/Panel.h"

namespace osk::core {

// Composes a mandatory base panel (QWERTY) with an ordered set of optional
// panels the user has chosen to show (numeric, function keys, arrow keys).
// Optional panels can be added, removed, and reordered independently of
// each other, in any order.
class KeyboardLayout {
 public:
  explicit KeyboardLayout(Panel basePanel);

  const Panel& BasePanel() const;
  const std::vector<Panel>& ActivePanels() const;
  bool HasPanel(const PanelId& panelId) const;

  // Appends panel to the end of the active list. Returns false (no-op) if
  // a panel with the same id is already active.
  bool AddPanel(Panel panel);

  // Returns false (no-op) if panelId isn't active.
  bool RemovePanel(const PanelId& panelId);

  // Moves an already-active panel so it occupies newIndex; newIndex is
  // clamped to the valid range. Returns false (no-op) if panelId isn't
  // active.
  bool ReorderPanel(const PanelId& panelId, std::size_t newIndex);

 private:
  Panel basePanel_;
  std::vector<Panel> activePanels_;
};

}  // namespace osk::core
