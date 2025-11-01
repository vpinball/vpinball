// license:GPLv3+

#include "core/stdafx.h"

#include "HomePage.h"

namespace VPX::InGameUI
{

HomePage::HomePage()
   : InGameUIPage("Settings & Options"s, ""s, SaveMode::None)
{
   if (!m_player->m_ptable->m_rules.empty())
      AddItem(std::make_unique<InGameUIItem>("Table Rules"s, ""s, "table/rules"s));

   if (!m_player->m_ptable->GetOptions().empty())
      AddItem(std::make_unique<InGameUIItem>("Table Options"s, ""s, "table/options"s));

   if (m_player->m_vrDevice)
   {
      #ifdef ENABLE_XR
         // Legacy OpenVR does not support dynamic repositioning through LiveUI (especially overall scale, this would need to be rewritten but not done as this is planned for deprecation)
         AddItem(std::make_unique<InGameUIItem>("VR Settings"s, ""s, "settings/vr"s));
      #endif
   }
   else
      AddItem(std::make_unique<InGameUIItem>("Point Of View"s, ""s, "settings/pov"s));

   AddItem(std::make_unique<InGameUIItem>("Sound Settings"s, ""s, "settings/audio"s));

   AddItem(std::make_unique<InGameUIItem>("Graphic Settings"s, ""s, "settings/graphic"s));

   #ifndef ENABLE_DX9
   if (m_player->m_renderer->m_stereo3D != STEREO_VR)
      AddItem(std::make_unique<InGameUIItem>("Stereo Settings"s, ""s, "settings/stereo"s));
   #endif

   if (m_player->m_backglassOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
      AddItem(std::make_unique<InGameUIItem>("Backglass"s, ""s, "settings/backglass"s));

   if (m_player->m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
      AddItem(std::make_unique<InGameUIItem>("Score View"s, ""s, "settings/scoreview"s));

   AddItem(std::make_unique<InGameUIItem>("Input Settings"s, ""s, "settings/input"s));

   AddItem(std::make_unique<InGameUIItem>("Plunger Settings"s, ""s, "settings/plunger"s));

   AddItem(std::make_unique<InGameUIItem>("Nudge & Tilt Settings"s, ""s, "settings/nudge"s));

   AddItem(std::make_unique<InGameUIItem>("Cabinet Settings"s, ""s, "settings/cabinet"s));

   AddItem(std::make_unique<InGameUIItem>("Alpha/DMD Profile Settings"s, ""s, "settings/display_profiles"s));

   AddItem(std::make_unique<InGameUIItem>("Miscellaneous Settings"s, ""s, "settings/misc"s));

   AddItem(std::make_unique<InGameUIItem>("Plugin Settings"s, ""s, "plugins/homepage"s));
}

}
