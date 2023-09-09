#pragma once

#include "inc/bass.h"

class AudioPlayer final
{
public:
   AudioPlayer();
   ~AudioPlayer();

   bool MusicInit(const string& szFileName, const float volume);

   bool MusicActive();

   //void MusicEnd();

   void MusicPause();
   void MusicUnpause();

   void MusicVolume(const float volume);

   //

   bool SetMusicFile(const string& szFileName);
   void MusicPlay();
   void MusicStop();
   void MusicClose();
   double GetMusicPosition();
   void SetMusicPosition(double seconds);
   bool StreamInit(DWORD frequency, int channels, const float volume);
   void StreamUpdate(void* buffer, DWORD length);
   void StreamVolume(const float volume);

private:
   HSTREAM m_stream;
};
