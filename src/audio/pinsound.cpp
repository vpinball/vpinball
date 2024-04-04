#include "stdafx.h"

float convert2decibelvolume(const float volume);

void BASS_ErrorMapCode(const int code, string& text)
{
	switch (code)
	{
	case BASS_OK: text = "all is OK"; break;
	case BASS_ERROR_MEM: text = "memory error"; break;
	case BASS_ERROR_FILEOPEN: text = "can't open the file"; break;
	case BASS_ERROR_DRIVER: text = "can't find a free/valid driver"; break;
	case BASS_ERROR_BUFLOST: text = "the sample buffer was lost"; break;
	case BASS_ERROR_HANDLE: text = "invalid handle"; break;
	case BASS_ERROR_FORMAT: text = "unsupported sample format"; break;
	case BASS_ERROR_POSITION: text = "invalid position"; break;
	case BASS_ERROR_INIT: text = "BASS_Init has not been successfully called"; break;
	case BASS_ERROR_START: text = "BASS_Start has not been successfully called"; break;
	case BASS_ERROR_SSL: text = "SSL/HTTPS support isn't available"; break;
	case BASS_ERROR_ALREADY: text = "already initialized/paused/whatever"; break;
	case BASS_ERROR_NOTAUDIO: text = "file does not contain audio"; break;
	case BASS_ERROR_NOCHAN: text = "can't get a free channel"; break;
	case BASS_ERROR_ILLTYPE: text = "an illegal type was specified"; break;
	case BASS_ERROR_ILLPARAM: text = "an illegal parameter was specified"; break;
	case BASS_ERROR_NO3D: text = "no 3D support"; break;
	case BASS_ERROR_NOEAX: text = "no EAX support"; break;
	case BASS_ERROR_DEVICE: text = "illegal device number"; break;
	case BASS_ERROR_NOPLAY: text = "not playing"; break;
	case BASS_ERROR_FREQ: text = "illegal sample rate"; break;
	case BASS_ERROR_NOTFILE: text = "the stream is not a file stream"; break;
	case BASS_ERROR_NOHW: text = "no hardware voices available"; break;
	case BASS_ERROR_EMPTY: text = "the MOD music has no sequence data"; break;
	case BASS_ERROR_NONET: text = "no internet connection could be opened"; break;
	case BASS_ERROR_CREATE: text = "couldn't create the file"; break;
	case BASS_ERROR_NOFX: text = "effects are not available"; break;
	case BASS_ERROR_NOTAVAIL: text = "requested data/action is not available"; break;
	case BASS_ERROR_DECODE: text = "the channel is/isn't a 'decoding channel'"; break;
	case BASS_ERROR_DX: text = "a sufficient DirectX version is not installed"; break;
	case BASS_ERROR_TIMEOUT: text = "connection timedout"; break;
	case BASS_ERROR_FILEFORM: text = "unsupported file format"; break;
	case BASS_ERROR_SPEAKER: text = "unavailable speaker"; break;
	case BASS_ERROR_VERSION: text = "invalid BASS version (used by add-ons)"; break;
	case BASS_ERROR_CODEC: text = "codec is not available/supported"; break;
	case BASS_ERROR_ENDED: text = "the channel/file has ended"; break;
	case BASS_ERROR_BUSY: text = "the device is busy"; break;
	case BASS_ERROR_UNSTREAMABLE: text = "unstreamable file"; break;
	case BASS_ERROR_UNKNOWN: text = "unknown error"; break;
	default: text = "unmapped error"; break;
	}
}

/*void CALLBACK PanDSP(HDSP handle, DWORD channel, void *buffer, DWORD length, void *user)
{
   const float pan = *((float*)user);
   if (pan == 0.f) return; // no processing neeeded for centre panning
   float * __restrict s = (float*)buffer;
   for (DWORD i = 0; i < length/8; ++i) {
      if (pan < 0.f)
         s[i*2+1] *= 1.f + pan; // pan left = reduce right
      else
         s[i*2  ] *= 1.f - pan; // vice versa
   }
}*/

PinSound::PinSound() : PinDirectSoundWavCopy(this)
{
   m_pDSBuffer = nullptr;
   m_pDS3DBuffer = nullptr;
   m_pdata = nullptr;
   m_pPinDirectSound = nullptr; // m_BASSstream = 0;
   m_outputTarget = SNDOUT_TABLE;
   m_balance = 0;
   m_fade = 0;
   m_volume = 0;
}

PinSound::~PinSound()
{
   UnInitialize();

   if (m_pdata)
      delete [] m_pdata;
}

void PinSound::UnInitialize()
{
   if (IsWav())
   {
#ifndef __STANDALONE__
      SAFE_PINSOUND_RELEASE(m_pDS3DBuffer);
      SAFE_PINSOUND_RELEASE(m_pDSBuffer);
#endif
   }
   else
   {
      if (m_BASSstream)
      {
         SetBassDevice();
         BASS_StreamFree(m_BASSstream);
         m_BASSstream = 0;
      }
   }
}

