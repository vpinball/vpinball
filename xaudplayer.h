#pragma once

#define NO_XAUDIO

#ifndef NO_XAUDIO
/* xaudio general includes */
#include "inc\xaudio\decoder.h"
#else
#include "inc\bass.h"
#endif

class XAudPlayer
{
public:
   XAudPlayer();
   ~XAudPlayer();

   int Init(char * const szFileName, const float volume);

   int Tick();

   void End();

   void Pause();
   void Unpause();

   void Volume(const float volume);

private:

#ifndef NO_XAUDIO
   XA_DecoderInfo *m_decoder;

   FILE *file;

   int m_cDataLeft;
   int m_lastplaypos;

   bool m_fStarted;
   bool m_fEndData; // all data has been decoded - wait for buffer to play

   HRESULT CreateBuffer(const int volume);
   HRESULT CreateStreamingBuffer(WAVEFORMATEX *pwfx);

   //#define NUM_PLAY_NOTIFICATIONS  16

   //LPDIRECTSOUND       m_pDS            = NULL;
   LPDIRECTSOUNDBUFFER m_pDSBuffer;
   //LPDIRECTSOUNDNOTIFY m_pDSNotify;

   //DSBPOSITIONNOTIFY   m_aPosNotify;  
   //HANDLE              m_hNotificationEvent;

   DWORD               m_dwBufferSize;
   DWORD               m_dwNextWriteOffset;
#else
   HSTREAM				m_stream;
#endif
};
