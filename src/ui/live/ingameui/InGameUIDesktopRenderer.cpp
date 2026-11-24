// license:GPLv3+

#include "core/stdafx.h"
#include "InGameUIDesktopRenderer.h"

#include "InGameUI.h"
#include "InGameUIPage.h"
#include "InGameUIWidgets.h"

#include "core/player.h"
#include "core/VPApp.h"
#include "parts/pintable.h"
#include "plugins/MsgPluginManager.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "renderer/ViewSetup.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::InGameUI
{

namespace
{
   constexpr ImU32 kAccentColor = IM_COL32(232, 84, 134, 255); // Selected tab underline / sub-category side bar
   constexpr ImU32 kFocusColor = IM_COL32(110, 180, 255, 255); // Flipper-nav focus ring on the focused element

   // Letter-spaced text (like the LOADING caption), measured then drawn char by char
   float SpacedTextWidth(ImFont* const font, const float fontSize, const char* const text, const float tracking)
   {
      float width = -tracking;
      for (const char* c = text; *c; c++)
         width += font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, c, c + 1).x + tracking;
      return width;
   }

   void DrawSpacedText(ImDrawList* const drawList, ImFont* const font, const float fontSize, const ImVec2& pos, const float tracking, const ImU32 col, const char* const text)
   {
      ImVec2 pen = pos;
      for (const char* c = text; *c; c++)
      {
         drawList->AddText(font, fontSize, pen, col, c, c + 1);
         pen.x += font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, c, c + 1).x + tracking;
      }
   }

   const char* PathIcon(const string& path)
   {
      if (path == "settings/audio")
         return ICON_LC_VOLUME_2;
      if (path == "settings/cabinet")
         return ICON_LC_BOX;
      if (path == "settings/display_profiles")
         return ICON_LC_SLIDERS_HORIZONTAL;
      if (path == "settings/displays")
         return ICON_LC_MONITOR;
      if (path == "settings/graphics")
         return ICON_LC_IMAGE;
      if (path == "settings/input")
         return ICON_LC_GAMEPAD_2;
      if (path == "settings/logging")
         return ICON_LC_FILE;
      if (path == "settings/misc")
         return ICON_LC_LIST_FILTER;
      if (path == "settings/nudge")
         return ICON_LC_MOVE_3D;
      if (path == "settings/plunger")
         return ICON_LC_MAGNET; // Fallback while the plunger image icon loads
      if (path == "settings/pov")
         return ICON_LC_EYE;
      if (path == "settings/stereo")
         return ICON_LC_RECTANGLE_GOGGLES;
      if (path == "settings/vr")
         return ICON_LC_ROTATE_3D;
      if (path == "table/rules")
         return ICON_LC_CROWN;
      if (path == "table/options")
         return ICON_LC_SETTINGS;
      if (path == "plugins/homepage")
         return ICON_LC_BOX;
      if (path == "misc/systeminfo")
         return ICON_LC_ATOM;
      return ICON_LC_LIST;
   }
}

InGameUIDesktopRenderer::InGameUIDesktopRenderer(InGameUI& owner)
   : m_inGameUI(owner)
   , m_player(g_pplayer)
{
}