class PinDirectSound *PinSound::GetPinDirectSound()
{
   if (m_pPinDirectSound)
      return m_pPinDirectSound;
   else
      return g_pvp->m_ps.GetPinDirectSound(m_outputTarget);
}

HRESULT PinSound::ReInitialize()
{
   UnInitialize();

   if(!IsWav())
   {
	   const SoundConfigTypes SoundMode3D = (m_outputTarget == SNDOUT_BACKGLASS) ? SNDCFG_SND3D2CH : (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

	   SetBassDevice();
	   m_BASSstream = BASS_StreamCreateFile(
		   TRUE,
		   m_pdata,
		   0,
		   m_cdata,
		   (SoundMode3D != SNDCFG_SND3D2CH) ? (BASS_SAMPLE_3D | BASS_SAMPLE_MONO) : 0 /*| BASS_SAMPLE_LOOP*/ //!! mono really needed? doc claims so
	   );

	   if (m_BASSstream == 0)
	   {
		   const int code = BASS_ErrorGetCode();
		   string bla2;
		   BASS_ErrorMapCode(code, bla2);
		   g_pvp->MessageBox(("BASS music/sound library cannot create stream \"" + m_szPath + "\" (error " + std::to_string(code) + ": " + bla2 + ')').c_str(), "Error", MB_ICONERROR);
		   return E_FAIL;
	   }
	   else {
		   BASS_ChannelGetAttribute(m_BASSstream, BASS_ATTRIB_FREQ, &m_freq);
	   }

	   return S_OK;
   }

#ifndef __STANDALONE__
   // else old wav code:

   PinDirectSound * const pds = GetPinDirectSound();
   if (pds->m_pDS == nullptr)
   {
      m_pPinDirectSound = nullptr;
      m_pDSBuffer = nullptr;
      return E_FAIL;
   }

   const SoundConfigTypes SoundMode3D = (m_outputTarget == SNDOUT_BACKGLASS) ? SNDCFG_SND3D2CH : (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

   WAVEFORMATEX wfx = m_wfx;  // Use a copy as we might be modifying it
   // Remark from MSDN: "If wFormatTag = WAVE_FORMAT_PCM or wFormatTag = WAVE_FORMAT_IEEE_FLOAT, set cbSize to zero"
   // Otherwise some tables crash in dsound when using certain WAVE_FORMAT_IEEE_FLOAT samples
   if ((wfx.wFormatTag == WAVE_FORMAT_PCM) || (wfx.wFormatTag == WAVE_FORMAT_IEEE_FLOAT))
      wfx.cbSize = 0;

   DSBUFFERDESC dsbd = {};
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | ((SoundMode3D != SNDCFG_SND3D2CH) ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN);
   dsbd.dwBufferBytes = m_cdata;
   dsbd.lpwfxFormat = &wfx;

   // If we are in a 2CH mode and the sample is a single channel and the .WAV file is in a
   // PCM format then we double the size of our ds buffer here and below we duplicate the 
   // mono channel so windows will play the sample on Left/Right channels and not center 
   // channel if Windows audio is configured for SurroundSound. I don't know if non-PCM 
   // formats were possible in the first place, but the channel duplication would definitely 
   // not work so we try to check for that here. *njk*

   if ((SoundMode3D == SNDCFG_SND3D2CH) && (m_wfx.nChannels == 1) && ((m_wfx.wFormatTag == WAVE_FORMAT_PCM) || (m_wfx.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)))
   {
	   wfx.nChannels = m_wfx.nChannels * 2;
	   wfx.nBlockAlign = m_wfx.nBlockAlign * 2;
	   wfx.nAvgBytesPerSec = m_wfx.nAvgBytesPerSec * 2;
	   dsbd.dwBufferBytes *= 2;
   }

   // Create the static DirectSound buffer 
   HRESULT hr;
   if (FAILED(hr = pds->m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, nullptr)))
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Error: 0x%X. Could not create sound buffer for load.", hr);
      ShowError(bla);
      m_pPinDirectSound = nullptr;
      m_pDSBuffer = nullptr;
      return hr;
   }

   m_pPinDirectSound = pds;

   // Lock the buffer down
   VOID*   pbData = nullptr;
   VOID*   pbData2 = nullptr;
   DWORD   dwLength,dwLength2;
   if (FAILED(hr = m_pDSBuffer->Lock(0, dsbd.dwBufferBytes, &pbData, &dwLength,
      &pbData2, &dwLength2, 0L)))
   {
      ShowError("Could not lock sound buffer for load.");
      return hr;
   }
   // Convert mono sample to dual channel to prevent Windows from playing it
   // on the center channel when in Surround Sound mode. We double the sample
   // buffer size above, now we duplicate data when copying it into the buffer.
   // *njk*

   if ((DWORD)m_cdata < dsbd.dwBufferBytes) // if buffer was resized then duplicate channel
   {
	   const unsigned int bps = wfx.wBitsPerSample / 8;
	   char * __restrict s = m_pdata;
	   char * __restrict d = (char*)pbData;

	   for (DWORD i = 0; i < dsbd.dwBufferBytes; i += wfx.nBlockAlign)
	   {
		   for (unsigned int j = 0; j < bps; j++)
			   *d++ = *s++;

		   s -= bps;

		   for (unsigned int j = 0; j < bps; j++)
			   *d++ = *s++;
	   }
   }
   else
   {
	   // Copy the memory to it.
	   memcpy(pbData, m_pdata, m_cdata);
   }
   // Unlock the buffer, we don't need it anymore.
   m_pDSBuffer->Unlock(pbData, m_cdata, nullptr, 0);

   if (SoundMode3D != SNDCFG_SND3D2CH)
      Get3DBuffer();
#endif

   return S_OK;
}

