#include "stdafx.h"

/*static*/ bool bass_init = false; //!! meh
int bass_BG_idx = -1;
int bass_STD_idx = -1;

/*static*/ float convert2decibelvolume(const float volume) // 0..100 -> DSBVOLUME_MIN..DSBVOLUME_MAX (-10000..0) (db/log scale)
{
   const float totalvolume = max(min(volume, 100.0f), 0.0f);
   const float decibelvolume = (totalvolume == 0.0f) ? DSBVOLUME_MIN : max(logf(totalvolume)*(float)(1000.0 / log(10.0)) - 2000.0f, (float)DSBVOLUME_MIN); // VP legacy conversion
   return decibelvolume;
}

AudioPlayer::AudioPlayer()
{
   m_stream = 0;

#ifdef DEBUG_NO_SOUND
   return;
#endif

   if (!bass_init)
   {
      BASS_INFO info;
      if (BASS_GetInfo(&info)) { // BASS is already in memory and initialized? (However that would've happened is a mystery to me)
          bass_init = true;
          return;
      }

      //

      const SoundConfigTypes SoundMode3D = (SoundConfigTypes)LoadValueIntWithDefault(regKey[RegName::Player], "Sound3D"s, (int)SNDCFG_SND3D2CH);
      const int DS_STD_idx = LoadValueIntWithDefault(regKey[RegName::Player], "SoundDevice"s,   -1);
      const int DS_BG_idx  = LoadValueIntWithDefault(regKey[RegName::Player], "SoundDeviceBG"s, -1);
      bass_STD_idx = -1;
      bass_BG_idx  = -1;

      for(unsigned int idx = 0; idx < 2; ++idx)
      {
      const int DSidx = (idx == 0) ? DS_STD_idx : DS_BG_idx;

      // now match the Direct Sound device with the BASS device (by name)
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
                          if(idx == 0)
                              bass_STD_idx = (dinfo.flags & BASS_DEVICE_DEFAULT) ? -1 : i;
                          else
                              bass_BG_idx  = (dinfo.flags & BASS_DEVICE_DEFAULT) ? -1 : i;
                          break;
                      }
              }

              for (size_t i = 0; i < DSads.size(); i++)
                  delete DSads[i];
          }
      }
      }

      //BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_FLOATDSP, fTrue);

      BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_CURVE_PAN, fTrue); // logarithmic scale, similar to DSound (although BASS still takes a 0..1 range)
      //!! BASS_CONFIG_THREAD so far only works on Net stuff, not these ones here..  :/
      //BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_CURVE_VOL, fTrue); // dto. // is now converted internally, as otherwise PinMAMEs altsound will also get affected! (note that pan is not used yet in PinMAME!)
      BASS_SetConfig(/*BASS_CONFIG_THREAD |*/ BASS_CONFIG_VISTA_SPEAKERS, fTrue); // to make BASS_ChannelSetAttribute(.., BASS_ATTRIB_PAN, pan); work, needs Vista or later

      for(unsigned int idx = 0; idx < 2; ++idx)
      {
      if (!BASS_Init((idx == 0) ? bass_STD_idx : bass_BG_idx, 44100, (SoundMode3D != SNDCFG_SND3D2CH) && (idx == 0) ? 0 /*| BASS_DEVICE_MONO*/ /*| BASS_DEVICE_DSOUND*/ : 0, g_pvp->GetHwnd(), nullptr)) // note that sample rate is usually ignored and set depending on the input/file automatically
      {
         const int code = BASS_ErrorGetCode();
         string bla;
         BASS_ErrorMapCode(code, bla);
         g_pvp->MessageBox(("BASS music/sound library initialization error " + std::to_string(code) + ": " + bla).c_str(), "Error", MB_ICONERROR);
      }
      if (/*SoundMode3D == SNDCFG_SND3D2CH &&*/ bass_STD_idx == bass_BG_idx) // skip 2nd device if it's the same and 3D is disabled //!!! for now try to just use one even if 3D! and then adapt channel settings if sample is a backglass sample
         break;
      }

      bass_init = true;
   }
}

AudioPlayer::~AudioPlayer()
{
   if (m_stream)
   {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      BASS_ChannelStop(m_stream);
      BASS_StreamFree(m_stream);
   }
}

void AudioPlayer::MusicPause()
{
   if (m_stream)
   {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      BASS_ChannelPause(m_stream);
   }
}

void AudioPlayer::MusicUnpause()
{
   if (m_stream)
   {
      if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      BASS_ChannelPlay(m_stream, 0);
   }
}

bool AudioPlayer::MusicActive()
{
   if (m_stream)
   {
      if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      return (BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PLAYING);
   }
   else
      return false;
}

/*void AudioPlayer::MusicEnd()
{
   if (m_stream)
   {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      BASS_ChannelStop(m_stream);
   }
}*/

bool AudioPlayer::MusicInit(const string& szFileName, const float volume)
{
   if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

   for (int i = 0; i < 5; ++i)
   {
      string fileName;
      switch (i)
      {
      case 0: fileName = szFileName; break;
      case 1: fileName = g_pvp->m_szMyPath + "music" + PATH_SEPARATOR_CHAR + szFileName; break;
      case 2: fileName = g_pvp->m_currentTablePath + szFileName; break;
      case 3: fileName = g_pvp->m_currentTablePath + "music" + PATH_SEPARATOR_CHAR + szFileName; break;
      case 4: fileName = PATH_MUSIC + szFileName; break;
      }
      m_stream = BASS_StreamCreateFile(FALSE, fileName.c_str(), 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?
      if (m_stream != 0)
         break;
   }

   if (m_stream == 0)
   {
      const int code = BASS_ErrorGetCode();
      string bla;
      BASS_ErrorMapCode(code, bla);
      g_pvp->MessageBox(("BASS music/sound library cannot load \"" + szFileName + "\" (error " + std::to_string(code) + ": " + bla + ')').c_str(), "Error", MB_ICONERROR);
      return false;
   }

   BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume);
   BASS_ChannelPlay(m_stream, 0);

   return true;
}

void AudioPlayer::MusicVolume(const float volume)
{
   if (m_stream)
   {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);
      BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume);
   }
}
