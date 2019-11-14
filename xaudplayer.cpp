#include "StdAfx.h"

static bool bass_init = false; //!! meh

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

      const int DSidx = LoadValueIntWithDefault("Player", "SoundDeviceBG", -1);

      // now match the Direct Sound device with the BASS device (by name)
      int BASSidx = -1;
      if (DSidx != -1)
      {
          DSAudioDevices DSads;
          if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
          {
              if ((size_t)DSidx >= DSads.size() || DSads[DSidx]->guid != NULL) // primary device has guid NULL, so use BASSidx = -1 in that case
              {
                  BASS_DEVICEINFO info;
                  for (int i = 1; BASS_GetDeviceInfo(i, &info); i++) // 0 = no sound/no device
                      if (info.flags & BASS_DEVICE_ENABLED) // device must be enabled
                      if (strcmp(info.name, DSads[DSidx]->description.c_str()) == 0)
                      {
                          BASSidx = i;
                          break;
                      }
              }

              for (size_t i = 0; i < DSads.size(); i++)
                  delete DSads[i];
          }
      }

      if (!BASS_Init(BASSidx, 44100, 0, g_pvp->m_hwnd, NULL)) // note that sample rate is usually ignored and set depending on the input/file automatically
      {
         char bla[128];
         sprintf_s(bla, "BASS music/sound library initialization error %d", BASS_ErrorGetCode());
         MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
      }
      bass_init = true;
   }
}

AudioPlayer::~AudioPlayer()
{
   if (m_stream)
      BASS_StreamFree(m_stream);
}

void AudioPlayer::MusicPause()
{
   if (m_stream)
      BASS_ChannelPause(m_stream);
}

void AudioPlayer::MusicUnpause()
{
   if (m_stream)
      BASS_ChannelPlay(m_stream, 0);
}

bool AudioPlayer::MusicActive()
{
   return m_stream ? (BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PLAYING) : false;
}

void AudioPlayer::MusicEnd()
{
   if (m_stream)
      BASS_ChannelPause(m_stream); //!! ?
}

bool AudioPlayer::MusicInit(const char * const szFileName, const float volume)
{
   m_stream = BASS_StreamCreateFile(FALSE, szFileName, 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?
   if (m_stream == NULL)
   {
      char bla[MAX_PATH];
      sprintf_s(bla, "BASS music/sound library cannot load %s", szFileName);
      MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
      return false;
   }

   BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume);

   BASS_ChannelPlay(m_stream, 0);

   return true;
}

void AudioPlayer::MusicVolume(const float volume)
{
   if (m_stream)
      BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume);
}