void PinSound::SetBassDevice()
{
   const int bass_idx = (m_outputTarget == SNDOUT_BACKGLASS) ? g_pvp->m_ps.bass_BG_idx : g_pvp->m_ps.bass_STD_idx;
   if (bass_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(bass_idx);
}

void PinSound::Play(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart)
{
   if (IsWav())
      PlayInternal(volume, randompitch, pitch, pan, front_rear_fade, flags, restart);
   else if (m_BASSstream)
   {
      SetBassDevice();

      BASS_ChannelSetAttribute(m_BASSstream, BASS_ATTRIB_VOL, sqrtf(saturate(volume*(float)(1.0/100.)))); // to match VP legacy

      if (randompitch > 0.f)
      {
         float freq = m_freq + (float)pitch;
         const float rndh = rand_mt_01();
         const float rndl = rand_mt_01();
         BASS_ChannelSetAttribute(m_BASSstream, BASS_ATTRIB_FREQ, freq + (freq * randompitch * rndh * rndh) - (freq * randompitch * rndl * rndl * 0.5f));
      }
      else if (pitch != 0)
      {
         float freq = m_freq + (float)pitch;
         BASS_ChannelSetAttribute(m_BASSstream, BASS_ATTRIB_FREQ, freq);
      }

      const SoundConfigTypes SoundMode3D = (m_outputTarget == SNDOUT_BACKGLASS) ? SNDCFG_SND3D2CH : (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);
      switch (SoundMode3D)
      {
      case SNDCFG_SND3DALLREAR:
      {
         const BASS_3DVECTOR v(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(1.0f));
         BASS_ChannelSet3DPosition(m_BASSstream, &v, nullptr, nullptr);
         BASS_Apply3D();
         break;
      }
      case SNDCFG_SND3DFRONTISFRONT:
      {
         const BASS_3DVECTOR v(PinDirectSound::PanTo3D(pan), 0.0f, PinDirectSound::PanTo3D(front_rear_fade));
         BASS_ChannelSet3DPosition(m_BASSstream, &v, nullptr, nullptr);
         BASS_Apply3D();
         break;
      }
      case SNDCFG_SND3DFRONTISREAR:
      {
         const BASS_3DVECTOR v(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(front_rear_fade));
         BASS_ChannelSet3DPosition(m_BASSstream, &v, nullptr, nullptr);
         BASS_Apply3D();
         break;
      }
      case SNDCFG_SND3D6CH:
      {
         const BASS_3DVECTOR v(PinDirectSound::PanTo3D(pan), 0.0f, -((PinDirectSound::PanTo3D(front_rear_fade) + 3.0f) / 2.0f));
         BASS_ChannelSet3DPosition(m_BASSstream, &v, nullptr, nullptr);
         BASS_Apply3D();
         break;
      }
      case SNDCFG_SND3DSSF:
      {
         const BASS_3DVECTOR v(PinDirectSound::PanSSF(pan), 0.0f, PinDirectSound::FadeSSF(front_rear_fade));
         BASS_ChannelSet3DPosition(m_BASSstream, &v, nullptr, nullptr);
         BASS_Apply3D();
         break;
      }
      case SNDCFG_SND3D2CH:
      default:
         BASS_ChannelSetAttribute(m_BASSstream, BASS_ATTRIB_PAN, pan);
         //!! When using DirectSound output on Windows, this attribute has no effect when speaker assignment is used,
         //   except on Windows Vista and newer with the BASS_CONFIG_VISTA_SPEAKERS config option enabled, so this would only be needed on pre-Vista systems now!
         //m_pan = pan;
         //if(pan != 0.f)
         //   BASS_ChannelSetDSP(m_BASSstream, PanDSP, &m_pan, 0);
         break;
      }

      if (flags & DSBPLAY_LOOPING)
         BASS_ChannelFlags(m_BASSstream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
      else
         BASS_ChannelFlags(m_BASSstream, 0, BASS_SAMPLE_LOOP);

      if (BASS_ChannelIsActive(m_BASSstream) != BASS_ACTIVE_PLAYING)
         BASS_ChannelPlay(m_BASSstream, true);
      else if (restart)
         BASS_ChannelSetPosition(m_BASSstream, 0, BASS_POS_BYTE);
   }
}

void PinSound::Stop()
{
   if (IsWav())
      StopInternal();
   else
      if (m_BASSstream)
      {
			SetBassDevice();
         BASS_ChannelStop(m_BASSstream);
      }
}

PinDirectSound::~PinDirectSound()
{
#ifndef __STANDALONE__
   SAFE_PINSOUND_RELEASE(m_pDSListener);
   SAFE_PINSOUND_RELEASE(m_pDS);
#endif
}

BOOL CALLBACK DSEnumCallBack(LPGUID guid, LPCSTR desc, LPCSTR mod, LPVOID list)
{
   DSAudioDevice *ad = new DSAudioDevice;
   if (guid == nullptr)
      ad->guid = nullptr;
   else {
      ad->guid = new GUID;
      memcpy(ad->guid, guid, sizeof(GUID));
   }
   ad->description = desc;
   ad->module = mod;
   (static_cast<DSAudioDevices*>(list))->push_back(ad);
   return fTrue;
}

void PinDirectSound::InitDirectSound(const HWND hwnd, const bool IsBackglass)
{
#ifdef DEBUG_NO_SOUND
   return;
#endif
#ifndef __STANDALONE__
   SAFE_PINSOUND_RELEASE(m_pDSListener);
   SAFE_PINSOUND_RELEASE(m_pDS);

   DSAudioDevices DSads;
   int DSidx = 0;
   if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
   {
      const bool hr = g_pvp->m_settings.LoadValue(Settings::Player, IsBackglass ? "SoundDeviceBG"s : "SoundDevice"s, DSidx);
      if ((!hr) || ((size_t)DSidx >= DSads.size()))
         DSidx = 0; // The default primary sound device
   }

   // Create IDirectSound using the selected sound device
   HRESULT hr;
   if (FAILED(hr = DirectSoundCreate((DSidx != 0) ? DSads[DSidx]->guid : nullptr, &m_pDS, nullptr)))
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not create Direct Sound.", hr);
      ShowError(bla);
      return;// hr;
   }

   // free audio devices list
   for (size_t i = 0; i < DSads.size(); i++)
      delete DSads[i];

   // Set coop level to DSSCL_PRIORITY
   if (FAILED(hr = m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not set Direct Sound Priority.", hr);
      ShowError(bla);
      return;// hr;
   }

   const SoundConfigTypes SoundMode3D = (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

   // Get the primary buffer 
   DSBUFFERDESC dsbd = {};
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
   if (!IsBackglass && (SoundMode3D != SNDCFG_SND3D2CH))
      dsbd.dwFlags |= DSBCAPS_CTRL3D;
   dsbd.dwBufferBytes = 0;
   dsbd.lpwfxFormat = nullptr;

   LPDIRECTSOUNDBUFFER pDSBPrimary = nullptr;
   if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, nullptr)))
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not create primary sound buffer.", hr);
      ShowError(bla);
      return;// hr;
   }

   // Set primary buffer format to 44kHz and 16-bit output.
   WAVEFORMATEX wfx = {};
   wfx.wFormatTag = WAVE_FORMAT_PCM;
   wfx.nChannels = (!IsBackglass && (SoundMode3D != SNDCFG_SND3D2CH)) ?  1 : 2;
   wfx.nSamplesPerSec = 44100;
   wfx.wBitsPerSample = 16;
   wfx.nBlockAlign = wfx.wBitsPerSample / (WORD)8 * wfx.nChannels;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

   if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
   {
      char bla[128];
      sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not set sound format.", hr);
      ShowError(bla);
      return;// hr;
   }
   if (!IsBackglass && (SoundMode3D != SNDCFG_SND3D2CH))
   {
	   // Obtain a listener interface.
	   hr = pDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_pDSListener);
	   if (FAILED(hr))
	   {
	      char bla[128];
	      sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not acquire 3D listener interface.", hr);
	      ShowError(bla);
	      return;// hr;
	   }

	   // Set the initial position of the listener to be sitting in between the front two speakers. 
	   m_pDSListener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);
   }
   SAFE_PINSOUND_RELEASE(pDSBPrimary);
