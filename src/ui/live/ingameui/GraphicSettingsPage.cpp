// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("Graphic Settings"s, ""s, SaveMode::Both)
{
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_FXAA, //
      [this]() { return m_player->m_renderer->m_FXAA; }, //
      [this](int, int v) { m_player->m_renderer->m_FXAA = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Sharpen, //
      [this]() { return m_player->m_renderer->m_sharpen; }, //
      [this](int, int v) { m_player->m_renderer->m_sharpen = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRDisableToneMapper, //
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; }, //
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ForceBloomOff, //
      [this]() { return m_player->m_renderer->m_bloomOff; }, //
      [this](bool v) { m_player->m_renderer->m_bloomOff = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ForceMotionBlurOff, //
      [this]() { return m_player->m_renderer->m_motionBlurOff; }, //
      [this](bool v) { m_player->m_renderer->m_motionBlurOff = v; }));
}

}