void InGameUIDesktopRenderer::RenderPages(const vector<InGameUIPage*>& pages, float elapsedS)
{
   if (pages.empty())
      return;
   const ImGuiIO& io = ImGui::GetIO();
   const ImGuiStyle& style = ImGui::GetStyle();
   const float dpi = m_player->m_liveUI->GetDPI();

   // The desktop chrome fills the whole frame in landscape and portrait FSS; in
   // portrait cabinet mode it is inset as a square "display" fitted to the frame
   // width, centered in the lower half while keeping a margin off the bottom edge
   const bool portraitFrame = io.DisplaySize.x < io.DisplaySize.y;
   const bool cabinetInset = portraitFrame && m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN;
   ImVec2 uiPos(0.f, 0.f), uiSize(io.DisplaySize);
   if (cabinetInset)
   {
      uiSize.y = uiSize.x;
      const float margin = 24.f * dpi;
      uiPos.y = std::min(io.DisplaySize.y * 0.5f + std::max(0.f, io.DisplaySize.y * 0.5f - uiSize.y - margin) * 0.5f, io.DisplaySize.y - margin - uiSize.y);
   }
   // Portrait frames stack each item's control under its label to keep rows usable
   m_stackedItemRows = portraitFrame && !cabinetInset;

   // Each page declares its panes for the frame; a modal pane replaces its page's
   // rendering entirely but stays under the pages stacked above it
   vector<vector<InGameUIPane>> allPanes(pages.size());
   bool topHasModal = false;
   for (size_t p = 0; p < pages.size(); ++p)
   {
      pages[p]->CollectPanes(allPanes[p]);
      for (const InGameUIPane& pane : allPanes[p])
         if (pane.placement == InGameUIPane::Placement::Modal && pane.render)
         {
            pane.render(ImVec4(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y));
            topHasModal = (p == pages.size() - 1);
         }
   }

   // The root page (the homepage table of contents) is the navigation source: its
   // navigation items are cached while it is closed under another page so the
   // section columns stay available on every page
   InGameUIPage* const rootPage = m_inGameUI.GetRootPage();
   if (rootPage != nullptr && std::find(pages.begin(), pages.end(), rootPage) != pages.end())
   {
      m_navEntries.clear();
      for (const auto& item : rootPage->GetItems())
         if (item->m_type == InGameUIItem::Type::Navigation)
            m_navEntries.push_back({ item->m_label, item->m_path });
   }

   // The plugins homepage's navigation items (one per plugin) are cached the same
   // way while it is closed under a plugin page, so the rail keeps listing them
   for (InGameUIPage* page : pages)
   {
      const auto& items = page->GetItems();
      bool isPluginHome = false;
      for (const auto& item : items)
         if (item->m_type == InGameUIItem::Type::Navigation && item->m_path.starts_with("plugin/"s))
            isPluginHome = true;
      if (!isPluginHome)
         continue;
      m_pluginEntries.clear();
      for (const auto& item : items)
         if (item->m_type == InGameUIItem::Type::Navigation && item->m_path.starts_with("plugin/"s))
            m_pluginEntries.push_back({ item->m_label, item->m_path });
      break;
   }

   const auto& history = m_inGameUI.GetNavigationHistory();
   const string activePath = history.empty() ? ""s : history.back();
   const string rootPath = history.empty() ? ""s : history.front();
   InGameUIPage* const contentPage = pages.back();

   // Flipper nav state: the focus level restarts on the page items when the
   // location changes or when navigation was disengaged (mouse took over)
   if (activePath != m_lastNavPath)
   {
      m_lastNavPath = activePath;
      m_navLevel = NavLevel::PageItems;
   }
   if (!m_inGameUI.IsFlipperNav())
      m_navLevel = NavLevel::PageItems;
   m_navWasActive = m_inGameUI.IsFlipperNav();

   const auto groupOf = [](const string& path) { return path.substr(0, path.find('/')); };

   // Selected category: follows the active page's path. The root page itself
   // belongs to the "Table" category as its "Table Info" sub-entry
   const string selectedNavPath = activePath;
   if (contentPage == rootPage)
      m_selectedGroup = "table"s;
   else if (!activePath.starts_with("popup/"s))
      m_selectedGroup = groupOf(activePath);
   // Individual plugin pages ("plugin/<id>") and plugin sheets
   // ("sheet/<pluginId>/...") live under the "plugins" category
   if (m_selectedGroup == "plugin"s || m_selectedGroup == "sheet"s)
      m_selectedGroup = "plugins"s;

   // Navigation entries per top-level category (first path segment)
   struct Group
   {
      string id;
      vector<const NavEntry*> entries;
   };
   vector<Group> groups;
   for (const NavEntry& entry : m_navEntries)
   {
      const string group = groupOf(entry.path);
      auto it = std::ranges::find_if(groups, [&group](const Group& g) { return g.id == group; });
      if (it == groups.end())
         it = groups.insert(groups.end(), { group, {} });
      it->entries.push_back(&entry);
   }
   const auto firstPathOf = [&groups](const string& id)
   {
      for (const Group& g : groups)
         if (g.id == id && !g.entries.empty())
            return g.entries.front()->path;
      return ""s;
   };
   const Group* selectedGroup = nullptr;
   for (const Group& g : groups)
      if (g.id == m_selectedGroup)
         selectedGroup = &g;

   // The rail relies on the cached navigation entries so it stays available while
   // the homepage is closed under a leaf page
   const bool showNav = !m_navEntries.empty();

   // Tabs in left-to-right order (right-aligned ones are laid out from the right
   // edge, rightmost first); the same order drives flipper navigation. They are
   // built ahead of the layout since their width decides whether the bar needs
   // a second line. The activate lambdas are kept in m_navTabs for flipper
   // navigation: they capture the target paths by value since they outlive this frame
   struct Tab
   {
      const char* label;
      const char* group;
      bool right; // Right-justified inside its line
      float scale; // Scale on a single-line bar (the folded second line uses the secondary 0.85 style)
      int line; // Line on the folded two-line bar
      std::function<void()> activate;
   };
   ImFont* const tabFont = m_player->m_liveUI->GetOverlayFont() ? m_player->m_liveUI->GetOverlayFont() : ImGui::GetFont();
   const float tabSize = ImGui::GetFontSize() * 1.6f;
   const float tracking = tabSize * 0.30f;
   const float tabGap = 56.f * dpi;
   vector<Tab> tabs;
   if (showNav)
   {
      tabs.push_back({ "GAME", "game", false, 1.f, 1, [this]() { m_player->m_liveUI->HideUI(); } });
      tabs.push_back({ "TABLE", "table", false, 1.f, 0, [this, rootPath]()
         {
            m_selectedGroup = "table";
            m_inGameUI.NavigateTo(rootPath);
         } });
      const string settingsPath = firstPathOf("settings");
      if (!settingsPath.empty())
         tabs.push_back({ "SETTINGS", "settings", false, 1.f, 0, [this, settingsPath]()
            {
               m_selectedGroup = "settings";
               m_inGameUI.NavigateTo(settingsPath);
            } });
      const string pluginsPath = firstPathOf("plugins");
      if (!pluginsPath.empty())
         tabs.push_back({ "PLUGINS", "plugins", false, 1.f, 0, [this, pluginsPath]()
            {
               m_selectedGroup = "plugins";
               m_inGameUI.NavigateTo(pluginsPath);
            } });
      const string aboutPath = firstPathOf("misc");
      if (!aboutPath.empty())
         tabs.push_back({ "ABOUT", "misc", true, 0.85f, 1, [this, aboutPath]()
            {
               m_selectedGroup = "misc";
               m_inGameUI.NavigateTo(aboutPath);
            } });
      // Quit discards the UI and requests the session close, using the same close
      // state selection as the Exit Game input action
      tabs.push_back({ "QUIT", "quit", true, 0.85f, 1, [this]()
         {
            m_player->m_liveUI->HideUI();
            m_player->SetCloseState(m_player->HasStackedTableSession() ? Player::CS_STOP_PLAY
                  : g_isMobile                                         ? Player::CS_CLOSE_CAPTURE_SCREENSHOT
                  : g_isStandalone                                     ? Player::CS_CLOSE_APP
                                                                       : Player::CS_STOP_PLAY);
         } });
   }
   // Portrait frames always fold the bar to two lines; on landscape it folds
   // when the tab labels don't fit. First line: the left-justified categories;
   // second line: Game (left) and About/Quit (right), in the secondary style
   bool twoLineTabs = portraitFrame;
   if (showNav && !twoLineTabs)
   {
      float leftW = 28.f * dpi, rightW = 28.f * dpi;
      for (const Tab& tab : tabs)
         (tab.right ? rightW : leftW) += SpacedTextWidth(tabFont, tabSize * tab.scale, tab.label, tracking) + tabGap;
      twoLineTabs = leftW + rightW - tabGap > uiSize.x;
   }
   const float tabLineH = ImGui::GetFrameHeight() + 16.f * dpi; // One line of the two-line top bar
   const float topBarH = showNav ? (twoLineTabs ? 2.f * tabLineH : ImGui::GetFrameHeight() + 40.f * dpi) : 0.f;
   const float hintH = showNav ? ImGui::GetFrameHeight() + 14.f * dpi : 0.f;
   const float columnH = uiSize.y - topBarH - hintH;
   // The rail folds to its icon column (slot width + paddings) when the frame is
   // too narrow to afford the full labels, deploying over the item pane on hover
   // or when flipper navigation reaches its level; the item pane keeps the folded
   // width either way
   const float subW = showNav ? std::clamp(uiSize.x * 0.20f, 220.f * dpi, 340.f * dpi) : 0.f;
   const float foldW = ImGui::GetFontSize() * 1.45f + 8.f * dpi + 2.f * style.WindowPadding.x;
   const bool foldRail = showNav && uiSize.x < 900.f * dpi;
   const float railW = foldRail ? foldW : subW;

   // A modal pane of the top page replaces the whole UI (loading screen, stereo
   // calibration): the desktop chrome is not drawn at all and the page keeps
   // handling its own navigation input
   m_modalActive = topHasModal;
   if (topHasModal)
   {
      for (InGameUIPage* page : pages)
         RenderInputActionPopup(m_player, *page);
      return;
   }

   // The flipper-nav item focus always rests on an item this renderer draws (the
   // page model's selection may sit on a non-rendered item like the Back action)
   if (m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::PageItems)
      EnsureNavSelection(*contentPage, 1);

   const auto& contentPanes = allPanes.back();
   const ImVec2 contentPos(uiPos.x + railW, uiPos.y + topBarH);
   const ImVec2 contentSize(uiSize.x - railW, columnH);

   // Backdrop panes (homepage table information) are drawn before the UI window so
   // they stay behind it: the page content renders on top while the pane shows
   // through the darkening
   for (const InGameUIPane& pane : contentPanes)
      if (pane.placement == InGameUIPane::Placement::Backdrop && pane.render)
         pane.render(ImVec4(contentPos.x, contentPos.y, contentSize.x, contentSize.y));

   // Attached panes (sensor plots, cabinet view, display preview...) stack in a
   // band at the bottom of the content region which shrinks accordingly; in
   // cabinet mode they instead stack in the free band above the UI inset,
   // bottom-aligned just over it, leaving the content untouched
   const float paneBandW = cabinetInset ? uiSize.x : contentSize.x;
   const float paneBandH = cabinetInset ? std::max(0.f, uiPos.y - 12.f * dpi) : contentSize.y * 0.5f;
   const float paneBandCap = cabinetInset ? paneBandH : contentSize.y * 0.55f;
   vector<std::pair<const InGameUIPane*, ImVec2>> attachedPanes;
   float panesH = 0.f;
   for (const InGameUIPane& pane : contentPanes)
   {
      if (pane.placement != InGameUIPane::Placement::Attached || !pane.render)
         continue;
      const ImVec2 paneSize = pane.measure ? pane.measure(paneBandW, paneBandH) : ImVec2(paneBandW, paneBandH);
      if (panesH + paneSize.y + style.ItemSpacing.y > paneBandCap)
         break; // Keep enough room for the settings list above the pane band
      attachedPanes.push_back({ &pane, paneSize });
      panesH += paneSize.y + style.ItemSpacing.y;
   }
   // In cabinet mode the panes live above the inset: the content keeps the full column
   const float contentPanesH = cabinetInset ? 0.f : panesH;

   // The window has no background of its own: the faint darkening is drawn
   // manually, leaving the attached pane band clear since previews rendered to
   // the framebuffer (display profile, DMD) sit below all ImGui windows
   ImGui::SetNextWindowPos(uiPos);
   ImGui::SetNextWindowSize(uiSize);
   ImGui::SetNextWindowBgAlpha(0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::Begin("##InGameUIDesktop", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
   ImGui::PopStyleVar(); // The window captured a zero padding at Begin; columns keep the standard padding
   // The chrome fades only when the whole UI closes back to the game (Close
   // clears the navigation history while the last page animates out); during
   // page-to-page transitions it stays opaque while the content fades.
   // GetColorU32-fetched colors are dimmed by the style alpha; explicit colors
   // must be passed through it
   const float pageFade = 1.f - fabs(contentPage->GetOpenCloseAnimPos());
   ImGui::PushStyleVar(ImGuiStyleVar_Alpha, history.empty() ? pageFade : 1.f);
   ImDrawList* const mainDrawList = ImGui::GetWindowDrawList();
   const ImVec2 winPos = ImGui::GetWindowPos();
   if (cabinetInset)
      mainDrawList->AddRect(winPos, winPos + uiSize, ImGui::GetColorU32(IM_COL32(255, 255, 255, 50)), 0.f, 0, 1.f * dpi);
   {
      const ImU32 dimCol = ImGui::GetColorU32(ImGuiCol_WindowBg, m_player->IsVR() ? 1.f : 0.55f);
      const float bandTop = topBarH + columnH - contentPanesH;
      mainDrawList->AddRectFilled(winPos, winPos + ImVec2(uiSize.x, bandTop), dimCol);
      mainDrawList->AddRectFilled(winPos + ImVec2(0.f, bandTop), winPos + ImVec2(railW, topBarH + columnH), dimCol);
   }

   if (showNav)
   {
      // Top bar: darker band with letter-spaced uppercase categories, underlined
      // when selected — Table/Settings/Plugins left, About/Quit on the right.
      // Folded to two lines on portrait frames or on overflow: categories on the
      // first line, Game (left) and About/Quit (right) on the second
      mainDrawList->AddRectFilled(winPos, winPos + ImVec2(uiSize.x, topBarH), ImGui::GetColorU32(IM_COL32(0, 0, 0, 120)));
      mainDrawList->AddLine(winPos + ImVec2(0.f, topBarH), winPos + ImVec2(uiSize.x, topBarH), ImGui::GetColorU32(IM_COL32(255, 255, 255, 28)));
      // One left and one right accumulator per line: left-justified tabs advance
      // left-to-right, right-justified ones accumulate right-to-left, rightmost first
      float tabX[2] = { winPos.x + 28.f * dpi, winPos.x + 28.f * dpi };
      float tabXRight[2] = { winPos.x + uiSize.x - 28.f * dpi, winPos.x + uiSize.x - 28.f * dpi };
      // The rail is empty for a group reduced to a single page (no rows are
      // rendered): activating the tab must then descend straight to the items,
      // the empty rail level being skipped by the render-time normalization
      const auto railEmpty = [&groups, this](const char* groupId)
      {
         if ("table"s == groupId)
            return false; // Always has the Table Info row
         if ("plugins"s == groupId)
            return m_pluginEntries.empty();
         for (const Group& g : groups)
            if (g.id == groupId)
               return g.entries.size() <= 1;
         return false; // Action tabs (Game/Quit) do not navigate to a page
      };
      m_navTabs.clear();
      for (const Tab& tab : tabs)
         m_navTabs.push_back({ tab.label, tab.activate, railEmpty(tab.group) });
      // The tab focus syncs to the selected group when entering the level
      if (m_navTabIndex < 0)
      {
         m_navTabIndex = 0;
         for (int i = 0; i < (int)tabs.size(); ++i)
            if (m_selectedGroup == tabs[i].group)
            {
               m_navTabIndex = i;
               break;
            }
      }
      m_navTabIndex = std::clamp(m_navTabIndex, 0, (int)tabs.size() - 1);
      const auto renderTab = [&](int i)
      {
         const Tab& tab = tabs[i];
         // On the folded bar, each tab renders on its assigned line at the
         // secondary style
         const int line = twoLineTabs ? tab.line : 0;
         const float size = tabSize * (twoLineTabs && line == 1 ? 0.85f : tab.scale);
         const float w = SpacedTextWidth(tabFont, size, tab.label, tracking);
         const float lineY = winPos.y + line * tabLineH;
         const float lineH = twoLineTabs ? tabLineH : topBarH;
         const float x = tab.right ? (tabXRight[line] -= w, tabXRight[line]) : tabX[line];
         ImGui::SetCursorScreenPos(ImVec2(x, lineY));
         const bool pressed = ImGui::InvisibleButton(std::format("##tab{}", tab.group).c_str(), ImVec2(w, lineH));
         const bool hovered = ImGui::IsItemHovered();
         const bool selected = m_selectedGroup == tab.group;
         const bool focused = m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::TopTabs && i == m_navTabIndex;
         const float textY = lineY + (lineH - size) * 0.5f;
         DrawSpacedText(mainDrawList, tabFont, size, ImVec2(x, textY), tracking, ImGui::GetColorU32(selected || hovered || focused ? ImGuiCol_Text : ImGuiCol_TextDisabled), tab.label);
         if (selected)
            mainDrawList->AddLine(ImVec2(x, textY + size + 5.f * dpi), ImVec2(x + w, textY + size + 5.f * dpi), ImGui::GetColorU32(IM_COL32(51, 153, 255, 255)), 2.f * dpi);
         if (focused)
            mainDrawList->AddRect(ImVec2(x - 8.f * dpi, lineY + 4.f * dpi), ImVec2(x + w + 8.f * dpi, lineY + lineH - 4.f * dpi), ImGui::GetColorU32(kFocusColor), 6.f * dpi, 0, 2.5f * dpi);
         if (tab.right)
            tabXRight[line] -= tabGap;
         else
            tabX[line] += w + tabGap;
         // A thin vertical bar sets the first tab (back to the game) apart —
         // only on the single-line bar; in two-line mode GAME is already set
         // apart on its own line and the bar would float mid-line
         if (i == 0 && !twoLineTabs)
         {
            const float sepX = x + w + tabGap * 0.5f;
            mainDrawList->AddLine(ImVec2(sepX, lineY + lineH * 0.28f), ImVec2(sepX, lineY + lineH * 0.72f), ImGui::GetColorU32(IM_COL32(255, 255, 255, 60)), 1.f * dpi);
         }
         if (pressed)
            tab.activate();
      };
      for (int i = 0; i < (int)tabs.size() && !tabs[i].right; ++i)
         renderTab(i);
      for (int i = (int)tabs.size() - 1; i >= 0 && tabs[i].right; --i)
         renderTab(i);
   }

   ImGui::SetCursorPos(ImVec2(0.f, topBarH));
   if (showNav)
   {
      // The plunger page's rail icon is the editor's plunger image, not a font glyph
      if (!m_plungerIcon && !m_plungerLoadFailed)
      {
         if (std::shared_ptr<const BaseTexture> tex = BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "editor/plunger.png")); tex != nullptr)
            m_plungerIcon = std::make_shared<Sampler>(m_player->m_renderer->m_renderDevice, "InGameUIPlungerIcon"s, tex, false);
         else
            m_plungerLoadFailed = true;
      }
      // Sub-pages below the rail level (plugin sheets, leaf pages, popups) only
      // show a back button in the rail, returning to their parent page
      bool inRail = activePath == rootPath;
      if (!inRail && m_selectedGroup == "plugins")
         for (const NavEntry& entry : m_pluginEntries)
            inRail |= entry.path == activePath;
      if (!inRail && selectedGroup != nullptr)
         for (const NavEntry* entry : selectedGroup->entries)
            inRail |= entry->path == activePath;
      const bool subPage = history.size() > 1 && !inRail;
      // Single-page categories keep the column width but need no entries
      const bool showRows = !subPage && selectedGroup != nullptr && (selectedGroup->entries.size() > 1 || m_selectedGroup == "table");
      // Build the rail's rows (also the flipper-nav order for this level)
      m_navRailRows.clear();
      if (subPage)
         m_navRailRows.push_back({ true, "Back"s, ""s });
      else
      {
         if (m_selectedGroup == "table")
            m_navRailRows.push_back({ false, "Table Info"s, rootPath });
         if (m_selectedGroup == "plugins")
            // Each plugin is a subsection; the plugins home itself is the content
            for (const NavEntry& entry : m_pluginEntries)
               m_navRailRows.push_back({ false, entry.label, entry.path });
         else if (showRows)
            for (const NavEntry* entry : selectedGroup->entries)
               m_navRailRows.push_back({ false, entry->label, entry->path });
      }
      // Pages without any focusable item have no item level: skip it (the
      // navigation source page is one — its table of contents is the rail)
      if (m_navLevel == NavLevel::PageItems && !HasNavFocusableItem(*contentPage))
         m_navLevel = NavLevel::SideRail;
      // The rail focus syncs to the selected entry when entering the level
      if (m_navRailIndex < 0)
      {
         m_navRailIndex = 0;
         for (int i = 0; i < (int)m_navRailRows.size(); ++i)
            if (!m_navRailRows[i].isBack && m_navRailRows[i].path == selectedNavPath)
            {
               m_navRailIndex = i;
               break;
            }
      }
      if (!m_navRailRows.empty())
         m_navRailIndex = std::clamp(m_navRailIndex, 0, (int)m_navRailRows.size() - 1);
      else if (m_navLevel == NavLevel::SideRail)
         m_navLevel = NavLevel::TopTabs; // An empty rail level is skipped
   }

   // Shared by the full rail, the folded icon column and the deployed overlay:
   // without labels the row only shows its icon, centered on the column
   const auto renderRailRows = [&](bool labels)
   {
      for (int i = 0; i < (int)m_navRailRows.size(); ++i)
      {
         const RailNav& row = m_navRailRows[i];
         const bool focused = labels && m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::SideRail && i == m_navRailIndex;
         const char* icon = row.isBack ? ICON_LC_CIRCLE_ARROW_LEFT : row.path == rootPath ? ICON_LC_INFO : m_selectedGroup == "plugins" ? ICON_LC_BOX : PathIcon(row.path);
         const auto iconImage = row.path == "settings/plunger" ? m_plungerIcon : nullptr;
         const bool selected = !row.isBack && row.path == selectedNavPath;
         if (NavRow(icon, labels ? row.label.c_str() : nullptr, selected, row.isBack ? "back" : row.path.c_str(), iconImage, focused))
         {
            if (row.isBack)
               m_inGameUI.NavigateBack();
            else
               m_inGameUI.NavigateTo(row.path);
         }
      }
   };

   if (showNav)
   {
      // The folded rail deploys over the item pane while interacted with: hovered,
      // or when the flipper navigation reaches the rail level. The deployed width
      // is kept in the hover test (hysteresis) so it doesn't flicker-collapse
      const bool navRail = m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::SideRail;
      const ImVec2 railMin(uiPos.x, uiPos.y + topBarH);
      const bool railHovered = ImGui::IsMouseHoveringRect(railMin, railMin + ImVec2(m_railDeployed ? subW : railW, columnH));
      m_railDeployed = foldRail && (navRail || railHovered);
      // The folded rail smoothly widens to its deployed width and back
      m_railDeployAnim += ((m_railDeployed ? 1.f : 0.f) - m_railDeployAnim) * std::min(1.f, elapsedS * 10.f);
      if (m_railDeployAnim < 0.001f)
         m_railDeployAnim = 0.f;
      else if (m_railDeployAnim > 0.999f)
         m_railDeployAnim = 1.f;
      if (foldRail)
         ImGui::Dummy(ImVec2(railW, columnH)); // Reserve the folded slot; the rail is drawn later, over the item pane
      else
      {
         // The selected category's sub-pages: flat list, icon + large label, accent
         // side bar and slight highlight on the selected one
         ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(18, 18, 26, 90));
         ImGui::BeginChild("##SubPages", ImVec2(railW, columnH), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
         ImGui::PopStyleColor();
         UpdateDragScroll();
         ImGui::SetCursorPosY(16.f * dpi);
         renderRailRows(true);
         ImGui::Dummy(ImVec2(0.f, 0.f)); // An item must follow the leading SetCursorPosY (empty rail)
         ImGui::EndChild();
         mainDrawList->AddLine(winPos + ImVec2(railW, topBarH), winPos + ImVec2(railW, topBarH + columnH), ImGui::GetColorU32(IM_COL32(255, 255, 255, 25)));
      }
      ImGui::SameLine(0.f, 0.f);
   }

   // The content column always follows the page open/close animation
   ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * pageFade);
   ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(18, 18, 26, 90));
   ImGui::BeginChild("##Content", ImVec2(contentSize.x, contentSize.y - contentPanesH), ImGuiChildFlags_None);
   ImGui::PopStyleColor();
   ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.25f);
   RenderContentPage(*contentPage, contentPanes, contentPage == rootPage, activePath);
   ImGui::PopFont();
   contentPage->SetWindowState(contentPos, contentSize, ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));
   ImGui::EndChild();
   ImGui::PopStyleVar();
   float paneY = cabinetInset ? uiPos.y - 12.f * dpi - panesH + style.ItemSpacing.y : contentPos.y + contentSize.y - panesH + style.ItemSpacing.y;
   const float paneBandX = cabinetInset ? uiPos.x : contentPos.x;
   for (const auto& [pane, paneSize] : attachedPanes)
   {
      pane->render(ImVec4(paneBandX + (paneBandW - paneSize.x) * 0.5f, paneY, paneSize.x, paneSize.y));
      paneY += paneSize.y + style.ItemSpacing.y;
   }

   // Bottom input helper bar: the verbs follow the focused navigation level
   if (showNav)
   {
      // Chips show the actual input binding (what to press), not the action name
      const auto findAction = [this](unsigned int id) -> const InputAction*
      {
         for (const auto& action : m_player->m_pininput.GetInputActions())
            if (action->GetActionId() == id)
               return action.get();
         return nullptr;
      };
      const auto mappingLabel = [&findAction](unsigned int id) -> string
      {
         const InputAction* action = findAction(id);
         return action != nullptr && action->IsMapped() ? action->GetMappingLabel() : "?"s;
      };
      const char* navVerb;
      const char* selVerb;
      switch (m_navLevel)
      {
      case NavLevel::TopTabs:
         navVerb = "Navigate Categories";
         selVerb = "Open Category";
         break;
      case NavLevel::SideRail:
         navVerb = "Navigate Sections";
         selVerb = "Open Section";
         break;
      default:
         navVerb = "Navigate Items";
         selVerb = "Adjust / Activate Item";
         break;
      }
      // [key chip] label pairs, like the live editor's status bar hints
      struct Hint
      {
         string key;
         const char* label;
      };
      vector<Hint> hints;
      //hints.push_back({ mappingLabel(m_player->m_pininput.GetUIUpActionId()) + " / "s + mappingLabel(m_player->m_pininput.GetUIDownActionId()), navVerb });
      //hints.push_back({ mappingLabel(m_player->m_pininput.GetUILeftActionId()) + " / "s + mappingLabel(m_player->m_pininput.GetUIRightActionId()), selVerb });
      //hints.push_back({ mappingLabel(m_player->m_pininput.GetExitGameActionId()), "Back" });
      hints.push_back({ "UI Next/Prev"s, navVerb });
      hints.push_back({ "UI Left/Right"s, selVerb });
      hints.push_back({ "Exit"s, "Back" });
      if (m_navLevel == NavLevel::PageItems)
      {
         // Only advertise the footer actions the page model currently provides
         // (defaults when not all default, undo/save when modified)
         int defaultsIndex = -1, undoIndex = -1, saveIndex = -1;
         for (int i = 0; i < (int)contentPage->GetItems().size(); ++i)
         {
            using enum InGameUIItem::Type;
            switch (contentPage->GetItems()[i]->m_type)
            {
            case ResetToDefaults: defaultsIndex = i; break;
            case ResetToStoredValues: undoIndex = i; break;
            case SaveChanges: saveIndex = i; break;
            default: break;
            }
         }
         if (defaultsIndex >= 0)
            //hints.push_back({ mappingLabel(m_player->m_pininput.GetLaunchBallActionId()), "Defaults" });
            hints.push_back({ "Launch"s, "Defaults" });
         if (undoIndex >= 0)
         {
            // Any credit slot undoes: advertise the first one that is actually mapped
            unsigned int creditId = m_player->m_pininput.GetAddCreditActionId(0);
            for (int i = 0; i < 4; ++i)
               if (const InputAction* action = findAction(m_player->m_pininput.GetAddCreditActionId(i)); action != nullptr && action->IsMapped())
               {
                  creditId = action->GetActionId();
                  break;
               }
            //hints.push_back({ mappingLabel(creditId), "Undo" });
            hints.push_back({ "Credit"s, "Undo" });
         }
         if (saveIndex >= 0)
            //hints.push_back({ mappingLabel(m_player->m_pininput.GetStartActionId()), "Save" });
            hints.push_back({ "Start"s, "Save" });
      }

      // Chips are sized on the glyphs' ink bounds like in the live editor:
      // CalcTextSize only covers advances and the line height, and icon glyphs
      // would overflow the chip. One chip height for the whole row.
      const float chipPadX = 5.f * dpi, chipPadY = 1.f * dpi, gap = 6.f * dpi, hintGap = 18.f * dpi;
      const float textH = ImGui::GetFontSize();
      const auto keyInkBounds = [textH](const string& key, float& x0, float& x1, float& y0, float& y1)
      {
         const ImVec4 ink = TextInkBounds(key.c_str());
         x0 = ink.x;
         x1 = ImMax(ink.z, ImGui::CalcTextSize(key.c_str()).x); // union with the layout box
         y0 = ink.y;
         y1 = ImMax(ink.w, textH);
      };
      float chipTop = 0.f, chipBottom = textH;
      const auto measureHints = [&]() -> float
      {
         chipTop = 0.f;
         chipBottom = textH;
         float w = 0.f;
         for (const Hint& hint : hints)
         {
            float x0, x1, y0, y1;
            keyInkBounds(hint.key, x0, x1, y0, y1);
            chipTop = ImMin(chipTop, y0);
            chipBottom = ImMax(chipBottom, y1);
            w += (x1 - x0) + 2.f * chipPadX + gap + ImGui::CalcTextSize(hint.label).x + hintGap;
         }
         return w - hintGap;
      };
      float totalW = measureHints();
      // The two first hints are the universal UI navigation hints, the same on
      // every page: drop them when the bar cannot fit all the hints
      if (totalW > uiSize.x && hints.size() > 2)
      {
         hints.erase(hints.begin(), hints.begin() + 2);
         totalW = measureHints();
      }

      const float hintY = winPos.y + topBarH + columnH;
      const float textY = hintY + (hintH - textH) * 0.5f;
      mainDrawList->AddRectFilled(ImVec2(winPos.x, hintY), ImVec2(winPos.x + uiSize.x, hintY + hintH), ImGui::GetColorU32(IM_COL32(0, 0, 0, 120)));
      mainDrawList->AddLine(ImVec2(winPos.x, hintY), ImVec2(winPos.x + uiSize.x, hintY), ImGui::GetColorU32(IM_COL32(255, 255, 255, 28)));
      float x = winPos.x + (uiSize.x - totalW) * 0.5f;
      for (const Hint& hint : hints)
      {
         float inkX0, inkX1, inkY0, inkY1;
         keyInkBounds(hint.key, inkX0, inkX1, inkY0, inkY1);
         const ImVec2 chipMin(x, textY + chipTop - chipPadY);
         const ImVec2 chipMax(x + (inkX1 - inkX0) + 2.f * chipPadX, textY + chipBottom + chipPadY);
         mainDrawList->AddRectFilled(chipMin, chipMax, ImGui::GetColorU32(IM_COL32(255, 255, 255, 20)), 3.f * dpi);
         mainDrawList->AddRect(chipMin, chipMax, ImGui::GetColorU32(IM_COL32(255, 255, 255, 50)), 3.f * dpi);
         mainDrawList->AddText(ImVec2(chipMin.x + chipPadX - inkX0, chipMin.y + chipPadY - chipTop), ImGui::GetColorU32(ImGuiCol_Text), hint.key.c_str());
         mainDrawList->AddText(ImVec2(chipMax.x + gap, textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), hint.label);
         x = chipMax.x + gap + ImGui::CalcTextSize(hint.label).x + hintGap;
      }
   }

   // The folded rail, drawn last so it deploys over the item pane: it widens
   // from the icon column to the full rail (the labels reveal as it grows —
   // the icons stay in place since the folded width is exactly the icon slot)
   // and goes opaque to stay readable over the content
   if (showNav && foldRail)
   {
      const float railAnimW = railW + (subW - railW) * m_railDeployAnim;
      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(18, 18, 26, 90 + (int)(150.f * m_railDeployAnim)));
      ImGui::SetCursorScreenPos(ImVec2(uiPos.x, uiPos.y + topBarH));
      ImGui::BeginChild("##SubPages", ImVec2(railAnimW, columnH), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
      ImGui::PopStyleColor();
      UpdateDragScroll();
      ImGui::SetCursorPosY(16.f * dpi);
      renderRailRows(m_railDeployAnim > 0.f);
      ImGui::Dummy(ImVec2(0.f, 0.f));
      ImGui::EndChild();
      // The folded slot boundary fades out as the rail deploys over it
      mainDrawList->AddLine(winPos + ImVec2(railW, topBarH), winPos + ImVec2(railW, topBarH + columnH), ImGui::GetColorU32(IM_COL32(255, 255, 255, (int)(25.f * (1.f - m_railDeployAnim)))));
   }

   ImGui::PopStyleVar(); // Alpha fade
   ImGui::End();

   for (InGameUIPage* page : pages)
      RenderInputActionPopup(m_player, *page);
}

