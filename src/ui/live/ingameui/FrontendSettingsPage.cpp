// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendSettingsPage.h"

#include "core/extern.h"
#include "core/FileLocator.h"
#include "core/SettingsService.h"
#include "core/VPApp.h"
#include "ui/live/LiveUI.h"

namespace VPX::InGameUI
{

namespace
{
   int FindThemeIndex(const vector<VPX::Frontend::FrontendTheme::Entry>& themes, const string& selected)
   {
      if (selected.empty())
         return 0;
      const auto it = std::ranges::find_if(themes, [&selected](const VPX::Frontend::FrontendTheme::Entry& theme) { return theme.name == selected; });
      return it == themes.end() ? 0 : static_cast<int>(std::distance(themes.begin(), it));
   }
}

FrontendSettingsPage::FrontendSettingsPage()
   : InGameUIPage("Frontend Settings"s, ""s, SaveMode::Global)
{
}

void FrontendSettingsPage::RefreshThemes()
{
   vector<std::filesystem::path> roots { g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "frontend"s };
   const string& setting = g_settingsService.GetAppSettings().GetFrontend_ThemeFolders();
   size_t start = 0;
   while (start <= setting.size())
   {
      const size_t end = setting.find(';', start);
      const string folder = setting.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
         roots.emplace_back(folder);
      if (end == string::npos)
         break;
      start = end + 1;
   }
   m_themes = VPX::Frontend::FrontendTheme::Discover(roots);
}

void FrontendSettingsPage::BuildPage()
{
   m_builtThemeFolders = g_settingsService.GetActiveSettings().GetFrontend_ThemeFolders();
   RefreshThemes();

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Table folders"s));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::StringPropertyDef("Frontend"s, "TableFolders"s, "Table Folders"s, "Semicolon separated list of root folders which are recursively scanned for tables"s, false, ""s,
         VPX::Properties::StringPropertyDef::PathType::Folder),
      [this]() { return g_settingsService.GetActiveSettings().GetFrontend_TableFolders(); },
      [](const Settings& settings) { return settings.GetString(Settings::m_propFrontend_TableFolders); }, [this](const string&, const string& v)
      { g_settingsService.GetActiveSettings().SetFrontend_TableFolders(v, false); }, [](Settings& settings) { settings.Reset(Settings::m_propFrontend_TableFolders); },
      [](const string& v, Settings& settings, bool isTableOverride) { settings.Set(Settings::m_propFrontend_TableFolders, v, isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Theme"s));
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Themes bundled with the application (in its 'assets/frontend' folder) are always available."s));
   if (m_themes.empty())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No usable theme was found in the configured theme folders."s));
   }
   else
   {
      vector<string> themeNames;
      themeNames.reserve(m_themes.size());
      for (const auto& theme : m_themes)
         themeNames.push_back(theme.name);
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Theme"s, "Theme used to lay out and decorate the frontend."s, false, 0, 0, themeNames), //
         [this]() { return FindThemeIndex(m_themes, g_settingsService.GetActiveSettings().GetFrontend_Theme()); }, //
         [this](const Settings& settings) { return FindThemeIndex(m_themes, settings.GetFrontend_Theme()); }, //
         [this](int, int v) { g_settingsService.GetActiveSettings().SetFrontend_Theme(m_themes[v].name, false); }, //
         [](Settings& settings) { settings.ResetFrontend_Theme(); }, //
         [this](int v, Settings& settings, bool isTableOverride) { settings.SetFrontend_Theme(m_themes[v].name, isTableOverride); }));
   }
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::StringPropertyDef("Frontend"s, "ThemeFolders"s, "Theme Folders"s, "Semicolon separated list of root folders which are scanned for frontend themes"s, false, ""s,
         VPX::Properties::StringPropertyDef::PathType::Folder),
      [this]() { return g_settingsService.GetActiveSettings().GetFrontend_ThemeFolders(); },
      [](const Settings& settings) { return settings.GetString(Settings::m_propFrontend_ThemeFolders); }, [this](const string&, const string& v)
      { g_settingsService.GetActiveSettings().SetFrontend_ThemeFolders(v, false); }, [](Settings& settings) { settings.Reset(Settings::m_propFrontend_ThemeFolders); },
      [](const string& v, Settings& settings, bool isTableOverride) { settings.Set(Settings::m_propFrontend_ThemeFolders, v, isTableOverride); }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Cache management"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propFrontend_CacheDiscardStale, //
      [this]() { return g_settingsService.GetActiveSettings().GetFrontend_CacheDiscardStale(); }, //
      [this](bool v) { g_settingsService.GetActiveSettings().SetFrontend_CacheDiscardStale(v, false); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propFrontend_TextureCacheLimit, //
      [this]() { return g_settingsService.GetActiveSettings().GetFrontend_TextureCacheLimit(); }, //
      [this](int, int v) { g_settingsService.GetActiveSettings().SetFrontend_TextureCacheLimit(v, false); }));
}

void FrontendSettingsPage::Update(float elapsedS)
{
   InGameUIPage::Update(elapsedS);
   if (!ImGui::IsAnyItemActive() && g_settingsService.GetActiveSettings().GetFrontend_ThemeFolders() != m_builtThemeFolders)
      RequestRebuild();
}

void FrontendSettingsPage::SaveGlobally()
{
   InGameUIPage::SaveGlobally();
   m_player->m_liveUI->m_frontendUI.Rescan();
}

}
