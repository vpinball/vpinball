// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendThemesPage.h"

#include "core/extern.h"
#include "core/FileLocator.h"
#include "core/SettingsService.h"
#include "core/VPApp.h"
#include "ui/live/IconsLucide.h"


namespace VPX::InGameUI
{

FrontendThemesPage::FrontendThemesPage()
   : InGameUIPage("Frontend Themes"s, ""s, SaveMode::None)
{
}

void FrontendThemesPage::RefreshThemes()
{
   vector<std::filesystem::path> roots { g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "frontend"s };
   m_folders.clear();
   const string &setting = g_settingsService.GetAppSettings().GetFrontend_ThemeFolders();
   size_t start = 0;
   while (start <= setting.size())
   {
      const size_t end = setting.find(';', start);
      const string folder = setting.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
      {
         m_folders.push_back(folder);
         roots.emplace_back(folder);
      }
      if (end == string::npos)
         break;
      start = end + 1;
   }
   m_themes = VPX::Frontend::FrontendTheme::Discover(roots);
}

void FrontendThemesPage::SaveFolders()
{
   string folders;
   for (const string &f : m_folders)
      folders += f + ';';
   if (!folders.empty())
      folders.pop_back();
   g_settingsService.GetAppSettings().SetFrontend_ThemeFolders(folders, false);
   g_settingsService.GetAppSettings().Save();
   RefreshThemes();
}

void FrontendThemesPage::BuildPage()
{
   RefreshThemes();
}

void FrontendThemesPage::CollectPanes(vector<InGameUIPane>& panes)
{
   InGameUIPage::CollectPanes(panes);

   InGameUIPane& pane = panes.emplace_back();
   pane.placement = InGameUIPane::Placement::Embedded;
   pane.id = "themeEditor"s;
   pane.render = [this](const ImVec4&) { RenderThemeEditor(); };
}

void FrontendThemesPage::RenderThemeEditor()
{
   ImGui::SeparatorText("Theme");
   const string &selected = g_settingsService.GetAppSettings().GetFrontend_Theme();
   const string current = selected.empty() && !m_themes.empty() ? m_themes.front().name : selected;
   ImGui::SetNextItemWidth(-1.f);
   if (ImGui::BeginCombo("##theme", current.empty() ? "None found" : current.c_str()))
   {
      for (const auto &theme : m_themes)
         if (ImGui::Selectable(theme.name.c_str(), theme.name == current))
         {
            g_settingsService.GetAppSettings().SetFrontend_Theme(theme.name, false);
            g_settingsService.GetAppSettings().Save();
         }
      ImGui::EndCombo();
   }

   ImGui::SeparatorText("Theme folders");
   ImGui::TextWrapped("Themes bundled with the application (in its 'assets/frontend' folder) are always available.");
   int removeIndex = -1;
   for (int i = 0; i < static_cast<int>(m_folders.size()); i++)
   {
      ImGui::PushID(i);
      if (ImGui::SmallButton(ICON_LC_X))
         removeIndex = i;
      ImGui::SameLine();
      ImGui::TextUnformatted(m_folders[i].c_str());
      ImGui::PopID();
   }
   if (removeIndex >= 0)
   {
      m_folders.erase(m_folders.begin() + removeIndex);
      SaveFolders();
   }
   ImGui::SetNextItemWidth(-80.f);
   ImGui::InputTextWithHint("##folder", "Folder path...", m_folderInput, sizeof(m_folderInput));
   ImGui::SameLine();
   if (ImGui::Button("Add") && m_folderInput[0] != '\0')
   {
      m_folders.emplace_back(m_folderInput);
      m_folderInput[0] = '\0';
      SaveFolders();
   }
}

}