#endif

   //return S_OK;
}

void AudioMusicPlayer::InitPinDirectSound(const Settings& settings, const HWND hwnd)
{
#ifndef __STANDALONE__
   const int DSidx1 = settings.LoadValueWithDefault(Settings::Player, "SoundDevice"s, 0);
   const int DSidx2 = settings.LoadValueWithDefault(Settings::Player, "SoundDeviceBG"s, 0);
#else
   const int DSidx1 = settings.LoadValueWithDefault(Settings::Player, "SoundDevice"s, -1);
   const int DSidx2 = settings.LoadValueWithDefault(Settings::Player, "SoundDeviceBG"s, -1);
#endif
   const SoundConfigTypes SoundMode3D = (SoundConfigTypes)settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

   //---- Initialize BASS Audio Library

   int prevBassStdIdx = bass_STD_idx;
   int prevBassBGIdx = bass_BG_idx;
#ifndef __STANDALONE__
   bass_STD_idx = -1;
   bass_BG_idx = -1;
#else
   bass_STD_idx = DSidx1;
   bass_BG_idx  = DSidx2;
#endif
#ifndef __STANDALONE__
   for (unsigned int idx = 0; idx < 2; ++idx)
   {
      const int DSidx = (idx == 0) ? DSidx1 : DSidx2;

		// Match the Direct Sound device with the BASS device by name
      if (DSidx != -1)
      {
         DSAudioDevices DSads;
         if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
         {
            if ((size_t)DSidx < DSads.size() && DSads[DSidx]->guid != nullptr) // primary device has guid nullptr, so use BASS_idx = -1 in that case
            {
               BASS_DEVICEINFO dinfo;
               for (int i = 1; BASS_GetDeviceInfo(i, &dinfo); i++) // 0 = no sound/no device
                  if (dinfo.flags & BASS_DEVICE_ENABLED) // device must be enabled
                     if (strcmp(dinfo.name, DSads[DSidx]->description.c_str()) == 0)
                     {
                        if (idx == 0)
                           bass_STD_idx = (dinfo.flags & BASS_DEVICE_DEFAULT) ? -1 : i;
                        else
                           bass_BG_idx = (dinfo.flags & BASS_DEVICE_DEFAULT) ? -1 : i;
                        break;
                     }
            }
            for (size_t i = 0; i < DSads.size(); i++)
               delete DSads[i];
         }
      }
   }
#endif

   //BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_FLOATDSP, fTrue);
   BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_CURVE_PAN, fTrue); // logarithmic scale, similar to DSound (although BASS still takes a 0..1 range)
   //!! BASS_CONFIG_THREAD so far only works on Net stuff, not these ones here..  :/
   //BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_CURVE_VOL, fTrue); // dto. // is now converted internally, as otherwise PinMAMEs altsound will also get affected! (note that pan is not used yet in PinMAME!)
   BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_VISTA_SPEAKERS, fTrue); // to make BASS_ChannelSetAttribute(.., BASS_ATTRIB_PAN, pan); work, needs Vista or later

   for (unsigned int idx = 0; idx < 2; ++idx)
   {
      int deviceIdx = (idx == 0) ? bass_STD_idx : bass_BG_idx;
		BASS_INFO info;
      const bool isReInit = BASS_SetDevice(deviceIdx) && BASS_GetInfo(&info);
      PLOGI << "Initializing BASS device #" << deviceIdx << " [Reinit: " << isReInit << "]";
      if (!BASS_Init(deviceIdx, 44100, 
				(isReInit ? BASS_DEVICE_REINIT : 0) | ((SoundMode3D != SNDCFG_SND3D2CH) && (idx == 0) ? 0 /*| BASS_DEVICE_MONO*/ /*| BASS_DEVICE_DSOUND*/ : 0),
            g_pvp->GetHwnd(), nullptr)) // note that sample rate is usually ignored and set depending on the input/file automatically
      {
         const int code = BASS_ErrorGetCode();
         string bla;
         BASS_ErrorMapCode(code, bla);
         PLOGE << ("BASS music/sound library initialization error " + std::to_string(code) + ": " + bla).c_str();
         g_pvp->MessageBox(("BASS music/sound library initialization error " + std::to_string(code) + ": " + bla).c_str(), "Error", MB_ICONERROR);
      }
      if (/*SoundMode3D == SNDCFG_SND3D2CH &&*/ bass_STD_idx == bass_BG_idx) // skip 2nd device if it's the same and 3D is disabled //!!! for now try to just use one even if 3D! and then adapt channel settings if sample is a backglass sample
         break;
   }

   //---- Initialize DirectSound
   m_pds.InitDirectSound(hwnd, false);
   // If these are the same device, and we are not in 3d mode, just point the backglass device to the main one.
   // For 3D we want two separate instances, one in basic stereo for music, and the other surround mode.
   if (SoundMode3D == SNDCFG_SND3D2CH && DSidx1 == DSidx2)
   {
      m_pbackglassds = &m_pds;
   }
   else
   {
      m_pbackglassds = new PinDirectSound();
      m_pbackglassds->InitDirectSound(hwnd, true);
   }
}