// Whether the item takes part in the flipper-nav item cycle: it must be
// selectable and actually rendered in the content pane. The core Back action is
// rendered by the rail row / back navigation, and the navigation source page
// does not re-render its table of contents (it is the top tabs and rail)
bool InGameUIDesktopRenderer::IsItemNavFocusable(const InGameUIPage& page, int index) const
{
   const auto& items = page.GetItems();
   if (index < 0 || index >= (int)items.size())
      return false;
   const InGameUIItem& item = *items[index];
   if (!item.IsSelectable() || item.m_type == InGameUIItem::Type::Back)
      return false;
   if (&page == m_inGameUI.GetRootPage()
      && (item.m_type == InGameUIItem::Type::Navigation || (item.m_type == InGameUIItem::Type::Label && item.m_labelType == InGameUIItem::LabelType::Header)))
      return false;
   return true;
}

// Whether the page has at least one item taking part in the flipper-nav item
// cycle; the navigation source page always reports none since its table of
// contents is navigated through the top tabs and the rail, not the item pane
bool InGameUIDesktopRenderer::HasNavFocusableItem(const InGameUIPage& page) const
{
   if (&page == m_inGameUI.GetRootPage())
      return false;
   const auto& items = page.GetItems();
   for (int i = 0; i < (int)items.size(); ++i)
      if (IsItemNavFocusable(page, i))
         return true;
   return false;
}

