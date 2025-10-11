// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("Graphic Settings"s, ""s, SaveMode::Both)
{
   const Settings& settings = GetSettings();

   auto postprocAA = std::make_unique<InGameUIItem>(
      "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s,
      vector({ "Disabled"s, "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s }), static_cast<int>(m_player->m_renderer->m_FXAA),
      [this]() { return static_cast<int>(m_player->m_renderer->m_FXAA); },
      [this](int prev, int v) { m_player->m_renderer->m_FXAA = v; },
      InGameUIItem::ResetSetting(Settings::Player, "FXAA"s),
      InGameUIItem::SaveSettingInt(Settings::Player, "FXAA"s));
   AddItem(postprocAA);

   auto postprocSharpen = std::make_unique<InGameUIItem>(
      "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, vector({ "Disabled"s, "CAS"s, "Bilateral CAS"s }),
      static_cast<int>(m_player->m_renderer->m_sharpen),
      [this]() { return static_cast<int>(m_player->m_renderer->m_sharpen); },
      [this](int prev, int v) { m_player->m_renderer->m_sharpen = v; },
      InGameUIItem::ResetSetting(Settings::Player, "Sharpen"s),
      InGameUIItem::SaveSettingInt(Settings::Player, "Sharpen"s));
   AddItem(postprocSharpen);

   auto disableTonemappingOnHDRDisplay = std::make_unique<InGameUIItem>(
      "Disable tonemapping on HDR display"s, "Do not perform tonemapping when rendering on a HDR display"s,
      settings.LoadValueBool(Settings::Player, "HDRDisableToneMapper"s),
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; },
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; },
      InGameUIItem::ResetSetting(Settings::Player, "HDRDisableToneMapper"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "HDRDisableToneMapper"s));
   AddItem(disableTonemappingOnHDRDisplay);

   auto disableBloom = std::make_unique<InGameUIItem>(
      "Disable Bloom"s, "Disable postprocessed bloom filter"s,
      settings.LoadValueBool(Settings::Player, "ForceBloomOff"s),
      [this]() { return m_player->m_renderer->m_bloomOff; },
      [this](bool v) { m_player->m_renderer->m_bloomOff = v; },
      InGameUIItem::ResetSetting(Settings::Player, "ForceBloomOff"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "ForceBloomOff"s));
   AddItem(disableBloom);

   auto disableMotionBlur = std::make_unique<InGameUIItem>(
      "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s,
      settings.LoadValueBool(Settings::Player, "ForceMotionBlurOff"s),
      [this]() { return m_player->m_renderer->m_motionBlurOff; },
      [this](bool v) { m_player->m_renderer->m_motionBlurOff = v; },
      InGameUIItem::ResetSetting(Settings::Player, "ForceMotionBlurOff"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "ForceMotionBlurOff"s));
   AddItem(disableMotionBlur);
}

}
