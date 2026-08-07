#include "keyboard/core/KeyboardLayout.h"

#include <gtest/gtest.h>

namespace osk::core {
namespace {

Panel MakePanel(PanelId id) {
  Panel panel;
  panel.id = std::move(id);
  panel.displayName = panel.id;
  panel.rows.push_back(PanelRow{{Key{.id = panel.id + ".key", .label = "x", .action = ToggleCapsLock{}}}});
  return panel;
}

TEST(KeyboardLayoutTest, StartsWithOnlyTheBasePanelActive) {
  const KeyboardLayout layout(MakePanel("qwerty"));
  EXPECT_EQ(layout.BasePanel().id, "qwerty");
  EXPECT_TRUE(layout.ActivePanels().empty());
}

TEST(KeyboardLayoutTest, AddPanelActivatesIt) {
  KeyboardLayout layout(MakePanel("qwerty"));
  EXPECT_TRUE(layout.AddPanel(MakePanel("numeric")));
  EXPECT_TRUE(layout.HasPanel("numeric"));
  ASSERT_EQ(layout.ActivePanels().size(), 1U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "numeric");
}

TEST(KeyboardLayoutTest, AddPanelRejectsDuplicateId) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));

  EXPECT_FALSE(layout.AddPanel(MakePanel("numeric")));
  EXPECT_EQ(layout.ActivePanels().size(), 1U);
}

TEST(KeyboardLayoutTest, AddPanelPreservesInsertionOrder) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));
  layout.AddPanel(MakePanel("function"));
  layout.AddPanel(MakePanel("arrows"));

  ASSERT_EQ(layout.ActivePanels().size(), 3U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "numeric");
  EXPECT_EQ(layout.ActivePanels()[1].id, "function");
  EXPECT_EQ(layout.ActivePanels()[2].id, "arrows");
}

TEST(KeyboardLayoutTest, RemovePanelDeactivatesIt) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));

  EXPECT_TRUE(layout.RemovePanel("numeric"));
  EXPECT_FALSE(layout.HasPanel("numeric"));
  EXPECT_TRUE(layout.ActivePanels().empty());
}

TEST(KeyboardLayoutTest, RemovePanelOnInactiveIdIsNoop) {
  KeyboardLayout layout(MakePanel("qwerty"));
  EXPECT_FALSE(layout.RemovePanel("numeric"));
}

TEST(KeyboardLayoutTest, ReorderPanelMovesItToTheFront) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));
  layout.AddPanel(MakePanel("function"));
  layout.AddPanel(MakePanel("arrows"));

  EXPECT_TRUE(layout.ReorderPanel("arrows", 0));

  ASSERT_EQ(layout.ActivePanels().size(), 3U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "arrows");
  EXPECT_EQ(layout.ActivePanels()[1].id, "numeric");
  EXPECT_EQ(layout.ActivePanels()[2].id, "function");
}

TEST(KeyboardLayoutTest, ReorderPanelClampsOutOfRangeIndexToEnd) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));
  layout.AddPanel(MakePanel("function"));

  EXPECT_TRUE(layout.ReorderPanel("numeric", 100));

  ASSERT_EQ(layout.ActivePanels().size(), 2U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "function");
  EXPECT_EQ(layout.ActivePanels()[1].id, "numeric");
}

TEST(KeyboardLayoutTest, ReorderPanelOnInactiveIdIsNoop) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));

  EXPECT_FALSE(layout.ReorderPanel("arrows", 0));
  ASSERT_EQ(layout.ActivePanels().size(), 1U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "numeric");
}

TEST(KeyboardLayoutTest, ReorderPanelWithSingleActivePanelIsIdentity) {
  KeyboardLayout layout(MakePanel("qwerty"));
  layout.AddPanel(MakePanel("numeric"));

  EXPECT_TRUE(layout.ReorderPanel("numeric", 0));
  ASSERT_EQ(layout.ActivePanels().size(), 1U);
  EXPECT_EQ(layout.ActivePanels()[0].id, "numeric");
}

}  // namespace
}  // namespace osk::core
