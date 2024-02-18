#include "core/stdafx.h"

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

      const SoundConfigTypes SoundMode3D = (SoundConfigTypes)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Sound3D"s, (int)SNDCFG_SND3D2CH);
      const int DS_STD_idx = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundDevice"s,   -1);
      const int DS_BG_idx  = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundDeviceBG"s, -1);
#ifndef __STANDALONE__
      bass_STD_idx = -1;
      bass_BG_idx  = -1;
#else
      bass_STD_idx = DS_STD_idx;
      bass_BG_idx  = DS_BG_idx;
#endif

#ifndef __STANDALONE__
      for(unsigned int idx = 0; idx < 2; ++idx)
      {
      const int DSidx = (idx == 0) ? DS_STD_idx : DS_BG_idx;

      // now match the Direct Sound device with the BASS device (by name)
      if (DSidx != -1)
      {
          DSAudioDevices DSads;
          if (SUCCEEDED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
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
#endif

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

#ifndef __STANDALONE__
   const string& filename = szFileName;
#else
   const string filename = normalize_path_separators(szFileName);
#endif

   for (int i = 0; i < 5; ++i)
   {
      string path;
      switch (i)
      {
      case 0: path = filename; break;
      case 1: path = g_pvp->m_szMyPath + "music" + PATH_SEPARATOR_CHAR + filename; break;
      case 2: path = g_pvp->m_currentTablePath + filename; break;
      case 3: path = g_pvp->m_currentTablePath + "music" + PATH_SEPARATOR_CHAR + filename; break;
      case 4: path = PATH_MUSIC + filename; break;
      }
      m_stream = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?
      if (m_stream != 0)
         break;
   }

   if (m_stream == 0)
   {
      const int code = BASS_ErrorGetCode();
      string bla;
      BASS_ErrorMapCode(code, bla);
      g_pvp->MessageBox(("BASS music/sound library cannot load \"" + filename + "\" (error " + std::to_string(code) + ": " + bla + ')').c_str(), "Error", MB_ICONERROR);
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

bool AudioPlayer::SetMusicFile(const string& szFileName)
{
   if (m_stream)
      MusicClose();

   m_stream = BASS_StreamCreateFile(FALSE, szFileName.c_str(), 0, 0, 0);

   if (m_stream == 0) {
      const int code = BASS_ErrorGetCode();
      string message;
      BASS_ErrorMapCode(code, message);
      g_pvp->MessageBox(("BASS music/sound library cannot load \"" + szFileName + "\" (error " + std::to_string(code) + ": " + message + ')').c_str(), "Error", MB_ICONERROR);
      return false;
   }

   return true;
}

void AudioPlayer::MusicPlay()
{
   if (m_stream) {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

      BASS_ChannelPlay(m_stream, 0);
   }
}

void AudioPlayer::MusicStop()
{
   if (m_stream) {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

      BASS_ChannelStop(m_stream);
   }
}

void AudioPlayer::MusicClose()
{
   if (m_stream) {
      if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

      BASS_ChannelStop(m_stream);
      BASS_StreamFree(m_stream);

      m_stream = 0;
   }
}

double AudioPlayer::GetMusicPosition()
{
   if (m_stream) {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

      return BASS_ChannelBytes2Seconds(m_stream, BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE));
   }

   return -1;
}

void AudioPlayer::SetMusicPosition(double seconds)
{
   if (m_stream) {
      if(bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

      BASS_ChannelSetPosition(m_stream, BASS_ChannelSeconds2Bytes(m_stream, seconds), BASS_POS_BYTE);
   }
}

bool AudioPlayer::StreamInit(DWORD frequency, int channels, const float volume)
{
   if (bass_BG_idx != -1 && bass_STD_idx != bass_BG_idx) BASS_SetDevice(bass_BG_idx);

   m_stream = BASS_StreamCreate( frequency, channels, 0, STREAMPROC_PUSH, 0 );

   if (m_stream == 0) {
      const int code = BASS_ErrorGetCode();
      string message;
      BASS_ErrorMapCode(code, message);
      g_pvp->MessageBox(("BASS music/sound library cannot play stream (error " + std::to_string(code) + ": " + message + ')').c_str(), "Error", MB_ICONERROR);
      return false;
   }

   BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume);
   BASS_ChannelPlay(m_stream, 0);

   return true;
}

void AudioPlayer::StreamUpdate(void* buffer, DWORD length) 
{
   if (m_stream)
      BASS_StreamPutData(m_stream, buffer, length);
}

void AudioPlayer::StreamVolume(const float volume)
{
   MusicVolume(volume);
}
