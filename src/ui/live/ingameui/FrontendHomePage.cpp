// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendHomePage.h"

#include "renderer/Renderer.h"


namespace VPX::InGameUI
{

FrontendHomePage::FrontendHomePage()
   // No title: the home page is only the navigation source (its items feed the top
   // tabs and the rail), the content pane stays blank until a page is selected
   : InGameUIPage(""s, ""s, SaveMode::None)
{
}

void FrontendHomePage::BuildPage()
{
   AddItem(std::make_unique<InGameUIItem>("Plugin Settings"s, ""s, "plugins/homepage"s));

   AddItem(std::make_unique<InGameUIItem>("Sound Settings"s, ""s, "settings/audio"s));

   AddItem(std::make_unique<InGameUIItem>("Graphics Settings"s, ""s, "settings/graphics"s));

#ifndef ENABLE_DX9
   if (m_player->m_renderer->m_stereo3D != STEREO_VR)
      AddItem(std::make_unique<InGameUIItem>("Stereo Settings"s, ""s, "settings/stereo"s));
#endif

   AddItem(std::make_unique<InGameUIItem>("Display Settings"s, ""s, "settings/displays"s));

   AddItem(std::make_unique<InGameUIItem>("Input Settings"s, ""s, "settings/input"s));

   AddItem(std::make_unique<InGameUIItem>("Plunger Settings"s, ""s, "settings/plunger"s));

   AddItem(std::make_unique<InGameUIItem>("Nudge & Tilt Settings"s, ""s, "settings/nudge"s));

   AddItem(std::make_unique<InGameUIItem>("Cabinet Settings"s, ""s, "settings/cabinet"s));

   AddItem(std::make_unique<InGameUIItem>("Alpha/DMD Profile Settings"s, ""s, "settings/display_profiles"s));

   AddItem(std::make_unique<InGameUIItem>("Legacy DOF & B2S Settings"s, ""s, "settings/misc"s));

   AddItem(std::make_unique<InGameUIItem>("Logging Settings"s, ""s, "settings/logging"s));

   AddItem(std::make_unique<InGameUIItem>("Frontend Themes"s, ""s, "settings/themes"s));

   AddItem(std::make_unique<InGameUIItem>("System Info"s, ""s, "misc/systeminfo"s));
}

}
