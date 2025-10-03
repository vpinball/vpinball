// license:GPLv3+

#include "core/stdafx.h"

#include "HomePage.h"

namespace VPX::InGameUI
{

HomePage::HomePage()
   : InGameUIPage("homepage"s, "Visual Pinball X"s, ""s, SaveMode::None)
{
}

void HomePage::Open()
{
   ClearItems();

   if (!m_player->m_ptable->m_rules.empty())
   {
      auto tableRules = std::make_unique<InGameUIItem>("Table Rules"s, ""s, "table/rules"s);
      AddItem(tableRules);
   }

   if (!m_player->m_ptable->m_settings.GetTableSettings().empty())
   {
      auto tableOptions = std::make_unique<InGameUIItem>("Table Options"s, ""s, "table/options"s);
      AddItem(tableOptions);
   }

   if (m_player->m_vrDevice)
   {
      #ifdef ENABLE_XR
         // Legacy OpenVR does not support dynamic repositioning through LiveUI (especially overall scale, this would need to be rewritten but not done as this is planned for deprecation)
         auto vrSettings = std::make_unique<InGameUIItem>("VR Settings"s, ""s, "settings/vr"s);
         AddItem(vrSettings);
      #endif
   }
   else
   {
      auto povSettings = std::make_unique<InGameUIItem>("Point Of View"s, ""s, "settings/pov"s);
      AddItem(povSettings);
   }

   auto audioSettings = std::make_unique<InGameUIItem>("Sound Settings"s, ""s, "settings/audio"s);
   AddItem(audioSettings);

   auto miscSettings = std::make_unique<InGameUIItem>("Miscellaneous Settings"s, ""s, "settings/misc"s);
   AddItem(miscSettings);

   auto graphicSettings = std::make_unique<InGameUIItem>("Graphic Settings"s, ""s, "settings/graphic"s);
   AddItem(graphicSettings);

   /*auto displaySettings = std::make_unique<InGameUIItem>("Display Settings"s, ""s, "settings/display"s);
   AddItem(displaySettings);*/

   auto inputSettings = std::make_unique<InGameUIItem>("Input Settings"s, ""s, "settings/input"s);
   AddItem(inputSettings);

   auto plungerSettings = std::make_unique<InGameUIItem>("Plunger Settings"s, ""s, "settings/plunger"s);
   AddItem(plungerSettings);

   auto nudgeSettings = std::make_unique<InGameUIItem>("Nudge & Tilt Settings"s, ""s, "settings/nudge"s);
   AddItem(nudgeSettings);

   // TODO implement Plugin option system to new UI (note that, for the time being, plugin setting system is just an unused draft so far, see GetOption in VPXPlugin.h)

   InGameUIPage::Open();
}

}