// Moves the page selection to the closest focusable item in the given direction
// if it does not already rest on one (the page model also selects items this
// renderer does not draw); does nothing when no item is focusable
void InGameUIDesktopRenderer::EnsureNavSelection(InGameUIPage& page, int direction)
{
   const auto& items = page.GetItems();
   const int n = (int)items.size();
   if (n == 0 || IsItemNavFocusable(page, page.GetSelectedItemIndex()))
      return;
   int sel = page.GetSelectedItemIndex();
   if (sel < 0 || sel >= n)
      sel = direction >= 0 ? -1 : n; // The first step then lands on an edge item
   for (int i = 0; i < n && !IsItemNavFocusable(page, sel); ++i)
      sel = (sel + direction + n) % n;
   if (IsItemNavFocusable(page, sel))
   {
      page.SelectItem(sel);
      page.SetItemScroll(direction > 0 ? 0.f : 10000.f);
   }
}

// First navigation press while the focus is not shown: engages flipper
// navigation, landing on the section rail when on the navigation source page
// (the top of the hierarchy), and normalizes the item selection
void InGameUIDesktopRenderer::EngageNav(InGameUIPage* page, int direction)
{
   m_navWasActive = true;
   if (page != nullptr && page == m_inGameUI.GetRootPage())
   {
      // The rail is the root page's table of contents: land its focus on the
      // currently shown section instead of the top categories, where the ring
      // would sit on the already-selected tab
      m_navLevel = NavLevel::SideRail;
      m_navRailIndex = -1; // Syncs to the selected section at next render
   }
   if (page != nullptr)
      EnsureNavSelection(*page, direction);
}

