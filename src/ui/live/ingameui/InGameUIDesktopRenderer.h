// license:GPLv3+

#pragma once

#include "InGameUIRenderer.h"
#include "InGameUIPane.h"
#include "imgui/imgui.h"

class Player;
class Sampler;

namespace VPX::InGameUI
{

// Fullscreen InGameUI renderer for desktop play (mouse & keyboard):
// a flat overlay on the table render — a darker top bar with letter-spaced
// uppercase categories (Table / Settings / Plugins / About), a left rail of the
// category's sub-pages (icon + large label, accent side bar on the selected one),
// and the active page's content as two-line rows (label + info subtext, control
// aligned right, thin separators) over a bottom input hint bar. The root page's
// items (the homepage table of contents) feed the navigation categories.
// On portrait frames the layout adapts: portrait FSS keeps the fullscreen layout
// with each item's control stacked under its label, and portrait cabinet renders
// the whole UI as a 16:11 inset "display" in the lower half of the frame. On
// narrow frames the tab bar splits in two lines and the left rail folds to an
// icon column, deploying over the item pane when hovered or navigated to. On
// very wide frames the chrome is capped to a centered column, the frame showing
// through dimmed side bands.
class InGameUIDesktopRenderer final : public InGameUIRenderer
{
public:
   explicit InGameUIDesktopRenderer(InGameUI& owner);

   void RenderPages(const vector<InGameUIPage*>& pages, float elapsedS) override;

   bool OnUINavigatePrev(InGameUIPage* page) override;
   bool OnUINavigateNext(InGameUIPage* page) override;
   bool OnUIAdjust(InGameUIPage* page, float direction, bool isInitialPress) override;
   bool OnUIReleaseAdjust(InGameUIPage* page) override;
   bool OnUINavigateBack(InGameUIPage* page) override;

private:
   bool NavStep(InGameUIPage* page, int direction);
   void EngageNav(InGameUIPage* page, int direction);
   bool IsNavSource(const InGameUIPage& page) const;
   bool IsItemNavFocusable(const InGameUIPage& page, int index) const;
   bool HasNavFocusableItem(const InGameUIPage& page) const;
   void EnsureNavSelection(InGameUIPage& page, int direction);
   void RenderContentPage(InGameUIPage& page, const vector<InGameUIPane>& panes, bool isNavSource, const string& pagePath);
   void RenderFooterActions(InGameUIPage& page);
   void RenderItems(InGameUIPage& page, const vector<InGameUIPane>& panes, bool isNavSource);
   bool NavRow(const char* icon, const char* label, bool selected, const char* id, const std::shared_ptr<Sampler>& iconImage, bool focused);

   InGameUI& m_inGameUI;
   Player* const m_player;

   // Navigation item of the root page, kept while it is closed under another page
   struct NavEntry
   {
      string label;
      string path;
   };
   vector<NavEntry> m_navEntries; // Cached root page navigation items
   vector<NavEntry> m_pluginEntries; // Cached plugins homepage items (one per plugin)
   string m_selectedGroup; // First path segment of the selected category (the root page belongs to "table")
   std::shared_ptr<Sampler> m_plungerIcon; // Lazy-loaded editor plunger icon for the rail
   bool m_plungerLoadFailed = false;

   // Flipper navigation: the focus has three levels — the top tab bar (cycled
   // left to right), the left rail (cycled top to bottom), and the page items
   // with their bottom action buttons (cycled top to bottom). UI up/down moves
   // the focus within a level, UI left/right selects or adjusts the focused
   // element, and back moves up a level (the last level goes back in history).
   enum class NavLevel
   {
      TopTabs,
      SideRail,
      PageItems
   };
   NavLevel m_navLevel = NavLevel::PageItems;
   int m_navTabIndex = -1; // Focused top tab (-1: sync to the selected group at next render)
   int m_navRailIndex = -1; // Focused rail row (-1: sync to the selected entry at next render)
   bool m_navWasActive = false; // Flipper-nav engagement as of the last rendered frame
   bool m_modalActive = false; // A modal pane replaced the whole UI on the last rendered frame (the page owns its input)
   bool m_stackedItemRows = false; // Portrait frame: item controls are stacked under their label instead of right-aligned
   bool m_railDeployed = false; // Narrow frame: the icon-folded rail is deployed over the item pane (hovered or focused navigation level)
   float m_railDeployAnim = 0.f; // Fold-to-deployed width animation (0: folded icon column, 1: fully deployed over the item pane)

   // Drag-to-scroll inside scrollable panes: press-and-hold (VR controller trigger or mouse button)
   // inside a scrollpane then move up/down drags its content, with a short inertial glide on release
   void UpdateDragScroll();
   bool m_isDraggingScroll = false;
   ImGuiID m_dragScrollPaneId = 0; // Pane that owns the current drag gesture
   ImVec2 m_dragScrollStartPos { 0.f, 0.f };
   float m_dragScrollVelocity = 0.f; // px/s, for the inertial glide after release
   static bool IsWidgetActive();
   ImGuiID m_dragPassThroughId = 0; // Last pressed whole-row button: a drag-scroll may start on it (the press is then cancelled)
   string m_lastNavPath; // Navigation path as of the last rendered frame (level resets on change)
   struct TabNav
   {
      string label;
      std::function<void()> activate;
      bool descendToItems; // The destination group's rail is empty (a single page): the activation descends straight to the page items
   };
   vector<TabNav> m_navTabs; // Top tabs in left-to-right order, rebuilt each frame
   struct RailNav
   {
      bool isBack;
      string label;
      string path;
   };
   vector<RailNav> m_navRailRows; // Rail rows top to bottom (back row first), rebuilt each frame
};

}
