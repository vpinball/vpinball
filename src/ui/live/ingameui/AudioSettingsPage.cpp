// license:GPLv3+

#include "core/stdafx.h"

#include "AudioSettingsPage.h"

namespace VPX::InGameUI
{

AudioSettingsPage::AudioSettingsPage()
   : InGameUIPage("Audio Settings"s, ""s, SaveMode::Both)
{
   const Settings& settings = GetSettings();

   // Volume can either be adjusted globally or individually between backglass/playfield.
   AddItem(std::make_unique<InGameUIItem>(
      "Lock Volumes"s, "Adjust backlglass and playfield volume simultaneaously"s, true,
      [this]() { return m_lockVolume; },
      [this](bool v) { m_lockVolume = v; },
      [](Settings&) { /* UI state is not persisted */ },
      [](bool, const Settings&, bool) { /* UI state is not persisted */ }));

   AddItem(std::make_unique<InGameUIItem>(
      "Backglass Volume"s, "Main volume for music and sound played in the backglass speakers"s, 0, 100, 
      m_player->m_ptable->m_settings.LoadValueInt(Settings::Player, "MusicVolume"s),
      "%3d %%"s, [this]() { return m_player->m_MusicVolume; },
      [this](int prev, int v)
      {
         m_player->m_MusicVolume = v;
         if (m_lockVolume)
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      },
      InGameUIItem::ResetSetting(Settings::Player, "MusicVolume"s),
      InGameUIItem::SaveSettingInt(Settings::Player, "MusicVolume"s)));

   AddItem(std::make_unique<InGameUIItem>(
      "Playfield Volume"s, "Main volume for mechanical sounds coming from the playfield"s, 0, 100,
      settings.LoadValueInt(Settings::Player, "SoundVolume"s), "%3d %%"s,
      [this]() { return m_player->m_SoundVolume; },
      [this](int prev, int v)
      {
         m_player->m_SoundVolume = v;
         if (m_lockVolume)
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      },
      InGameUIItem::ResetSetting(Settings::Player, "SoundVolume"s),
      InGameUIItem::SaveSettingInt(Settings::Player, "SoundVolume"s)));

   // FIXME play music is not really dynamic at the moment => switch to a clean mute/unmute implementation
   AddItem(std::make_unique<InGameUIItem>(
      "Enable Backglass"s, "Enable/Disable backglass game sound & music"s,
      settings.LoadValueBool(Settings::Player, "PlayMusic"s),
      [this]() { return m_player->m_PlayMusic; },
      [this](bool v)
      {
         m_player->m_PlayMusic = v;
         m_player->UpdateVolume();
      },
      InGameUIItem::ResetSetting(Settings::Player, "PlayMusic"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "PlayMusic"s)));
   // if (settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName))

   // FIXME play sound is not really dynamic at the moment => switch to a clean mute/unmute implementation
   AddItem(std::make_unique<InGameUIItem>(
      "Enable Playfield"s, "Enable/Disable playfield mechanical sounds"s,
      settings.LoadValueBool(Settings::Player, "PlaySound"s),
      [this]() { return m_player->m_PlaySound; },
      [this](bool v)
      {
         m_player->m_PlaySound = v;
         m_player->UpdateVolume();
      },
      InGameUIItem::ResetSetting(Settings::Player, "PlaySound"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "PlaySound"s)));
   // if (settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName))
   // int fmusic = settings.LoadValueUInt(Settings::Player, "Sound3D"s);
}

}
