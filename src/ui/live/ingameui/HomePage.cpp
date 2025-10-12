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

   if (!m_player->m_ptable->m_settings.GetTableSettings().empty())
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

   AddItem(std::make_unique<InGameUIItem>("Miscellaneous Settings"s, ""s, "settings/misc"s));

   AddItem(std::make_unique<InGameUIItem>("Graphic Settings"s, ""s, "settings/graphic"s));

   // AddItem(std::make_unique<InGameUIItem>("Display Settings"s, ""s, "settings/display"s));

   AddItem(std::make_unique<InGameUIItem>("Input Settings"s, ""s, "settings/input"s));

   AddItem(std::make_unique<InGameUIItem>("Plunger Settings"s, ""s, "settings/plunger"s));

   AddItem(std::make_unique<InGameUIItem>("Nudge & Tilt Settings"s, ""s, "settings/nudge"s));

   // TODO implement Plugin option system to new UI (note that, for the time being, plugin setting system is just an unused draft so far, see GetOption in VPXPlugin.h)
}

}
