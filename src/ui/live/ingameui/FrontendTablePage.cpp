// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendTablePage.h"

#include "core/player.h"
#include "ui/live/frontend/FrontendUI.h"
#include "ui/live/LiveUI.h"


namespace VPX::InGameUI
{

static string PageTitle()
{
   // The page is built on each navigation: pick up the entry the wheel is on
   const TableCatalog::Entry *const selected = g_pplayer->m_liveUI->m_frontendUI.GetSelected();
   return selected != nullptr ? selected->title : "Table Actions"s;
}

FrontendTablePage::FrontendTablePage()
   : InGameUIPage(PageTitle(), ""s, SaveMode::None)
{
}

void FrontendTablePage::BuildPage()
{
   VPX::Frontend::FrontendUI &frontend = m_player->m_liveUI->m_frontendUI;
   if (frontend.HasSelection())
   {
      AddItem(std::make_unique<InGameUIItem>("Play"s, ""s, [&frontend]() { frontend.LaunchSession(Player::PlayMode::Play); }));
      AddItem(std::make_unique<InGameUIItem>("Edit in live editor"s, ""s, [&frontend]() { frontend.LaunchSession(Player::PlayMode::FullEdit); }));
   }
   AddItem(std::make_unique<InGameUIItem>("Rescan tables"s, ""s,
      [this, &frontend]()
      {
         m_player->m_liveUI->HideUI();
         frontend.Rescan();
      }));
#ifdef ENABLE_XR
   if (m_player->IsVR())
      AddItem(std::make_unique<InGameUIItem>("Exit VR"s, ""s, [&frontend]() { frontend.RestartFrontendSession(Player::VROverride::ForceOff); }));
   else if (frontend.IsVRAvailable())
      AddItem(std::make_unique<InGameUIItem>("Enter VR"s, ""s, [&frontend]() { frontend.RestartFrontendSession(Player::VROverride::ForceOn); }));
#endif
}

}
