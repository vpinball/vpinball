// license:GPLv3+

#include "core/stdafx.h"

#include "AudioSettingsPage.h"

namespace VPX::InGameUI
{

AudioSettingsPage::AudioSettingsPage()
   : InGameUIPage("Audio Settings"s, ""s, SaveMode::Both)
{
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Lock Volumes"s, "Adjust backglass and playfield volume simultaneaously"s, false, true), //
      [this]() { return m_lockVolume; }, //
      [this](Settings&) { return m_lockVolume; }, //
      [this](bool v) { m_lockVolume = v; }, //
      [](Settings&) { /* UI state, not persisted */ }, //
      [](bool, Settings&, bool) { /* UI state, not persisted */ }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MusicVolume, "%3d %%"s, //
      [this]() { return m_player->m_MusicVolume; },
      [this](int prev, int v)
      {
         m_player->m_MusicVolume = v;
         if (m_lockVolume)
            m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SoundVolume, "%3d %%"s, //
      [this]() { return m_player->m_SoundVolume; },
      [this](int prev, int v)
      {
         m_player->m_SoundVolume = v;
         if (m_lockVolume)
            m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + v - prev, 0, 100);
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlayMusic, //
      [this]() { return m_player->m_PlayMusic; }, //
      [this](bool v)
      {
         m_player->m_PlayMusic = v;
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlaySound, //
      [this]() { return m_player->m_PlaySound; }, //
      [this](bool v)
      {
         m_player->m_PlaySound = v;
         m_player->UpdateVolume();
      }));

   for (const auto& device : VPX::AudioPlayer::EnumerateAudioDevices())
      m_devices.push_back(device.name);

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Backglass Sound Device"s, "Select backglass sound device"s, false, 0, 0, m_devices), //
      [this]()
      {
         auto it = std::ranges::find(m_devices, m_player->m_audioPlayer->GetBackglassDeviceName());
         return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
      }, // Live
      [this](Settings& settings)
      {
         auto it = std::ranges::find(m_devices, settings.GetPlayer_SoundDeviceBG());
         return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
      }, // Stored
      [this](int, int v) {
         m_player->m_audioPlayer = std::make_unique<VPX::AudioPlayer>( //
            m_devices[v], //
            m_player->m_ptable->m_settings.GetPlayer_SoundDevice(), //
            static_cast<VPX::SoundConfigTypes>(m_player->m_ptable->m_settings.GetPlayer_Sound3D()));
      }, //
      [](Settings& settings) { settings.ResetPlayer_SoundDeviceBG(); }, //
      [this](int v, Settings& settings, bool isTableOverride) { settings.SetPlayer_SoundDeviceBG(m_devices[v], isTableOverride); })).m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Playfield Sound Device"s, "Select playfield sound device"s, false, 0, 0, m_devices), //
      [this]()
      {
         auto it = std::ranges::find(m_devices, m_player->m_audioPlayer->GetPlayfieldDeviceName());
         return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
      }, // Live
      [this](Settings& settings)
      {
         auto it = std::ranges::find(m_devices, settings.GetPlayer_SoundDevice());
         return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
      }, // Stored
      [this](int, int v)
      {
         m_player->m_audioPlayer = std::make_unique<VPX::AudioPlayer>( //
            m_player->m_ptable->m_settings.GetPlayer_SoundDeviceBG(), //
            m_devices[v], //
            static_cast<VPX::SoundConfigTypes>(m_player->m_ptable->m_settings.GetPlayer_Sound3D()));
      }, //
      [](Settings& settings) { settings.ResetPlayer_SoundDevice(); }, //
      [this](int v, Settings& settings, bool isTableOverride) { settings.SetPlayer_SoundDevice(m_devices[v], isTableOverride); })).m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Sound3D, //
      [this]() { return m_player->m_audioPlayer->GetSoundMode3D(); }, //
      [this](int, int v) {
         m_player->m_audioPlayer = std::make_unique<VPX::AudioPlayer>( //
            m_player->m_ptable->m_settings.GetPlayer_SoundDeviceBG(), //
            m_player->m_ptable->m_settings.GetPlayer_SoundDevice(), //
            static_cast<VPX::SoundConfigTypes>(v));
      }));
}
}
