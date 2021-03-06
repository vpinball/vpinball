#include "StdAfx.h"

/*static*/ bool bass_init = false; //!! meh
int bass_BG_idx = -1;
int bass_STD_idx = -1;

AudioPlayer::AudioPlayer()
{
   m_stream = NULL;

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

      const SoundConfigTypes SoundMode3D = (SoundConfigTypes)LoadValueIntWithDefault("Player", "Sound3D", (int)SNDCFG_SND3D2CH);
      const int DS_STD_idx = LoadValueIntWithDefault("Player", "SoundDevice",   -1);
      const int DS_BG_idx  = LoadValueIntWithDefault("Player", "SoundDeviceBG", -1);
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
              if ((size_t)DSidx >= DSads.size() || DSads[DSidx]->guid != NULL) // primary device has guid NULL, so use BASS_idx = -1 in that case
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

      //BASS_SetConfig(BASS_CONFIG_FLOATDSP, fTrue);

      for(unsigned int idx = 0; idx < 2; ++idx)
      {
      if (!BASS_Init((idx == 0) ? bass_STD_idx : bass_BG_idx, 44100, (SoundMode3D != SNDCFG_SND3D2CH) && (idx == 0) ? BASS_DEVICE_3D : 0, g_pvp->GetHwnd(), NULL)) // note that sample rate is usually ignored and set depending on the input/file automatically
      {
         const int code = BASS_ErrorGetCode();
         string bla2;
         BASS_ErrorMapCode(code, bla2);
         const string bla = "BASS music/sound library initialization error " + std::to_string(code) + ": " + bla2;
         g_pvp->MessageBox(bla.c_str(), "Error", MB_ICONERROR);
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

bool AudioPlayer::MusicInit(const string& szFileName, const string& alt_szFileName, const float volume)
{
   if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

   m_stream = BASS_StreamCreateFile(FALSE, szFileName.c_str(), 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?
   if (m_stream == NULL)
      m_stream = BASS_StreamCreateFile(FALSE, alt_szFileName.c_str(), 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?

   if (m_stream == NULL)
   {
      const int code = BASS_ErrorGetCode();
      string bla2;
      BASS_ErrorMapCode(code, bla2);
      const string bla = "BASS music/sound library cannot load \"" + szFileName + "\" (error " + std::to_string(code) + ": " + bla2 + ")";
      g_pvp->MessageBox(bla.c_str(), "Error", MB_ICONERROR);
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
