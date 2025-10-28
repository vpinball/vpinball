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
      VPX::Properties::BoolPropertyDef(""s, ""s, "Lock Volumes"s, "Adjust backlglass and playfield volume simultaneaously"s, true), //
      [this]() { return m_lockVolume; }, //
      [this]() { return m_lockVolume; }, //
      [this](bool v) { m_lockVolume = v; }, //
      [](Settings&) { /* UI state, not persisted */ },
      [](bool, const Settings&, bool) { /* UI state, not persisted */ }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BackglassVolume, "%3d %%"s, //
      [this]() { return m_player->m_MusicVolume; },
      [this](int prev, int v)
      {
         m_player->m_MusicVolume = v;
         if (m_lockVolume)
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlayfieldVolume, "%3d %%"s, //
      [this]() { return m_player->m_SoundVolume; },
      [this](int prev, int v)
      {
         m_player->m_SoundVolume = v;
         if (m_lockVolume)
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      }));

   // FIXME play music is not really dynamic at the moment => switch to a clean mute/unmute implementation
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlayMusic, //
      [this]() { return m_player->m_PlayMusic; }, //
      [this](bool v)
      {
         m_player->m_PlayMusic = v;
         m_player->UpdateVolume();
      }));
   // if (settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName))

   // FIXME play sound is not really dynamic at the moment => switch to a clean mute/unmute implementation
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlaySound, //
      [this]() { return m_player->m_PlaySound; }, //
      [this](bool v)
      {
         m_player->m_PlaySound = v;
         m_player->UpdateVolume();
      }));
   // if (settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName))
   // int fmusic = settings.LoadValueUInt(Settings::Player, "Sound3D"s);
}

}
