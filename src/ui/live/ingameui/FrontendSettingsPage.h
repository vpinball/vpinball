// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "ui/live/frontend/FrontendTheme.h"

namespace VPX::InGameUI
{

class FrontendSettingsPage final : public InGameUIPage
{
public:
   FrontendSettingsPage();

   void SaveGlobally() override;
   void Update(float elapsedS) override;

private:
   void BuildPage() override;
   void RefreshThemes();

   vector<VPX::Frontend::FrontendTheme::Entry> m_themes;
   string m_builtThemeFolders;
};

}
