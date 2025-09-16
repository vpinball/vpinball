// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("settings/graphic"s, "Graphic Settings"s, ""s)
{
   const Settings& settings = GetSettings();

   auto postprocAA = std::make_unique<InGameUIItem>(
      "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s,
      vector({ "Disabled"s, "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s }), static_cast<int>(m_player->m_renderer->m_FXAA),
      [this]() { return static_cast<int>(m_player->m_renderer->m_FXAA); }, [this](int prev, int v) { m_player->m_renderer->m_FXAA = v; },
      [](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "FXAA"s, v, isTableOverride); });
   AddItem(postprocAA);

   auto postprocSharpen = std::make_unique<InGameUIItem>(
      "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, vector({ "Disabled"s, "CAS"s, "Bilateral CAS"s }),
      static_cast<int>(m_player->m_renderer->m_sharpen), [this]() { return static_cast<int>(m_player->m_renderer->m_sharpen); }, [this](int prev, int v)
      { m_player->m_renderer->m_sharpen = v; }, [](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "Sharpen"s, v, isTableOverride); });
   AddItem(postprocSharpen);

   auto disableTonemappingOnHDRDisplay = std::make_unique<InGameUIItem>(
      "Disable tonemapping on HDR display"s, "Do not perform tonemapping when rendering on a HDR display"s, settings.LoadValueBool(Settings::Player, "HDRDisableToneMapper"s),
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; },
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; },
      [](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "HDRDisableToneMapper"s, v, isTableOverride); });
   AddItem(disableTonemappingOnHDRDisplay);

   auto disableBloom = std::make_unique<InGameUIItem>(
      "Disable Bloom"s, "Disable postprocessed bloom filter"s, settings.LoadValueBool(Settings::Player, "ForceBloomOff"s), [this]() { return m_player->m_renderer->m_bloomOff; },
      [this](bool v) { m_player->m_renderer->m_bloomOff = v; },
      [](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "ForceBloomOff"s, v, isTableOverride); });
   AddItem(disableBloom);

   auto disableMotionBlur = std::make_unique<InGameUIItem>(
      "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s, settings.LoadValueBool(Settings::Player, "ForceMotionBlurOff"s),
      [this]() { return m_player->m_renderer->m_motionBlurOff; }, [this](bool v) { m_player->m_renderer->m_motionBlurOff = v; },
      [](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "ForceMotionBlurOff"s, v, isTableOverride); });
   AddItem(disableMotionBlur);
}

}