PinSound *AudioMusicPlayer::LoadFile(const string& strFileName)
{
   PinSound * const pps = new PinSound();

   pps->m_szPath = strFileName;
   pps->m_szName = TitleFromFilename(strFileName);

   if (pps->IsWav()) // only use old direct sound code and wav reader if playing wav's
   {
#ifndef __STANDALONE__
	   // Create a new wave file class
	   CWaveSoundRead* const pWaveSoundRead = new CWaveSoundRead();

	   // Load the wave file
	   if (FAILED(pWaveSoundRead->Open(strFileName)))
	   {
		   ShowError("Could not open wav file.");
		   delete pWaveSoundRead;
		   delete pps;
		   return nullptr;
	   }

	   const SoundConfigTypes SoundMode3D = (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

	   // Set up the direct sound buffer, and only request the flags needed
	   // since each requires some overhead and limits if the buffer can
	   // be hardware accelerated
	   DSBUFFERDESC dsbd = {};
	   dsbd.dwSize = sizeof(DSBUFFERDESC);
	   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN;
	   if (SoundMode3D != SNDCFG_SND3D2CH)
		   dsbd.dwFlags |= DSBCAPS_CTRL3D;
	   dsbd.dwBufferBytes = pWaveSoundRead->m_ckIn.cksize;
	   dsbd.lpwfxFormat = pWaveSoundRead->m_pwfx;
	   memcpy(&pps->m_wfx, pWaveSoundRead->m_pwfx, sizeof(pps->m_wfx));

	   // Create the static DirectSound buffer
	   HRESULT hr;
	   if (FAILED(hr = m_pds.m_pDS->CreateSoundBuffer(&dsbd, &pps->m_pDSBuffer, nullptr)))
	   {
		   char bla[128];
		   sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not create static sound buffer.", hr);
		   ShowError(bla);
		   delete pWaveSoundRead;
		   delete pps;
		   return nullptr;
	   }
	   if (SoundMode3D != SNDCFG_SND3D2CH)
		   pps->Get3DBuffer();

	   // Remember how big the buffer is
	   pps->m_cdata = dsbd.dwBufferBytes;
	   pps->m_pPinDirectSound = &m_pds;

	   // Fill the buffer with wav data

		  // Allocate that buffer.
		  pps->m_pdata = new char[pps->m_cdata];

		  UINT cbWavSize; // Size of data
		  if (FAILED(hr = pWaveSoundRead->Read(pps->m_cdata,
			  (BYTE*)pps->m_pdata,
			  &cbWavSize)))
		  {
			  ShowError("Could not read wav file.");
			  delete pWaveSoundRead;
			  delete pps;
			  return nullptr;
		  }

		  delete pWaveSoundRead;

		  // Lock the buffer down
		  VOID* pbData = nullptr;
		  VOID* pbData2 = nullptr;
		  DWORD dwLength, dwLength2;
		  if (FAILED(hr = pps->m_pDSBuffer->Lock(0, pps->m_cdata, &pbData, &dwLength,
			  &pbData2, &dwLength2, 0L)))
		  {
			  ShowError("Could not lock sound buffer.");
			  delete pps;
			  return nullptr;
		  }
		  // Copy the memory to it.
		  memcpy(pbData, pps->m_pdata, pps->m_cdata);
		  // Unlock the buffer, we don't need it anymore.
		  pps->m_pDSBuffer->Unlock(pbData, pps->m_cdata, nullptr, 0);

         // Update the UI controls to show the sound as the file is loaded
         //SetFileUI( hDlg, strFileName );
         //OnEnablePlayUI( hDlg, TRUE );
#endif
   }
   else
   {
	   FILE *f;
	   if (fopen_s(&f, strFileName.c_str(), "rb") != 0 || !f)
	   {
		   ShowError("Could not open sound file.");
		   return nullptr;
	   }
	   fseek(f, 0, SEEK_END);
	   pps->m_cdata = (int)ftell(f);
	   fseek(f, 0, SEEK_SET);
	   pps->m_pdata = new char[pps->m_cdata];
	   fread_s(pps->m_pdata, pps->m_cdata, 1, pps->m_cdata, f);
	   fclose(f);

	   const SoundConfigTypes SoundMode3D = (pps->GetOutputTarget() == SNDOUT_BACKGLASS) ? SNDCFG_SND3D2CH : (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

	   pps->SetBassDevice();
	   pps->m_BASSstream = BASS_StreamCreateFile(
		   TRUE,
		   pps->m_pdata,
		   0,
		   pps->m_cdata,
		   (SoundMode3D != SNDCFG_SND3D2CH) ? (BASS_SAMPLE_3D | BASS_SAMPLE_MONO) : 0 /*| BASS_SAMPLE_LOOP*/ //!! mono really needed? doc claims so
	   );

	   if (pps->m_BASSstream == 0)
	   {
		   delete pps;

		   const int code = BASS_ErrorGetCode();
		   string bla2;
		   BASS_ErrorMapCode(code, bla2);
		   g_pvp->MessageBox(("BASS music/sound library cannot load \"" + strFileName + "\" (error " + std::to_string(code) + ": " + bla2 + ')').c_str(), "Error", MB_ICONERROR);
		   return nullptr;
	   }
	   else {
		   BASS_ChannelGetAttribute(pps->m_BASSstream, BASS_ATTRIB_FREQ, &pps->m_freq);
	   }
   }

   return pps;
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// the total distance necessary to fully pan away from one side at the center of the room.

float PinDirectSound::PanTo3D(float input)
{
	// DirectSound's position command does weird things at exactly 0. 
	if (fabsf(input) < 0.0001f)
		input = (input < 0.0f) ? -0.0001f : 0.0001f;
	if (input < 0.0f)
	{
		return -powf(-max(input, -1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
	else
	{
		return powf(min(input, 1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
}

// This is a replacement function for PanTo3D() for sound effect panning (audio x-axis).
// It performs the same calculations but maps the resulting values to an area of the 3D 
// sound stage that has the expected panning effect for this application. It is written 
// in a long form to facilitate tweaking the formulas.  *njk*

float PinDirectSound::PanSSF(float pan)
{
	// This math could probably be simplified but it is kept in long form
	// to aide in fine tuning and clarity of function.

	// Clip the pan input range to -1.0 to 1.0
	float x = clamp(pan, -1.f, 1.f);

	// Rescale pan range from an exponential [-1,0] and [0,1] to a linear [-1.0, 1.0]
	// Do not avoid values close to zero like PanTo3D() does as that
	// prevents the middle range of the exponential curves converting back to 
	// a linear scale (which would leave a gap in the center of the range).
	// This basically undoes the Pan() fading function in the table scripts.

	x = (x < 0.0f) ? -powf(-x, 0.1f) : powf(x, 0.1f);

	// Increase the pan range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	x *= 3.0f;

	// BASS pan effect is much better than VPX 10.6/DirectSound3d but it
	// could still stand a little enhancement to exaggerate the effect.
	// The effect goal is to place slingshot effects almost entirely left/right
	// and flipper effects in the cross fade region (louder on their corresponding
	// sides but still audible on the opposite side..)

	// Rescale [-3.0,0.0) to [-3.00,-2.00] and [0,3.0] to [2.00,3.00]

	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// x' = ( (x - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	if (x >= 0.0f)
		x = ((x -  0.0f) / (3.0f -  0.0f)) * ( 3.0f -  2.0f) +  2.0f;
	else
		x = ((x - -3.0f) / (0.0f - -3.0f)) * (-2.0f - -3.0f) + -2.0f;

	// Clip the pan output range to 3.0 to -3.0
	//
	// This probably can never happen but is here in case the formulas above
	// change or there is a rounding issue.

	if (x > 3.0f)
		x = 3.0f;
	else if (x < -3.0f)
		x = -3.0f;

	// If the final value is sufficiently close to zero it causes sound to come from
	// all speakers and lose it's positional effect. We scale well away from zero
	// above but will keep this check to document the effect or catch the condition
	// if the formula above is later changed to one that can result in x = 0.0.

	// NOTE: This no longer seems to be the case with VPX 10.7/BASS

	// HOWEVER: Weird things still happen NEAR 0.0 or if both x and z are at 0.0.
	//          So we keep the fix here with wider margins to prevent that case.
	//          The current formula won't produce values in this weird range.

	if (fabsf(x) < 0.1f)
		x = (x < 0.0f) ? -0.1f : 0.1f;

	return x;
}

// This is a replacement function for PanTo3D() for sound effect fading (audio z-axis).
// It performs the same calculations but maps the resulting values to 
// an area of the 3D sound stage that has the expected fading
// effect for this application. It is written in a long form to facilitate tweaking the 
// values (which turned out to be more straightforward than originally coded). *njk*

float PinDirectSound::FadeSSF(float front_rear_fade)
{
	float z = 0.0f;

	// Clip the fade input range to -1.0 to 1.0

	if (front_rear_fade < -1.0f)
		z = -1.0f;
	else if (front_rear_fade > 1.0f)
		z = 1.0f;
	else
		z = front_rear_fade;

	// Rescale fade range from an exponential [0,-1] and [0,1] to a linear [-1.0, 1.0]
	// Do not avoid values close to zero like PanTo3D() does at this point as that
	// prevents the middle range of the exponential curves converting back to 
	// a linear scale (which would leave a gap in the center of the range).
	// This basically undoes the AudioFade() fading function in the table scripts.	

	z = (z < 0.0f) ? -powf(-z, 0.1f) : powf(z, 0.1f);

	// Increase the fade range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	z *= 3.0f;

	// Rescale fade range from [-3.0,3.0] to [0.0,-2.5] in an attempt to remove all sound from
	// the surround sound front (backbox) speakers and place them close to the surround sound
	// side (cabinet rear) speakers.
	//
	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// z' = ( (z - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	// Rescale to -2.5 instead of -3.0 to further push sound away from rear channels
	z = ((z - -3.0f) / (3.0f - -3.0f)) * (-2.5f - 0.0f) + 0.0f;

	// With BASS the above scaling is sufficient to keep the playfield sounds out of 
	// the backbox. However playfield sounds are heavily weighted to the rear channels. 
	// For BASS we do a simple scale of the top third [0,-1.0] BY 0.10 to favor
	// the side channels. This is better than we could do in VPX 10.6 where z just
	// had to be set to 0.0 as there was no fade range that didn't leak to the backbox
	// as well.
	
	if (z > -1.0f)
		z = z / 10.0f;

	// Clip the fade output range to 0.0 to -3.0
	//
	// This probably can never happen but is here in case the formulas above
	// change or there is a rounding issue. A result even slightly greater
	// than zero can bleed to the backbox speakers.

	if (z > 0.0f)
		z = 0.0f;
	else if (z < -3.0f)
		z = -3.0f;

	// If the final value is sufficiently close to zero it causes sound to come from
	// all speakers on some systems and lose it's positional effect. We do use 0.0 
	// above and could set the safe value there. Instead will keep this check to document 
	// the effect or catch the condition if the formula/conditions above are later changed

	// NOTE: This no longer seems to be the case with VPX 10.7/BASS

	// HOWEVER: Weird things still happen near 0.0 or if both x and z are at 0.0.
	//          So we keep the fix here to prevent that case. This does push a tiny bit 
	//          of audio to the rear channels but that is perfectly ok.

	if (fabsf(z) < 0.0001f)
		z = -0.0001f;
	
	return z;
}

PinDirectSoundWavCopy::PinDirectSoundWavCopy(class PinSound * const pOriginal)
{
#ifndef __STANDALONE__
	m_ppsOriginal = pOriginal;

	if (this != pOriginal)
	{
		m_pDSBuffer = nullptr;
		m_pDS3DBuffer = nullptr;
		pOriginal->GetPinDirectSound()->m_pDS->DuplicateSoundBuffer(pOriginal->m_pDSBuffer, &m_pDSBuffer);
		if (m_pDSBuffer && pOriginal->m_pDS3DBuffer != nullptr)
			Get3DBuffer();
	}
#endif
}

void PinDirectSoundWavCopy::PlayInternal(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart)
{
#ifndef __STANDALONE__
	m_pDSBuffer->SetVolume((LONG)convert2decibelvolume(volume));

	// Frequency tweaks are relative to original sound.  If the copy failed for some reason, don't alter original
	if (m_ppsOriginal != this)
	{
		if (randompitch > 0.f)
		{
			DWORD freq;
			m_ppsOriginal->m_pDSBuffer->GetFrequency(&freq);
			freq += pitch;
			const float rndh = rand_mt_01();
			const float rndl = rand_mt_01();
			m_pDSBuffer->SetFrequency(freq + (DWORD)((float)freq * randompitch * rndh * rndh) - (DWORD)((float)freq * randompitch * rndl * rndl * 0.5f));
		}
		else if (pitch != 0)
		{
			DWORD freq;
			m_ppsOriginal->m_pDSBuffer->GetFrequency(&freq);
			m_pDSBuffer->SetFrequency(freq + pitch);
		}
	}

	const SoundConfigTypes SoundMode3D = (m_ppsOriginal->GetOutputTarget() == SNDOUT_BACKGLASS) ? SNDCFG_SND3D2CH : (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);

	switch (SoundMode3D)
	{
	case SNDCFG_SND3DALLREAR:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(1.0f), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3DFRONTISFRONT:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, PinDirectSound::PanTo3D(front_rear_fade), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3DFRONTISREAR:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(front_rear_fade), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3D6CH:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -((PinDirectSound::PanTo3D(front_rear_fade) + 3.0f) / 2.0f), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3DSSF:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanSSF(pan), 0.0f, PinDirectSound::FadeSSF(front_rear_fade), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3D2CH:
	default:
		if (pan != 0.f)
			m_pDSBuffer->SetPan((LONG)(pan*DSBPAN_RIGHT));
		break;
	}

	DWORD status;
	m_pDSBuffer->GetStatus(&status);
	if (!(status & DSBSTATUS_PLAYING))
		m_pDSBuffer->Play(0, 0, flags);
	else if (restart)
		m_pDSBuffer->SetCurrentPosition(0);
#endif
}

HRESULT PinDirectSoundWavCopy::Get3DBuffer()
{
#ifndef __STANDALONE__
	const HRESULT hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&m_pDS3DBuffer);
	if (FAILED(hr))
	{
		char bla[128];
		sprintf_s(bla, sizeof(bla), "Error 0x%X. Could not get interface to 3D sound buffer.", hr);
		ShowError(bla);
	}
	else
		m_pDS3DBuffer->SetMinDistance(5.0f, DS3D_IMMEDIATE);
	return hr;
#else
    return S_OK;
#endif
}

#ifdef __STANDALONE__
void EnumerateAudioDevices(vector<AudioDevice>& audioDevices)
{
   audioDevices.clear();

   BASS_DEVICEINFO info;
   for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) {
      AudioDevice audioDevice = {};
      audioDevice.id = i;
      strncpy((char*)audioDevice.name, info.name, MAX_DEVICE_IDENTIFIER_STRING);
      audioDevice.enabled = (info.flags & BASS_DEVICE_ENABLED);
      audioDevices.push_back(audioDevice);
   }
}
#endif