// Moves the focus within the current navigation level (cycling); the first
// press only engages flipper navigation and normalizes the item selection
bool InGameUIDesktopRenderer::NavStep(InGameUIPage* page, int direction)
{
   if (m_modalActive)
      return false; // A modal pane owns the page's input (loading screen, ...)
   if (!m_navWasActive)
   {
      EngageNav(page, direction);
      return true;
   }
   switch (m_navLevel)
   {
   case NavLevel::TopTabs:
      if (!m_navTabs.empty())
         m_navTabIndex = (m_navTabIndex + direction + (int)m_navTabs.size()) % (int)m_navTabs.size();
      break;
   case NavLevel::SideRail:
      if (!m_navRailRows.empty())
         m_navRailIndex = (m_navRailIndex + direction + (int)m_navRailRows.size()) % (int)m_navRailRows.size();
      break;
   case NavLevel::PageItems:
      if (page != nullptr)
      {
         // Keep the page's own step (pages may intercept it, e.g. the loading
         // screen opens its options on any navigation input), then skip over the
         // items this renderer does not draw
         if (direction < 0)
            page->SelectPrevItem();
         else
            page->SelectNextItem();
         EnsureNavSelection(*page, direction);
      }
      break;
   }
   return true;
}

bool InGameUIDesktopRenderer::OnUINavigatePrev(InGameUIPage* page) { return NavStep(page, -1); }

bool InGameUIDesktopRenderer::OnUINavigateNext(InGameUIPage* page) { return NavStep(page, 1); }

// UI left/right: selects the focused element on the tab/rail levels (pressing
// descends to the first element of the level below), adjusts the focused item
// on the page level
bool InGameUIDesktopRenderer::OnUIAdjust(InGameUIPage* page, float direction, bool isInitialPress)
{
   if (m_modalActive)
      return false; // A modal pane owns the page's input
   if (isInitialPress)
   {
      if (!m_navWasActive)
      {
         // The first press only engages the navigation focus instead of
         // blindly adjusting an element that has no visible focus
         EngageNav(page, direction < 0.f ? -1 : 1);
         return true;
      }
      switch (m_navLevel)
      {
      case NavLevel::TopTabs:
         if (m_navTabIndex >= 0 && m_navTabIndex < (int)m_navTabs.size())
         {
            const TabNav& tab = m_navTabs[m_navTabIndex];
            tab.activate();
            // Descend on the first item of the activated category's rail; the
            // tracked path follows the navigation so the frame render does not
            // reset the level we just descended to
            const auto& history = m_inGameUI.GetNavigationHistory();
            m_lastNavPath = history.empty() ? ""s : history.back();
            m_navRailIndex = 0;
            m_navLevel = tab.descendToItems ? NavLevel::PageItems : NavLevel::SideRail;
            if (tab.descendToItems)
               if (InGameUIPage* const active = m_inGameUI.GetActivePage())
               {
                  active->SelectItem(-1);
                  EnsureNavSelection(*active, 1);
               }
         }
         break;
      case NavLevel::SideRail:
         if (m_navRailIndex >= 0 && m_navRailIndex < (int)m_navRailRows.size())
         {
            const RailNav row = m_navRailRows[m_navRailIndex];
            if (row.isBack)
               m_inGameUI.NavigateBack();
            else
               m_inGameUI.NavigateTo(row.path);
            const auto& history = m_inGameUI.GetNavigationHistory();
            m_lastNavPath = history.empty() ? ""s : history.back();
            // Descend on the first item of the activated page
            m_navLevel = NavLevel::PageItems;
            if (InGameUIPage* const active = m_inGameUI.GetActivePage())
            {
               active->SelectItem(-1);
               EnsureNavSelection(*active, 1);
            }
         }
         break;
      case NavLevel::PageItems:
         if (page != nullptr)
            page->AdjustItem(direction, true);
         break;
      }
   }
   else if (m_navWasActive && m_navLevel == NavLevel::PageItems && page != nullptr)
   {
      page->AdjustItem(direction, false);
   }
   return true;
}

bool InGameUIDesktopRenderer::OnUIReleaseAdjust(InGameUIPage* page)
{
   if (m_modalActive)
      return false;
   if (m_navLevel == NavLevel::PageItems && page != nullptr)
      page->ReleaseAdjustedItem();
   return true;
}

// Back moves the focus up a level; at the top tab level it navigates back in
// the page history (or closes the UI on the root page)
bool InGameUIDesktopRenderer::OnUINavigateBack(InGameUIPage* page)
{
   if (m_modalActive || !m_navWasActive)
      return false; // A modal pane owns its input; mouse mode navigates the history directly
   switch (m_navLevel)
   {
   case NavLevel::PageItems:
      // On sub-pages the rail only offers a Back row: apply it right away (go
      // back one page) and land the rail focus on the row that led there
      if (!m_navRailRows.empty() && m_navRailRows.front().isBack)
      {
         m_inGameUI.NavigateBack();
         const auto& history = m_inGameUI.GetNavigationHistory();
         m_lastNavPath = history.empty() ? ""s : history.back();
      }
      m_navLevel = NavLevel::SideRail;
      m_navRailIndex = -1; // Resyncs on the row of the shown page
      return true;
   case NavLevel::SideRail:
      m_navLevel = NavLevel::TopTabs;
      m_navTabIndex = -1;
      return true;
   case NavLevel::TopTabs: return false;
   }
   return false;
}

// A sub-category row: icon + large label, accent side bar and slight background
// highlight when selected, fill highlight when hovered or keyboard-focused
bool InGameUIDesktopRenderer::NavRow(const char* icon, const char* label, bool selected, const char* id, const std::shared_ptr<Sampler>& iconImage, bool focused)
{
   const ImGuiStyle& style = ImGui::GetStyle();
   const float dpi = m_player->m_liveUI->GetDPI();
   ImFont* const font = ImGui::GetFont();
   const float labelSize = ImGui::GetFontSize() * 1.45f;
   const float rowH = labelSize + 14.f * dpi;
   const ImVec2 pos = ImGui::GetCursorScreenPos();
   const float w = ImGui::GetContentRegionAvail().x;
   const bool pressed = ImGui::InvisibleButton(std::format("##{}", id).c_str(), ImVec2(w, rowH));
   const bool hovered = ImGui::IsItemHovered() || focused;
   // Keep the flipper-nav focused row inside the rail's visible area (the
   // cursor pos is in document space, the scroll adjusts the visible offset).
   // Bottom alignment is capped at the row top: a row taller than the pane
   // settles on its top edge instead of oscillating between the two alignments
   if (focused)
   {
      const float rowStartY = ImGui::GetCursorPosY() - rowH;
      if (rowStartY - ImGui::GetScrollY() < 0.f)
         ImGui::SetScrollY(rowStartY);
      else if (rowStartY + rowH - ImGui::GetScrollY() > ImGui::GetWindowHeight())
         ImGui::SetScrollY(std::min(rowStartY + rowH - ImGui::GetWindowHeight(), rowStartY));
   }
   ImDrawList* const dl = ImGui::GetWindowDrawList();
   // The highlight spans the full column width including both paddings so the
   // accent side bar sits flush on the column edge
   const float rowX = pos.x - style.WindowPadding.x;
   const float rowW = w + 2.f * style.WindowPadding.x;
   if (selected || hovered)
      dl->AddRectFilled(ImVec2(rowX, pos.y), ImVec2(rowX + rowW, pos.y + rowH), ImGui::GetColorU32(selected ? IM_COL32(255, 255, 255, 24) : IM_COL32(255, 255, 255, 12)));
   if (selected)
      dl->AddRectFilled(ImVec2(rowX, pos.y), ImVec2(rowX + 3.f * dpi, pos.y + rowH), ImGui::GetColorU32(kAccentColor));
   if (focused)
      dl->AddRect(ImVec2(rowX, pos.y), ImVec2(rowX + rowW, pos.y + rowH), ImGui::GetColorU32(kFocusColor), 4.f * dpi, 0, 2.5f * dpi);
   const float slot = labelSize + 8.f * dpi;
   const ImU32 textCol = ImGui::GetColorU32(selected || hovered ? ImGuiCol_Text : ImGuiCol_TextDisabled);
   const float iconSide = std::min(labelSize, slot * 0.72f);
   // The icon keeps the same position folded or deployed: the folded column
   // width is exactly the icon slot, so anchoring on it never makes the icon move
   const float iconCX = slot * 0.5f;
   if (iconImage != nullptr)
      dl->AddImage(iconImage, ImVec2(pos.x + iconCX - iconSide * 0.5f, pos.y + (rowH - iconSide) * 0.5f), ImVec2(pos.x + iconCX + iconSide * 0.5f, pos.y + (rowH + iconSide) * 0.5f),
         ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), textCol);
   else
   {
      const ImVec4 ink = TextInkBounds(icon);
      dl->AddText(font, labelSize, ImVec2(pos.x + iconCX - (ink.z + ink.x) * 0.5f, pos.y + (rowH - (ink.w - ink.y)) * 0.5f - ink.y), textCol, icon);
   }
   if (label != nullptr)
      dl->AddText(font, labelSize, ImVec2(pos.x + slot + 12.f * dpi, pos.y + (rowH - labelSize) * 0.5f), textCol, label);
   return pressed;
}

