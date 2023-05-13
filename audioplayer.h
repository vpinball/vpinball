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

private:
   HSTREAM m_stream;
};
