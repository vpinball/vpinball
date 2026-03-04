// license:GPLv3+

#include "WMPCore.h"
#include "WMPControls.h"
#include "WMPSettings.h"
#include "WMPAudioPlayer.h"

namespace WMP {

WMPCore::WMPCore(MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int onAudioUpdateId)
{
   m_playState = wmppsUndefined;
   m_pAudioPlayer = new WMPAudioPlayer(msgApi, endpointId, onAudioUpdateId);

   m_pAudioPlayer->UpdateVolume(m_volume, m_mute);

   LOGI("WMP Core created"s);
}

WMPCore::~WMPCore()
{
   Close();

   delete m_pAudioPlayer;

   LOGI("WMP Core destroyed"s);
}

WMPControls* WMPCore::GetControls()
{
   return new WMPControls(this);
}

WMPSettings* WMPCore::GetSettings()
{
   return new WMPSettings(this); 
}

void WMPCore::Close()
{
   LOGI("WMP Core close"s);
   UnloadAudio();
   m_url.clear();
   m_playState = wmppsUndefined;
}

void WMPCore::SetURL(const string& url)
{
   m_url = url;
   LOGI("WMP Core URL set to: " + url);

   m_url = find_case_insensitive_file_path(m_url);
   m_playState = (!m_url.empty() && LoadAudio(m_url)) ? wmppsReady : wmppsUndefined;
}

WMPPlayState WMPCore::GetPlayState() const
{
   if (!m_pAudioPlayer->IsLoaded())
      return wmppsUndefined;

   if (m_pAudioPlayer->IsPlaying())
      return m_pAudioPlayer->IsPaused() ? wmppsPaused : wmppsPlaying;

   return wmppsStopped;
}

bool WMPCore::LoadAudio(const string& url)
{
   LOGI("Loading audio file: " + url);
   return m_pAudioPlayer->LoadFile(url);
}

void WMPCore::UnloadAudio()
{
   LOGI("Unloading current audio"s);
   m_pAudioPlayer->UnloadFile();
}

}
