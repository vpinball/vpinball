// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendDesktopRenderer.h"

#include "core/player.h"
#include "ui/live/frontend/FrontendUI.h"
#include "ui/live/ingameui/InGameUI.h"
#include "ui/live/LiveUI.h"


namespace VPX::Frontend
{

void FrontendDesktopRenderer::BuildTopTabs(vector<InGameUI::DesktopTopTab> &tabs, const string &rootPath, const std::function<string(const string &)> &firstPathOf)
{
   // GAMES stands in for GAME: it closes the UI back to the table catalog (the
   // frontend's base view) as GAME closes it back to play
   tabs.push_back({ "GAMES", "games", false, 1, 1.f, [this]() { m_player->m_liveUI->HideUI(); } });
   const string tablePath = firstPathOf("table");
   if (!tablePath.empty())
      tabs.push_back({ "TABLE", "table", false, 0, 1.f, [this, tablePath]()
         {
            m_selectedGroup = "table";
            m_inGameUI.NavigateTo(tablePath);
         } });
   const string settingsPath = firstPathOf("settings");
   if (!settingsPath.empty())
      tabs.push_back({ "SETTINGS", "settings", false, 0, 1.f, [this, settingsPath]()
         {
            m_selectedGroup = "settings";
            m_inGameUI.NavigateTo(settingsPath);
         } });
   const string pluginsPath = firstPathOf("plugins");
   if (!pluginsPath.empty())
      tabs.push_back({ "PLUGINS", "plugins", false, 0, 1.f, [this, pluginsPath]()
         {
            m_selectedGroup = "plugins";
            m_inGameUI.NavigateTo(pluginsPath);
         } });
   const string aboutPath = firstPathOf("misc");
   if (!aboutPath.empty())
      tabs.push_back({ "ABOUT", "misc", true, 1, 0.85f, [this, aboutPath]()
         {
            m_selectedGroup = "misc";
            m_inGameUI.NavigateTo(aboutPath);
         } });
   tabs.push_back({ "QUIT", "quit", true, 1, 0.85f, [this]()
      {
         m_player->m_liveUI->HideUI();
         m_player->SetCloseState(Player::CS_CLOSE_APP);
      } });
}

string FrontendDesktopRenderer::NavGroup(const string &path) const
{
   // The frontend's table actions page is its TABLE section: it is registered
   // under the frontend path (the "table" group is the play session's own pages)
   return path == "frontend/table"s ? "table"s : InGameUIDesktopRenderer::NavGroup(path);
}

string FrontendDesktopRenderer::NavRootGroup(const string &rootPath) const
{
   // The frontend's navigation root is its own category: there is no table the
   // way a play session has one, and GAMES is a base view action, not a category
   return NavGroup(rootPath);
}

const char *FrontendDesktopRenderer::NavRootRailLabel(const string &group) const
{
   return nullptr; // No "Table Info" root row: the frontend's root page is a pure table of contents
}

const char *FrontendDesktopRenderer::NavRootPageTitle() const
{
   return nullptr; // The root page keeps its own title
}

}
