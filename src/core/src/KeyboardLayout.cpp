#include "keyboard/core/KeyboardLayout.h"

#include <algorithm>
#include <utility>

namespace osk::core {

KeyboardLayout::KeyboardLayout(Panel basePanel) : basePanel_(std::move(basePanel)) {}

const Panel& KeyboardLayout::BasePanel() const {
  return basePanel_;
}

const std::vector<Panel>& KeyboardLayout::ActivePanels() const {
  return activePanels_;
}

bool KeyboardLayout::HasPanel(const PanelId& panelId) const {
  return std::any_of(activePanels_.begin(), activePanels_.end(),
                      [&panelId](const Panel& panel) { return panel.id == panelId; });
}

bool KeyboardLayout::AddPanel(Panel panel) {
  if (HasPanel(panel.id)) {
    return false;
  }
  activePanels_.push_back(std::move(panel));
  return true;
}

bool KeyboardLayout::RemovePanel(const PanelId& panelId) {
  const auto it = std::find_if(activePanels_.begin(), activePanels_.end(),
                                [&panelId](const Panel& panel) { return panel.id == panelId; });
  if (it == activePanels_.end()) {
    return false;
  }
  activePanels_.erase(it);
  return true;
}

bool KeyboardLayout::ReorderPanel(const PanelId& panelId, std::size_t newIndex) {
  const auto it = std::find_if(activePanels_.begin(), activePanels_.end(),
                                [&panelId](const Panel& panel) { return panel.id == panelId; });
  if (it == activePanels_.end()) {
    return false;
  }

  Panel panel = std::move(*it);
  activePanels_.erase(it);

  const std::size_t clampedIndex = std::min(newIndex, activePanels_.size());
  activePanels_.insert(activePanels_.begin() + static_cast<std::ptrdiff_t>(clampedIndex), std::move(panel));
  return true;
}

}  // namespace osk::core
