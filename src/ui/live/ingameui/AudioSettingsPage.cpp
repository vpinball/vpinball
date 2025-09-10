// license:GPLv3+

#include "core/stdafx.h"

#include "AudioSettingsPage.h"

namespace VPX::InGameUI
{

AudioSettingsPage::AudioSettingsPage()
   : InGameUIPage("settings/audio"s, "Audio Settings"s, ""s)
{
   const Settings& settings = GetSettings();

   // Volume can either be adjusted globally or individually between backglass/playfield.
   auto lockVolume = std::make_unique<InGameUIItem>(
      "Lock Volumes"s, "Adjust backlglass and playfield volume simultaneaously"s, true,
      [this]() { return m_lockVolume; },
      [this](bool v) { m_lockVolume = v; },
      [](bool, const Settings&, bool) { /* UI state is not persisted */ });
   AddItem(lockVolume);

   auto backglassVolume = std::make_unique<InGameUIItem>(
      "Backglass Volume"s, "Main volume for music and sound played in the backglass speakers"s, 0, 100, m_player->m_ptable->m_settings.LoadValueInt(Settings::Player, "MusicVolume"s),
      "%3d %%"s, [this]() { return m_player->m_MusicVolume; },
      [this](int prev, int v)
      {
         m_player->m_MusicVolume = v;
         if (m_lockVolume)
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      },
      [](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "MusicVolume"s, v, isTableOverride); });
   AddItem(backglassVolume);

   auto playfieldVolume = std::make_unique<InGameUIItem>(
      "Playfield Volume"s, "Main volume for mechanical sounds coming from the playfield"s, 0, 100, m_player->m_ptable->m_settings.LoadValueInt(Settings::Player, "SoundVolume"s), "%3d %%"s,
      [this]() { return m_player->m_SoundVolume; },
      [this](int prev, int v)
      {
         m_player->m_SoundVolume = v;
         if (m_lockVolume)
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      },
      [](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "SoundVolume"s, v, isTableOverride); });
   AddItem(playfieldVolume);

   // FIXME play music is not really dynamic at the moment => switch to a clean mute/unmute implementation
   auto enableBackglass = std::make_unique<InGameUIItem>(
      "Enable Backglass"s, "Enable/Disable backglass game sound & music"s, settings.LoadValueBool(Settings::Player, "PlayMusic"s), [this]() { return m_player->m_PlayMusic; },
      [this](bool v)
      {
         m_player->m_PlayMusic = v;
         m_player->UpdateVolume();
      },
      [](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "PlayMusic"s, v, isTableOverride); });
   AddItem(enableBackglass);
   // if (settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName))

   // FIXME play sound is not really dynamic at the moment => switch to a clean mute/unmute implementation
   auto enablePlayfield = std::make_unique<InGameUIItem>(
      "Enable Playfield"s, "Enable/Disable playfield mechanical sounds"s, settings.LoadValueBool(Settings::Player, "PlaySound"s), [this]() { return m_player->m_PlaySound; },
      [this](bool v)
      {
         m_player->m_PlaySound = v;
         m_player->UpdateVolume();
      },
      [](bool v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "PlaySound"s, v, isTableOverride); });
   AddItem(enablePlayfield);
   // if (settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName))
   // int fmusic = settings.LoadValueUInt(Settings::Player, "Sound3D"s);
}

}
