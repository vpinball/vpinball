// license:GPLv3+

#include "core/stdafx.h"
#include "TouchDevicePage.h"

#include "core/player.h"
#include "ui/live/LiveUI.h"

namespace VPX::InGameUI
{

TouchDevicePage::TouchDevicePage()
   : InGameUIPage("Touch"s, ""s, SaveMode::Global)
{
}

void TouchDevicePage::BuildPage()
{
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_TouchOverlay, //
      [this]() { return m_player->m_liveUI->IsTouchOverlayEnabled(); }, //
      [this](bool showTouchOverlay) { m_player->m_liveUI->ShowTouchOverlay(showTouchOverlay); }));
}

}
