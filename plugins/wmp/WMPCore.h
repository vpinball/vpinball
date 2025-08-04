#pragma once

#include "common.h"
#include "WMPControls.h"
#include "WMPSettings.h"
#include "WMPAudioPlayer.h"

namespace WMP {

class WMPControls;
class WMPSettings; 
class WMPAudioPlayer;

enum WMPPlayState
{
   wmppsUndefined = 0,
   wmppsStopped = 1,
   wmppsPaused = 2,
   wmppsPlaying = 3,
   wmppsScanForward = 4,
   wmppsScanReverse = 5,
   wmppsBuffering = 6,
   wmppsWaiting = 7,
   wmppsMediaEnded = 8,
   wmppsTransitioning = 9,
   wmppsReady = 10,
   wmppsReconnecting = 11,
   wmppsLast = 12
};

enum WMPOpenState
{
   wmposUndefined = 0,
   wmposPlaylistChanging = 1,
   wmposPlaylistLocating = 2,
   wmposPlaylistConnecting = 3,
   wmposPlaylistLoading = 4,
   wmposPlaylistOpening = 5,
   wmposPlaylistOpenNoMedia = 6,
   wmposPlaylistChanged = 7,
   wmposMediaChanging = 8,
   wmposMediaLocating = 9,
   wmposMediaConnecting = 10,
   wmposMediaLoading = 11,
   wmposMediaOpening = 12,
   wmposMediaOpen = 13,
   wmposBeginCodecAcquisition = 14,
   wmposEndCodecAcquisition = 15,
   wmposBeginLicenseAcquisition = 16,
   wmposEndLicenseAcquisition = 17,
   wmposBeginIndividualization = 18,
   wmposEndIndividualization = 19,
   wmposMediaWaiting = 20,
   wmposOpeningUnknownURL = 21
};

class WMPCore
{
public:
   WMPCore(MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int onAudioUpdateId);
   ~WMPCore();

   PSC_IMPLEMENT_REFCOUNT()

   void Close();
   string GetURL() const { return m_url; }
   void SetURL(const string& url);
   WMPOpenState GetOpenState() const { PSC_NOT_IMPLEMENTED("GetOpenState is not implemented"); return wmposUndefined; }
   WMPPlayState GetPlayState() const;
   WMPControls* GetControls();
   WMPSettings* GetSettings();
   WMPAudioPlayer* GetAudioPlayer() const { return m_pAudioPlayer; }
   string GetVersionInfo() const { PSC_NOT_IMPLEMENTED("GetVersionInfo is not implemented"); return string(); }
   bool GetIsOnline() const { PSC_NOT_IMPLEMENTED("GetIsOnline is not implemented"); return false; }
   string GetStatus() const { PSC_NOT_IMPLEMENTED("GetStatus is not implemented"); return string(); }

public:
   bool m_autoStart = false;
   bool m_mute = false;
   long m_volume = 50;

private:
   string m_url;
   WMPPlayState m_playState;
   WMPAudioPlayer* m_pAudioPlayer;

   bool LoadAudio(const string& url);
   void UnloadAudio();
};

}
