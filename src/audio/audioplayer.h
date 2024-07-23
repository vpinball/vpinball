#pragma once

#ifndef __STANDALONE__
#include "bass.h"
#else
#pragma push_macro("WINAPI")
#pragma push_macro("CALLBACK")
#pragma push_macro("LOBYTE")
#pragma push_macro("HIBYTE")
#pragma push_macro("LOWORD")
#pragma push_macro("HIWORD")
#pragma push_macro("MAKEWORD")
#pragma push_macro("MAKELONG")
#undef WINAPI
#undef CALLBACK
#undef LOBYTE
#undef HIBYTE
#undef LOWORD
#undef HIWORD
#undef MAKEWORD
#undef MAKELONG
#include "bass.h"
#pragma pop_macro("WINAPI")
#pragma pop_macro("CALLBACK")
#pragma pop_macro("LOBYTE")
#pragma pop_macro("HIBYTE")
#pragma pop_macro("LOWORD")
#pragma pop_macro("HIWORD")
#pragma pop_macro("MAKEWORD")
#pragma pop_macro("MAKELONG")
#endif

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
