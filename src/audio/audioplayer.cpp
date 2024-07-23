#include "stdafx.h"

/*static*/ float convert2decibelvolume(const float volume) // 0..100 -> DSBVOLUME_MIN..DSBVOLUME_MAX (-10000..0) (db/log scale)
{
   const float totalvolume = max(min(volume, 100.0f), 0.0f);
   const float decibelvolume = (totalvolume == 0.0f) ? DSBVOLUME_MIN : max(logf(totalvolume)*(float)(1000.0 / log(10.0)) - 2000.0f, (float)DSBVOLUME_MIN); // VP legacy conversion
   return decibelvolume;
}

AudioPlayer::AudioPlayer()
{
   m_stream = 0;
}

AudioPlayer::~AudioPlayer()
{
   if (m_stream)
   {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
      BASS_ChannelStop(m_stream);
      BASS_StreamFree(m_stream);
   }
}

void AudioPlayer::MusicPause()
{
   if (m_stream)
   {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
      BASS_ChannelPause(m_stream);
   }
}

void AudioPlayer::MusicUnpause()
{
   if (m_stream)
   {
      if (g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
      BASS_ChannelPlay(m_stream, 0);
   }
}

bool AudioPlayer::MusicActive()
{
   if (m_stream)
   {
      if (g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
      return (BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PLAYING);
   }
   else
      return false;
}

/*void AudioPlayer::MusicEnd()
{
   if (m_stream)
   {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
      BASS_ChannelStop(m_stream);
   }
}*/

bool AudioPlayer::MusicInit(const string& szFileName, const float volume)
{
   if (g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

#ifndef __STANDALONE__
   string filename = szFileName;
#else
   string filename = normalize_path_separators(szFileName);
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
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);
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
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

      BASS_ChannelPlay(m_stream, 0);
   }
}

void AudioPlayer::MusicStop()
{
   if (m_stream) {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

      BASS_ChannelStop(m_stream);
   }
}

void AudioPlayer::MusicClose()
{
   if (m_stream) {
      if (g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

      BASS_ChannelStop(m_stream);
      BASS_StreamFree(m_stream);

      m_stream = 0;
   }
}

double AudioPlayer::GetMusicPosition()
{
   if (m_stream) {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

      return BASS_ChannelBytes2Seconds(m_stream, BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE));
   }

   return -1;
}

void AudioPlayer::SetMusicPosition(double seconds)
{
   if (m_stream) {
      if(g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

      BASS_ChannelSetPosition(m_stream, BASS_ChannelSeconds2Bytes(m_stream, seconds), BASS_POS_BYTE);
   }
}

bool AudioPlayer::StreamInit(DWORD frequency, int channels, const float volume)
{
   if (g_pvp->m_ps.bass_BG_idx != -1 && g_pvp->m_ps.bass_STD_idx != g_pvp->m_ps.bass_BG_idx) BASS_SetDevice(g_pvp->m_ps.bass_BG_idx);

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
