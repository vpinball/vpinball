// license:GPLv3+

#include "core/stdafx.h"
#include "AudioSettingsPage.h"

#include "parts/pintable.h"

namespace VPX::InGameUI
{

AudioSettingsPage::AudioSettingsPage()
   : InGameUIPage("Audio Settings"s, ""s, SaveMode::Both)
{
}

void AudioSettingsPage::BuildPage()
{
   for (const auto& device : VPX::AudioPlayer::EnumerateAudioDevices())
      m_devices.push_back(device.name);

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Global Sound Levels"s));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Lock Volumes"s, "Adjust backglass and playfield main volume simultaneaously"s, false, true), //
      [this]() { return m_lockVolume; }, //
      [this](const Settings&) { return m_lockVolume; }, //
      [this](bool v) { m_lockVolume = v; }, //
      [](Settings&) { /* UI state, not persisted */ }, //
      [](bool, Settings&, bool) { /* UI state, not persisted */ }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MusicVolume, "%3d %%"s, //
      [this]() { return quantizeSignedPercent(m_player->m_backglassVolume); },
      [this](int prev, int v)
      {
         m_player->m_backglassVolume = dequantizeSignedPercent(v);
         if (m_lockVolume)
            m_player->m_playfieldVolume = clamp(m_player->m_playfieldVolume + dequantizeSignedPercent(v - prev), 0.f, 1.f);
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SoundVolume, "%3d %%"s, //
      [this]() { return quantizeSignedPercent(m_player->m_playfieldVolume); },
      [this](int prev, int v)
      {
         m_player->m_playfieldVolume = dequantizeSignedPercent(v);
         if (m_lockVolume)
            m_player->m_backglassVolume = clamp(m_player->m_backglassVolume + dequantizeSignedPercent(v - prev), 0.f, 1.f);
         m_player->UpdateVolume();
      }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Audio Source Levels"s));

   unsigned int getAudioSrcMsgId = m_player->m_pluginManager.GetMsgAPI().GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_GET_SRC_MSG);
   for (const auto& audioSrc : GetCtrlItems<AudioSrcId>(&m_player->m_pluginManager.GetMsgAPI(), m_player->m_pluginAPI.GetVPXEndPointId(), getAudioSrcMsgId))
   {
      MsgEndpointInfo info { };
      m_player->m_pluginManager.GetMsgAPI().GetEndpointInfo(audioSrc.id.endpointId, &info);
      const string endpointId = info.id ? info.id : std::to_string(audioSrc.id.endpointId);
      const string endpointName = audioSrc.name ? audioSrc.name : info.name ? info.name : endpointId;
      const string propId = std::format("AudioSource.{}.Gain", endpointId);
      const auto propPropId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(
         "Player"s, propId, std::format("{} Gain", endpointName), std::format("Volume gain applied to audio from '{}'.", endpointName), true, 0.f, 2.f, 0.f, 1.f));
      const uint64_t laneId = audioSrc.id.id;
      AddItem(std::make_unique<InGameUIItem>(
         *Settings::GetRegistry().GetFloatProperty(propPropId), 100.f, "%3.0f %%"s,
         [this, laneId]() { return m_player->GetAudioLaneMixerVolume(laneId); },
         [propPropId](const Settings& settings) { return settings.GetFloat(propPropId); },
         [this, laneId](float, float v) { m_player->SetAudioLaneMixerVolume(laneId, v); },
         [propPropId](Settings& settings) { settings.Reset(propPropId); },
         [propPropId](float v, Settings& settings, bool isTableOverride) { settings.Set(propPropId, v, isTableOverride); }));
   }
   m_player->m_pluginManager.GetMsgAPI().ReleaseMsgID(getAudioSrcMsgId);

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Hardware Setup"s));

   AddItem(std::make_unique<InGameUIItem>(
              VPX::Properties::EnumPropertyDef(""s, ""s, "Backglass Sound Device"s, "Select backglass sound device"s, false, 0, 0, m_devices), //
              [this]()
              {
                 auto it = std::ranges::find(m_devices, m_player->m_audioPlayer->GetBackglassDeviceName());
                 return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
              }, // Live
              [this](const Settings& settings)
              {
                 auto it = std::ranges::find(m_devices, settings.GetPlayer_SoundDeviceBG());
                 return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
              }, // Stored
              [this](int, int v)
              {
                 m_player->m_audioPlayer = std::make_unique<VPX::AudioPlayer>( //
                    m_devices[v], //
                    m_player->m_ptable->m_settings.GetPlayer_SoundDevice(), //
                    static_cast<VPX::SoundConfigTypes>(m_player->m_ptable->m_settings.GetPlayer_Sound3D()));
              }, //
              [](Settings& settings) { settings.ResetPlayer_SoundDeviceBG(); }, //
              [this](int v, Settings& settings, bool isTableOverride) { settings.SetPlayer_SoundDeviceBG(m_devices[v], isTableOverride); }))
      .m_excludeFromDefault = true;

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Playfield Sound Device"s, "Select playfield sound device"s, false, 0, 0, m_devices), //
      [this]()
      {
         auto it = std::ranges::find(m_devices, m_player->m_audioPlayer->GetPlayfieldDeviceName());
         return it == m_devices.end() ? 0 : (int)std::distance(m_devices.begin(), it);
      }, // Live
      [this](const Settings& settings)
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