// Drag-to-scroll for scrollable panes: press inside a pane, hold and move up/down
// to drag its content (VR controller trigger or mouse button), with a short
// inertial glide on release. Must be called right after the pane's BeginChild.
void InGameUIDesktopRenderer::UpdateDragScroll()
{
   constexpr float kDragThreshold = 6.f; // px before we commit to a drag
   constexpr float kFriction = 8.f; // deceleration rate (higher = quicker stop)
   constexpr float kVelocitySmooth = 0.2f; // EMA weight for velocity sampling
   const ImGuiIO& io = ImGui::GetIO();
   const float elapsedS = io.DeltaTime;
   const ImGuiID paneId = ImGui::GetCurrentWindow()->ID;

   if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
   {
      m_isDraggingScroll = false;
      m_dragScrollPaneId = paneId;
      m_dragScrollStartPos = ImGui::GetMousePos();
      m_dragScrollVelocity = 0.f;
   }
   if (paneId != m_dragScrollPaneId)
      return;
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
   {
      // Inertial glide keeps scrolling the pane briefly after release
      if (fabsf(m_dragScrollVelocity) > 0.5f)
      {
         ImGui::SetScrollY(ImGui::GetScrollY() + m_dragScrollVelocity * elapsedS);
         m_dragScrollVelocity *= max(0.f, 1.f - kFriction * elapsedS);
         if (fabsf(m_dragScrollVelocity) < 0.5f)
         {
            m_dragScrollVelocity = 0.f;
            m_dragScrollPaneId = 0;
         }
      }
      else
         m_dragScrollPaneId = 0;
      return;
   }
   if (!m_isDraggingScroll)
      m_isDraggingScroll = fabsf(ImGui::GetMousePos().y - m_dragScrollStartPos.y) > kDragThreshold;
   else
   {
      const float dy = io.MouseDelta.y;
      ImGui::SetScrollY(ImGui::GetScrollY() - dy);
      const float frameVel = (elapsedS > 0.f) ? (-dy / elapsedS) : 0.f;
      m_dragScrollVelocity = m_dragScrollVelocity * (1.f - kVelocitySmooth) + frameVel * kVelocitySmooth;
   }
}

void InGameUIDesktopRenderer::RenderContentPage(InGameUIPage& page, const vector<InGameUIPane>& panes, bool isNavSource, const string& pagePath)
{
   const ImGuiStyle& style = ImGui::GetStyle();
   const float dpi = m_player->m_liveUI->GetDPI();
   ImDrawList* const dl = ImGui::GetWindowDrawList();

   // Page title, with some breathing room below the top bar
   ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.f * dpi);
   const ImVec2 titlePos = ImGui::GetCursorScreenPos();
   ImFont* const font = ImGui::GetFont();
   const float titleSize = ImGui::GetFontSize() * 1.8f;
   const float titleLineH = titleSize + 8.f * dpi;
   // The navigation source page is shown as the "Table Info" sub-category: use
   // that label instead of its own ("Options & Settings") title
   const char* title = isNavSource ? "Table Info" : page.GetTitle().c_str();
   dl->AddText(font, titleSize, titlePos, ImGui::GetColorU32(ImGuiCol_Text), title);
   const float titleW = font->CalcTextSizeA(titleSize, FLT_MAX, 0.f, title).x;
   ImGui::SetCursorScreenPos(titlePos + ImVec2(0.f, (titleSize - ImGui::GetFrameHeight()) * 0.5f));
   dl->AddLine(titlePos + ImVec2(0.f, titleLineH), titlePos + ImVec2(titleW, titleLineH), ImGui::GetColorU32(IM_COL32(255, 255, 255, 60)));
   ImGui::SetCursorScreenPos(titlePos + ImVec2(0.f, titleLineH + 4.f * dpi));

   // A plugin landing page ("plugin/<id>") gets a dedicated subtitle: a one
   // liner with the description and "by <author>" on the left and the version
   // on the right, then a clickable homepage link
   std::shared_ptr<MsgPI::MsgPlugin> plugin;
   if (pagePath.starts_with("plugin/"s) && pagePath.find('/', 7) == string::npos)
      plugin = g_pplayer->m_pluginManager.GetPlugin(pagePath.substr(7));
   if (plugin != nullptr)
   {
      ImGui::PushFont(font, ImGui::GetFontSize() * 1.15f);
      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
      const float infoW = ImGui::GetContentRegionAvail().x;
      const ImVec2 linePos = ImGui::GetCursorScreenPos();
      const float lineH = ImGui::GetTextLineHeight();
      const ImU32 subCol = ImGui::GetColorU32(ImGuiCol_TextDisabled);
      float rightX = linePos.x + infoW;
      if (!plugin->m_version.empty())
      {
         const float versionW = ImGui::CalcTextSize(plugin->m_version.c_str()).x;
         rightX -= versionW;
         dl->AddText(ImVec2(rightX, linePos.y), subCol, plugin->m_version.c_str());
      }
      string byline = plugin->m_description;
      if (!plugin->m_author.empty())
         byline += (byline.empty() ? ""s : " "s) + "by "s + plugin->m_author;
      if (!byline.empty())
      {
         const ImVec4 clip(linePos.x, linePos.y, rightX - 8.f * dpi, linePos.y + lineH);
         dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), linePos, subCol, byline.c_str(), nullptr, 0.f, &clip);
      }
      ImGui::SetCursorScreenPos(linePos + ImVec2(0.f, lineH));
      if (!plugin->m_link.empty())
      {
         const ImVec2 linkPos = ImGui::GetCursorScreenPos();
         const ImVec2 linkSize = ImGui::CalcTextSize(plugin->m_link.c_str());
         const bool pressed = ImGui::InvisibleButton("##pluginlink", ImVec2(linkSize.x, linkSize.y));
         const bool linkHovered = ImGui::IsItemHovered();
         const ImU32 linkCol = ImGui::GetColorU32(linkHovered ? kAccentColor : IM_COL32(150, 180, 255, 255));
         dl->AddText(linkPos, linkCol, plugin->m_link.c_str());
         dl->AddLine(linkPos + ImVec2(0.f, linkSize.y), linkPos + ImVec2(linkSize.x, linkSize.y), linkCol);
         if (linkHovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
         if (pressed)
            SDL_OpenURL(plugin->m_link.c_str());
      }
      ImGui::PopStyleColor();
      ImGui::PopFont();
      ImGui::Spacing();
   }
   else if (!page.GetInfo().empty())
   {
      ImGui::PushFont(font, ImGui::GetFontSize() * 1.15f);
      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
      ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
      ImGui::TextWrapped("%s", page.GetInfo().c_str());
      ImGui::PopTextWrapPos();
      ImGui::PopStyleColor();
      ImGui::PopFont();
      ImGui::Spacing();
   }

   // Item list (scrollable), each row: label + info subtext / right aligned
   // control / thin separator; the action bar sits at the bottom of the pane
   const float footerH = page.IsAdjustable() ? ImGui::GetFrameHeight() + 18.f * dpi : 0.f;
   ImGui::BeginChild("PageItems", ImVec2(0.f, -footerH), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
   UpdateDragScroll();

   // Header panes sit at the top of the item list and scroll with it (e.g. the
   // input device diagram); they get the content width and may claim a share
   // of the visible height
   for (const InGameUIPane& pane : panes)
      if (pane.placement == InGameUIPane::Placement::Header && pane.render)
      {
         const ImVec2 avail = ImGui::GetContentRegionAvail();
         const ImVec2 paneSize = pane.measure ? pane.measure(avail.x, avail.y * 0.55f) : ImVec2(avail.x, avail.y * 0.5f);
         const ImVec2 panePos = ImGui::GetCursorScreenPos();
         pane.render(ImVec4(panePos.x, panePos.y, paneSize.x, paneSize.y));
         ImGui::SetCursorPosY(ImGui::GetCursorPosY() + paneSize.y + style.ItemSpacing.y);
      }

   RenderItems(page, panes, isNavSource);
   ImGui::Dummy(ImVec2(0.f, 0.f));
   ImGui::EndChild();
   if (footerH > 0.f)
      RenderFooterActions(page);
}

// Large icon + text action buttons (defaults / undo / save) in a bar at the
// bottom of the item pane
void InGameUIDesktopRenderer::RenderFooterActions(InGameUIPage& page)
{
   const ImGuiStyle& style = ImGui::GetStyle();
   const float dpi = m_player->m_liveUI->GetDPI();
   ImDrawList* const dl = ImGui::GetWindowDrawList();
   const auto& items = page.GetItems();
   int resetDefaultsIndex = -1;
   int resetStoredIndex = -1;
   int saveChangesIndex = -1;
   for (int i = 0; i < (int)items.size(); i++)
   {
      using enum InGameUIItem::Type;
      switch (items[i]->m_type)
      {
      case ResetToDefaults: resetDefaultsIndex = i; break;
      case ResetToStoredValues: resetStoredIndex = i; break;
      case SaveChanges: saveChangesIndex = i; break;
      default: break;
      }
   }

   const ImVec2 pos = ImGui::GetCursorScreenPos();
   const float barH = ImGui::GetFrameHeight() + 14.f * dpi;
   const float availW = ImGui::GetContentRegionAvail().x;
   dl->AddLine(pos, pos + ImVec2(availW, 0.f), ImGui::GetColorU32(IM_COL32(255, 255, 255, 40)));
   ImGui::SetCursorScreenPos(pos + ImVec2(0.f, 6.f * dpi));

   const int nButtons = page.GetSaveMode() != InGameUIPage::SaveMode::None ? 3 : 2;
   const float buttonW = (availW - (nButtons - 1) * style.ItemSpacing.x) / nButtons;
   const float buttonH = barH - 6.f * dpi;
   // Icon and label are drawn separately: the glyph ink overshoots the text line
   // box, so both are centered on their real ink bounds inside the frame
   const float textH = ImGui::GetTextLineHeight();
   const auto actionButton = [&](const char* icon, const char* label, int index, bool disabled)
   {
      const ImVec2 btnPos = ImGui::GetCursorScreenPos();
      ImGui::BeginDisabled(disabled);
      const bool navFocused = m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::PageItems && page.GetSelectedItemIndex() == index;
      // Keep the nav-focused button inside the content pane's visible area
      if (navFocused)
      {
         const float btnStartY = ImGui::GetCursorPosY();
         if (btnStartY - ImGui::GetScrollY() < 0.f)
            ImGui::SetScrollY(btnStartY);
         else if (btnStartY + buttonH - ImGui::GetScrollY() > ImGui::GetWindowHeight())
            ImGui::SetScrollY(std::min(btnStartY + buttonH - ImGui::GetWindowHeight(), btnStartY));
      }
      const bool highlighted = navFocused || ImGui::IsMouseHoveringRect(btnPos, btnPos + ImVec2(buttonW, buttonH));
      const bool pressed = ImGui::InvisibleButton(std::format("##{}", label).c_str(), ImVec2(buttonW, buttonH));
      const ImU32 bg = disabled ? ImGui::GetColorU32(ImGuiCol_FrameBg) : highlighted ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Button);
      dl->AddRectFilled(btnPos, btnPos + ImVec2(buttonW, buttonH), bg, style.FrameRounding);
      if (navFocused)
         dl->AddRect(btnPos, btnPos + ImVec2(buttonW, buttonH), ImGui::GetColorU32(kFocusColor), style.FrameRounding, 0, 2.5f * dpi);
      const ImVec4 iconInk = TextInkBounds(icon);
      const float labelW = ImGui::CalcTextSize(label).x;
      const float gap = 10.f * dpi;
      const float contentW = (iconInk.z - iconInk.x) + gap + labelW;
      const float startX = btnPos.x + (buttonW - contentW) * 0.5f;
      const ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
      dl->AddText(ImVec2(startX - iconInk.x, btnPos.y + (buttonH - (iconInk.w - iconInk.y)) * 0.5f - iconInk.y), col, icon);
      dl->AddText(ImVec2(startX + (iconInk.z - iconInk.x) + gap, btnPos.y + (buttonH - textH) * 0.5f), col, label);
      if (pressed)
      {
         page.SelectItem(index);
         page.AdjustItem(1.f, true);
      }
      ImGui::EndDisabled();
      ImGui::SameLine(0.f, style.ItemSpacing.x);
   };

   actionButton(ICON_LC_HEART, "Reset to defaults", resetDefaultsIndex, resetDefaultsIndex < 0);
   actionButton(ICON_LC_UNDO_2, "Undo changes", resetStoredIndex, resetStoredIndex < 0);
   if (nButtons == 3)
      actionButton(ICON_LC_SAVE, "Save changes", saveChangesIndex, saveChangesIndex < 0);
}

