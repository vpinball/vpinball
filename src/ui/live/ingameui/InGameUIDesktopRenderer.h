// license:GPLv3+

#pragma once

#include "InGameUIRenderer.h"
#include "InGameUIPane.h"
#include "imgui/imgui.h"

class Player;
class Sampler;

namespace VPX::InGameUI
{

// Fullscreen InGameUI renderer for desktop play (mouse & keyboard, landscape):
// a flat overlay on the table render — a darker top bar with letter-spaced
// uppercase categories (Table / Settings / Plugins / About), a left rail of the
// category's sub-pages (icon + large label, accent side bar on the selected one),
// and the active page's content as two-line rows (label + info subtext, control
// aligned right, thin separators) over a bottom input hint bar. The root page's
// items (the homepage table of contents) feed the navigation categories.
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
   string m_lastNavPath; // Navigation path as of the last rendered frame (level resets on change)
   struct TabNav
   {
      string label;
      std::function<void()> activate;
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
