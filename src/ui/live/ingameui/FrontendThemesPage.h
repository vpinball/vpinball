// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "ui/live/frontend/FrontendTheme.h"

namespace VPX::InGameUI
{

// Frontend theme management: select the active theme among the ones found in the theme folders,
// and manage the folders themselves (roots scanned for theme subfolders, on top of the bundled
// application themes).
class FrontendThemesPage final : public InGameUIPage
{
public:
   FrontendThemesPage();

private:
   void BuildPage() override;
   void CollectPanes(vector<InGameUIPane>& panes) override;
   void RenderThemeEditor();
   void RefreshThemes();
   void SaveFolders();

   vector<VPX::Frontend::FrontendTheme::Entry> m_themes;
   vector<string> m_folders;
   char m_folderInput[512] = { };
};

}