void InGameUIDesktopRenderer::RenderItems(InGameUIPage& page, const vector<InGameUIPane>& panes, bool isNavSource)
{
   const ImGuiStyle& style = ImGui::GetStyle();
   const float dpi = m_player->m_liveUI->GetDPI();
   const auto& items = page.GetItems();
   const float itemPadding = 6.f * dpi;
   const float itemAreaW = ImGui::GetContentRegionAvail().x;
   const float markerW = ImGui::GetFrameHeight();
   const float ctlW = std::clamp((itemAreaW - markerW) * 0.42f, 180.f * dpi, 420.f * dpi);
   const float labelW = itemAreaW - ctlW - markerW - 2.f * style.ItemSpacing.x;
   const float textH = ImGui::GetTextLineHeight();
   const float frameH = ImGui::GetFrameHeight();
   ImDrawList* const itemDrawList = ImGui::GetWindowDrawList();
   const ImU32 hoveredBackgroundColor = ImGui::GetColorU32(IM_COL32(0, 0, 128, 50));
   const ImU32 separatorColor = ImGui::GetColorU32(IM_COL32(255, 255, 255, 22));

   for (int i = 0; i < (int)items.size(); i++)
   {
      // A value change handled while rendering a previous item may have requested a page
      // rebuild: it is deferred to the next frame, so stop before the now-stale items
      if (page.IsRebuildRequested())
         break;

      using enum InGameUIItem::Type;
      const auto& item = items[i];

      // Skip as these items are rendered in the header
      if (item->m_type == Back || item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToStoredValues)
         continue;
      // The navigation source page does not re-render its table of contents in the content column
      if (isNavSource && (item->m_type == Navigation || (item->m_type == Label && item->m_labelType == InGameUIItem::LabelType::Header)))
         continue;

      // Section headers get extra breathing room above them
      if (item->m_type == Label && item->m_labelType == InGameUIItem::LabelType::Header)
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, 24.f * dpi));

      const bool navHovered = m_inGameUI.IsFlipperNav() && m_navLevel == NavLevel::PageItems && i == page.GetSelectedItemIndex();
      if (navHovered)
         ImGui::SetScrollHereY();
      const ImVec2 rowPos = ImGui::GetCursorScreenPos();
      const float rowStartY = ImGui::GetCursorPosY();
      const string& itemLabel = (item->m_type == Property) ? item->m_property->m_label : item->m_label;

      // Label items have custom layouts (info text, section headers, markdown blocks)
      if (item->m_type == Label)
      {
         ImGui::SetCursorScreenPos(rowPos + ImVec2(0.f, itemPadding));
         switch (item->m_labelType)
         {
         case InGameUIItem::LabelType::Info:
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            ImGui::TextWrapped("%s", item->m_label.c_str());
            ImGui::PopStyleColor();
            break;
         case InGameUIItem::LabelType::Header:
         {
            // Section titles inside a page (e.g. "Performance & Troubleshooting")
            ImGui::PushFont(ImGui::GetFont(), ImGui::GetFontSize() * 1.15f);
            ImGui::Text("%s", item->m_label.c_str());
            ImGui::PopFont();
            const ImVec2 max = ImGui::GetItemRectMax();
            itemDrawList->AddLine(ImVec2(rowPos.x, max.y + 2.f * dpi), ImVec2(rowPos.x + itemAreaW, max.y + 2.f * dpi), ImGui::GetColorU32(IM_COL32(255, 255, 255, 140)));
            break;
         }
         case InGameUIItem::LabelType::Markdown:
            m_player->m_liveUI->SetMarkdownStartId(ImGui::GetItemID());
            ImGui::Markdown(item->m_label.c_str(), item->m_label.length(), m_player->m_liveUI->GetMarkdownConfig());
            break;
         }
         const float itemEndY = ImGui::GetCursorPosY();
         const float itemHeight = itemEndY - rowStartY;
         // Markdown blocks are selected through flipper navigation (for scrolling);
         // hovering them with the mouse does not highlight them
         if (item->m_labelType == InGameUIItem::LabelType::Markdown && navHovered)
         {
            itemDrawList->AddRect(rowPos, rowPos + ImVec2(itemAreaW, itemHeight), ImGui::GetColorU32(kFocusColor), 3.f * dpi, 0, 2.5f * dpi);
            if (navHovered)
            {
               // Scrollable markdown item: scroll within its bounds (item scroll is adjusted by AdjustItem)
               if (itemHeight > ImGui::GetWindowHeight())
               {
                  page.SetItemScroll(clamp(page.GetItemScroll(), 0.f, itemHeight - ImGui::GetWindowHeight()));
                  ImGui::SetScrollY(rowStartY + page.GetItemScroll());
               }
            }
         }
         // A markdown item taller than the pane scrolls inside itself via
         // GetItemScroll: the pane scroll is already driven above
         if (navHovered && item->m_labelType == InGameUIItem::LabelType::Markdown && itemHeight <= ImGui::GetWindowHeight())
         {
            if (rowStartY - ImGui::GetScrollY() < 0.f)
               ImGui::SetScrollY(rowStartY);
            else if (rowStartY + itemHeight - ImGui::GetScrollY() > ImGui::GetWindowHeight())
               ImGui::SetScrollY(std::min(rowStartY + itemHeight - ImGui::GetWindowHeight(), rowStartY));
         }
         ImGui::SetCursorPosY(itemEndY + itemPadding);
         continue;
      }

      // Standard row: label + info subtext on the left, control on the right;
      // in portrait frames the label takes the full width and the control is
      // stacked under it, spanning nearly the full row width
      const bool stacked = m_stackedItemRows && (item->m_type == Property || item->m_type == ActionInputMapping);
      const float rowLabelW = stacked ? itemAreaW - markerW - style.ItemSpacing.x : labelW;
      float subH = 0.f;
      if (!item->m_tooltip.empty())
         subH = style.ItemSpacing.y * 0.5f + ImGui::CalcTextSize(item->m_tooltip.c_str(), nullptr, false, rowLabelW).y;
      const float rowH = std::max(textH + subH, frameH) + itemPadding * 2.f + (stacked ? frameH + style.ItemSpacing.y : 0.f);
      const ImVec2 rowMax = rowPos + ImVec2(itemAreaW, rowH);
      const bool isMouseOver
         = ImGui::IsWindowHovered() && ImGui::GetMousePos().x >= rowPos.x && ImGui::GetMousePos().x <= rowMax.x && ImGui::GetMousePos().y >= rowPos.y && ImGui::GetMousePos().y <= rowMax.y;

      // Navigation and runnable rows are a whole clickable target; other rows highlight on hover
      bool rowPressed = false;
      if (item->m_type == Navigation || item->m_type == Runnable)
      {
         ImGui::SetCursorScreenPos(rowPos);
         rowPressed = ImGui::InvisibleButton(std::format("##Item{}", i).c_str(), ImVec2(itemAreaW, rowH));
      }
      const bool hovered = navHovered || (!m_inGameUI.IsFlipperNav() && (isMouseOver || rowPressed) && item->IsSelectable());
      if (hovered)
      {
         itemDrawList->AddRectFilled(rowPos, rowMax, hoveredBackgroundColor, 3.f * dpi);
         if (navHovered)
            itemDrawList->AddRect(rowPos, rowMax, ImGui::GetColorU32(kFocusColor), 3.f * dpi, 0, 2.5f * dpi);
      }

      if (navHovered)
      {
         ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_FrameBgActive]);
         ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_FrameBgActive]);
         ImGui::PushStyleColor(ImGuiCol_SliderGrab, style.Colors[ImGuiCol_SliderGrabActive]);
         ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonActive]);
         ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_HeaderActive]);
         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_HeaderActive]);
      }

      // Label + info subtext
      ImGui::SetCursorScreenPos(rowPos + ImVec2(0.f, itemPadding));
      TextWithEllipsis(itemLabel, rowLabelW);
      if (subH > 0.f)
      {
         ImGui::SetCursorScreenPos(rowPos + ImVec2(0.f, itemPadding + textH + style.ItemSpacing.y * 0.5f));
         ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
         ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + rowLabelW);
         ImGui::TextWrapped("%s", item->m_tooltip.c_str());
         ImGui::PopTextWrapPos();
         ImGui::PopStyleColor();
      }

      // Control, vertically centered across the full row height so it can be
      // taller than a single line when the info subtext makes the row grow;
      // stacked rows place it in a band under the label, spanning nearly the
      // whole row with a small inset for readability and the marker column
      const float ctlPadX = stacked ? 12.f * dpi : 0.f;
      const float rowCtlW = stacked ? itemAreaW - markerW - style.ItemSpacing.x - 2.f * ctlPadX : ctlW;
      const ImVec2 ctlPos(stacked ? rowPos.x + ctlPadX : rowMax.x - markerW - ctlW, stacked ? rowMax.y - itemPadding - frameH : rowPos.y + (rowH - frameH) * 0.5f);
      switch (item->m_type)
      {
      case Navigation:
      case Runnable:
      {
         // The chevron glyph ink overshoots the text layout box: draw its ink vertically centered instead
         const ImVec4 chevronInk = TextInkBounds(ICON_LC_CHEVRONS_RIGHT);
         itemDrawList->AddText(ImVec2(rowMax.x - (chevronInk.z - chevronInk.x) - 8.f * dpi - chevronInk.x, rowPos.y + (rowH - (chevronInk.w - chevronInk.y)) * 0.5f - chevronInk.y),
            ImGui::GetColorU32(ImGuiCol_TextDisabled), ICON_LC_CHEVRONS_RIGHT);
         break;
      }

      case ActionInputMapping:
      {
         ImGui::SetCursorScreenPos(ctlPos);
         const float stateW = ImGui::CalcTextSize(ICON_LC_DISC).x + style.ItemSpacing.x;
         if (item->m_inputAction->IsMapped())
         {
            if (IconButtonInk(ICON_LC_X, std::format("##Item{:d}", i).c_str()))
            {
               item->m_inputAction->ClearMapping();
               if (item->m_inputAction->IsNavigationAction())
                  page.RequestInputActionPopup(*item);
            }
            ImGui::SameLine();
         }
         const string mappingLabel = item->m_inputAction->GetMappingLabel();
         const float mapButtonWidth = rowCtlW - (ImGui::GetCursorScreenPos().x - ctlPos.x) - stateW - style.ItemSpacing.x;
         if (ImGui::Button(std::format("{}##Item{}", mappingLabel, i).c_str(), ImVec2(mapButtonWidth, 0.f)))
            page.RequestInputActionPopup(*item);
         if (ImGui::CalcTextSize(mappingLabel.c_str()).x >= mapButtonWidth - style.ItemSpacing.x * 2.f)
            ImGui::SetItemTooltip("%s", mappingLabel.c_str());
         ImGui::SameLine(0.f, style.ItemSpacing.x);
         const string state = item->m_inputAction->IsPressed() ? ICON_LC_DISC : ICON_LC_CIRCLE;
         ImGui::Text("%s", state.c_str());
         break;
      }

      case Property:
         ImGui::SetCursorScreenPos(ctlPos);
         switch (item->m_property->m_type)
         {
         case VPX::Properties::PropertyDef::Type::Float:
         {
            auto prop = dynamic_cast<VPX::Properties::FloatPropertyDef*>(item->m_property.get());
            float v = item->GetFloatValue() * item->m_floatValueDisplayScale;
            ImGui::SetNextItemWidth(rowCtlW);
            ImGui::SliderFloat(std::format("##Item{}", i).c_str(), &v, prop->m_min * item->m_floatValueDisplayScale, prop->m_max * item->m_floatValueDisplayScale, item->m_format.c_str(),
               ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            const bool sliderActive = ImGui::IsItemActive();
            page.SetRebuildDeferred(sliderActive);
            item->SetValue(v / item->m_floatValueDisplayScale);
            page.SetRebuildDeferred(false);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Int:
         {
            auto prop = dynamic_cast<VPX::Properties::IntPropertyDef*>(item->m_property.get());
            int v = item->GetIntValue();
            const auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
            if (id.has_value()
               && (((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) || (Settings::m_propPlayer_PlayfieldHeight.index == id.value().index)) && !m_inGameUI.IsFlipperNav()))
            {
               // Special handling for editing main window size as mouse interaction would break (since the control is moved/resized while interacted)
               const float butWidth = IconButtonInkSize(ICON_LC_MOVE_HORIZONTAL).x;
               ImGui::SetNextItemWidth(rowCtlW - style.ItemSpacing.x - butWidth);
               ImGui::InputInt(std::format("##Item{}", i).c_str(), &v, 1, 100, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank);
               ImGui::SameLine();
               if (IconButtonInk((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) ? ICON_LC_MOVE_HORIZONTAL : ICON_LC_MOVE_VERTICAL, std::format("##Item{}", i).c_str()))
                  v = prop->m_max;
            }
            else
            {
               ImGui::SetNextItemWidth(rowCtlW);
               ImGui::SliderInt(std::format("##Item{}", i).c_str(), &v, prop->m_min, prop->m_max, item->m_format.c_str(), ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            }
            const bool sliderActive = ImGui::IsItemActive();
            page.SetRebuildDeferred(sliderActive);
            item->SetValue(v);
            page.SetRebuildDeferred(false);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Enum:
         {
            auto prop = dynamic_cast<VPX::Properties::EnumPropertyDef*>(item->m_property.get());
            int v = item->GetIntValue() - prop->m_min;
            ImGui::SetNextItemWidth(rowCtlW);
            ImGui::Combo(
               std::format("##Item{}", i).c_str(), &v,
               [](void* data, int idx)
               {
                  const auto* vec = static_cast<const vector<string>*>(data);
                  if (idx < 0 || idx >= (int)vec->size())
                     return "";
                  return vec->at(idx).c_str();
               },
               (void*)&prop->m_values, (int)prop->m_values.size());
            item->SetValue(prop->m_min + v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Bool:
         {
            bool v = item->GetBoolValue();
            // The toggle uses the extra row height offered by the two-line layout
            const float toggleH = stacked ? frameH : std::min(rowH - 2.f * itemPadding, frameH * 1.5f);
            ImGui::SetCursorScreenPos(ImVec2(ctlPos.x, stacked ? rowMax.y - itemPadding - toggleH : rowPos.y + (rowH - toggleH) * 0.5f));
            RenderToggle(m_player, std::format("{}##Item{}", item->m_property->m_label, i), ImVec2(rowCtlW, toggleH), v);
            item->SetValue(v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::String:
         {
            auto prop = dynamic_cast<VPX::Properties::StringPropertyDef*>(item->m_property.get());
            string v = item->GetStringValue();
            if (prop->m_pathType != VPX::Properties::StringPropertyDef::PathType::None)
            {
               const bool isFolder = prop->m_pathType == VPX::Properties::StringPropertyDef::PathType::Folder;
               const char* browseIcon = isFolder ? ICON_LC_FOLDER_OPEN : ICON_LC_FILE;
               const char* hint = isFolder ? "Select a folder..." : "Select a file...";
               const float browseButtonWidth = IconButtonInkSize(browseIcon).x;
               ImGui::SetNextItemWidth(rowCtlW - browseButtonWidth - style.ItemSpacing.x);
               ImGui::InputTextWithHint(std::format("##Item{}", i).c_str(), hint, &v);
               ImGui::SameLine();
               if (IconButtonInk(browseIcon, std::format("##Item{}", i).c_str()))
                  page.BrowseForPath(*item);
               if (ImGui::IsItemHovered())
                  ImGui::SetTooltip("%s", hint);
            }
            else
            {
               ImGui::SetNextItemWidth(rowCtlW);
               ImGui::InputText(std::format("##Item{}", i).c_str(), &v);
            }
            item->SetValue(v);
            break;
         }

         default: assert(false); break;
         }
         // Modified / overridden status icon in the row's right margin, with a
         // delayed tooltip explaining its meaning
         if (item->IsModified() || item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
         {
            const bool modified = item->IsModified();
            const char* icon = modified ? ICON_LC_PENCIL : ICON_LC_CIRCLE_DOT;
            const ImVec4 ink = TextInkBounds(icon);
            itemDrawList->AddText(ImVec2(rowMax.x - markerW + (markerW - (ink.z - ink.x)) * 0.5f - ink.x,
                                     rowPos.y + (stacked ? itemPadding + (textH - (ink.w - ink.y)) * 0.5f : (rowH - (ink.w - ink.y)) * 0.5f) - ink.y),
               ImGui::GetColorU32(ImGuiCol_Text), icon);
            ImGui::SetCursorScreenPos(ImVec2(rowMax.x - markerW, rowPos.y));
            ImGui::InvisibleButton(std::format("##marker{}", i).c_str(), ImVec2(markerW, rowH));
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
               ImGui::SetTooltip("%s", modified ? "Modified: the current value differs from the stored one" : "Table override: this setting is customized for this table");
         }
         break;

      default: assert(false); break;
      }

      if (navHovered)
         ImGui::PopStyleColor(7);

      // Thin separator between rows + hover bookkeeping
      itemDrawList->AddLine(ImVec2(rowPos.x, rowMax.y - 0.5f), ImVec2(rowMax.x, rowMax.y - 0.5f), separatorColor);
      if (navHovered)
      {
         if (rowStartY - ImGui::GetScrollY() < 0.f)
            ImGui::SetScrollY(rowStartY);
         else if (rowStartY + rowH - ImGui::GetScrollY() > ImGui::GetWindowHeight())
            // Bottom alignment is capped at the row top: a row taller than the
            // pane settles on its top edge instead of oscillating
            ImGui::SetScrollY(std::min(rowStartY + rowH - ImGui::GetWindowHeight(), rowStartY));
      }
      if ((item->m_type == Navigation || item->m_type == Runnable) && rowPressed)
      {
         page.SelectItem(i);
         page.AdjustItem(1.f, true);
      }
      ImGui::SetCursorScreenPos(rowPos + ImVec2(0.f, rowH));
   }

   // Embedded panes are drawn inline at the end of the item list
   for (const InGameUIPane& pane : panes)
      if (pane.placement == InGameUIPane::Placement::Embedded && pane.render)
         pane.render(ImVec4(0.f, 0.f, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
}

}
