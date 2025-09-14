// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("settings/graphic"s, "Graphic Settings"s, ""s)
{
   const Settings& settings = GetSettings();

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
