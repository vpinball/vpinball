#include "StdAfx.h"

#ifndef NO_XAUDIO

#include "Mmsystem.h"

/* xaudio plug-in modules */
#include "inc\xaudio\memory_input.h"
#include "inc\xaudio\audio_output.h"
#include "inc\xaudio\mpeg_codec.h"

XAudPlayer::XAudPlayer()
{
   m_pDSBuffer = NULL;
   //m_pDSNotify = NULL;
   m_decoder = NULL;
   //m_hNotificationEvent = NULL;
   m_fEndData = false;
   m_lastplaypos = 0;
}

XAudPlayer::~XAudPlayer()
{
   SAFE_RELEASE(m_pDSBuffer);
   //SAFE_RELEASE(m_pDSNotify);

   /*if (m_hNotificationEvent)
      {
      CloseHandle(m_hNotificationEvent);
      }*/

   if (m_decoder)
   {
      decoder_delete(m_decoder);
   }
}

void XAudPlayer::Pause()
{
   if (m_fStarted)
   {
      m_pDSBuffer->Stop();
   }
}

void XAudPlayer::Unpause()
{
   if (m_fStarted)
   {
      m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
   }
}

int XAudPlayer::Tick()
{
   DWORD playpos, writepos;
   m_pDSBuffer->GetCurrentPosition(&playpos, &writepos);

   if (m_fEndData)
   {
      int timeplayed = playpos - m_lastplaypos;
      if (timeplayed < 0)
      {
         timeplayed += m_dwBufferSize;
      }

      m_cDataLeft -= timeplayed;

      if (m_cDataLeft < 0)
      {
         End();
         return 0;
      }

      m_lastplaypos = playpos;

      return 1;
   }

   const DWORD halfaround = (playpos + m_dwBufferSize/2);// % m_dwBufferSize;

   if (halfaround > m_dwBufferSize)
   {
      if (m_dwNextWriteOffset < playpos && (m_dwNextWriteOffset > (halfaround % m_dwBufferSize)))
      {
         return 1;
      }
   }
   else if (m_dwNextWriteOffset < playpos || m_dwNextWriteOffset > halfaround)
   {
      return 1;
   }

   int status = decoder_decode(m_decoder, NULL);

   if (status == XA_ERROR_TIMEOUT) // Need more input
   {
      unsigned char mp3_buffer[4096];
      const int nb_read = fread(mp3_buffer, 1, 4096, file);

      if (nb_read == 0)
      {
         //if (feof(file))
         //	{
         //fprintf(stderr, "end of file\n");
         status = 0xffff;
         //	}
         //else
         //	{
         //fprintf(stderr, "cannot read bytes from input\n");
         //status = 0xffff;
         //	}
      }
      else
      {
         /* feed the input buffer */
         decoder_input_send_message(m_decoder, XA_MEMORY_INPUT_MESSAGE_FEED, 
            mp3_buffer, nb_read);

         status = decoder_decode(m_decoder, NULL);
      }
   }

   if (status == XA_SUCCESS)
   {
      /*HRESULT hr =*/ m_pDSBuffer->SetFrequency(m_decoder->status->info.frequency);

      const unsigned int cbData = m_decoder->output_buffer->nb_samples*m_decoder->output_buffer->bytes_per_sample*m_decoder->output_buffer->nb_channels;

      VOID* pbBuffer  = NULL;
      DWORD dwBufferLength;

      VOID* pbBuffer2 = NULL;
      DWORD dwBufferLength2;

      HRESULT hr;
      if( FAILED( hr = m_pDSBuffer->Lock(m_dwNextWriteOffset, cbData, 
         &pbBuffer, &dwBufferLength, &pbBuffer2, &dwBufferLength2, 0L) ) )
         return 0;

      memcpy(pbBuffer, m_decoder->output_buffer->pcm_samples, dwBufferLength);

      if (dwBufferLength < cbData)
      {
         memcpy(pbBuffer2, ((char*)(m_decoder->output_buffer->pcm_samples))+dwBufferLength, dwBufferLength2);
      }

      m_pDSBuffer->Unlock(pbBuffer,dwBufferLength, pbBuffer2, dwBufferLength2);
      pbBuffer = NULL;

      m_dwNextWriteOffset += cbData;
      m_dwNextWriteOffset %= m_dwBufferSize;

      if (!m_fStarted)
      {
         m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
         m_fStarted = true;
      }
   }
   else
   {
      m_fEndData = true;
      m_cDataLeft = m_dwNextWriteOffset - playpos;
      if (m_cDataLeft < 0)
      {
         m_cDataLeft += m_dwBufferSize;
      }
      m_lastplaypos = playpos;
      //End();
   }

   return 1;
}

void XAudPlayer::End()
{
   m_pDSBuffer->Stop();
}

int XAudPlayer::Init(char * const szFileName, const int volume)
{
   m_fStarted = false;

   /* open the mp3 file (name passed as program argument */
   if (fopen_s(&file, szFileName, "rb")) {
      //fprintf(stderr, "cannot open input file\n");
      return 0;
   }

   /* create a decoder */
   if (decoder_new(&m_decoder) != XA_SUCCESS) {
      //fprintf(stderr, "cannot create decoder");
      return 0;
   }

   /* register mpeg audio codec */
    {
       XA_CodecModule module;

       mpeg_codec_module_register(&module);
       decoder_codec_module_register(m_decoder, &module);
    }

    /* register the memory input module */
    {
       XA_InputModule module;

       memory_input_module_register(&module);
       decoder_input_module_register(m_decoder, &module);
    }

    /* create and open input object */
    const int status = decoder_input_new(m_decoder, NULL, XA_DECODER_INPUT_AUTOSELECT);
    if (status != XA_SUCCESS) 
    {
       //error("cannot create input [%d]\n", status);
    }
    if (decoder_input_open(m_decoder) != XA_SUCCESS) {
       printf("cannot open input\n");
       return 0;
    }

    CreateBuffer(volume);

    return 1;
}

HRESULT XAudPlayer::CreateBuffer(const int volume)
{
   /*const int status =*/ decoder_decode(m_decoder, NULL);

   WAVEFORMATEX wfex;
   wfex.wFormatTag = WAVE_FORMAT_PCM;
   wfex.nChannels = 2;//m_decoder->output_buffer->nb_channels;
   wfex.nSamplesPerSec = 44100; //Bogus frequency value - the real value gets set as data is decompressed - look for SetFrequency
   wfex.wBitsPerSample = 16;//m_decoder->output_buffer->bytes_per_sample;
   wfex.cbSize = 0;
   wfex.nBlockAlign = (wfex.nChannels * wfex.wBitsPerSample) / (WORD)8;
   wfex.nAvgBytesPerSec = wfex.nBlockAlign * wfex.nSamplesPerSec;

   CreateStreamingBuffer(&wfex);

   m_pDSBuffer->SetCurrentPosition(0);

   m_pDSBuffer->SetVolume(volume);

   return S_OK;
}

HRESULT XAudPlayer::CreateStreamingBuffer(WAVEFORMATEX *pwfx)
{
   // This samples works by dividing a 3 second streaming buffer into 
   // NUM_PLAY_NOTIFICATIONS (or 16) pieces.  it creates a notification for each
   // piece and when a notification arrives then it fills the circular streaming 
   // buffer with new wav data over the sound data which was just played

   // The size of wave data is in pWaveFileSound->m_ckIn
   const DWORD nBlockAlign = pwfx->nBlockAlign;
   const INT nSamplesPerSec = pwfx->nSamplesPerSec;

   // The g_dwNotifySize should be an integer multiple of nBlockAlign
   DWORD dwNotifySize = nSamplesPerSec * 3 * nBlockAlign;// / NUM_PLAY_NOTIFICATIONS;
   dwNotifySize -= dwNotifySize % nBlockAlign;

   // The buffersize should approximately 3 seconds of wav data
   m_dwBufferSize  = dwNotifySize;// * NUM_PLAY_NOTIFICATIONS;

   // Set up the direct sound buffer, and only request the flags needed
   // since each requires some overhead and limits if the buffer can 
   // be hardware accelerated
   DSBUFFERDESC dsbd;
   ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
   dsbd.dwSize        = sizeof(DSBUFFERDESC);
   dsbd.dwFlags       = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
   dsbd.dwBufferBytes = m_dwBufferSize;
   dsbd.lpwfxFormat   = pwfx;

   // Create a DirectSound buffer
   HRESULT hr;
   if( FAILED( hr = g_pvp->m_pbackglassds->m_pDS->CreateSoundBuffer( &dsbd, &m_pDSBuffer, NULL ) ) )
      return hr;

   m_dwNextWriteOffset = 0; 

   m_pDSBuffer->SetCurrentPosition(0);

   //m_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

   return S_OK;
}

#else // no XAudio

static bool bass_init = false; //!! meh

XAudPlayer::XAudPlayer()
{
   m_stream = NULL;

#ifdef DEBUG_NO_SOUND
   return;
#endif

   if (!bass_init)
   {
      int DSidx = -1;
      const HRESULT hr = GetRegInt("Player", "SoundDeviceBG", &DSidx);
      if (hr == S_OK && DSidx != -1)
         DSidx++; // as 0 is nosound //!! mapping is otherwise the same or not?!
      else
         DSidx = -1;
      if (!BASS_Init(DSidx, 44100, 0, g_pvp->m_hwnd, NULL))
      {
         char bla[128];
         sprintf_s(bla, "BASS music/sound library initialization error %d", BASS_ErrorGetCode());
         MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
      }
      bass_init = true;
   }
}

XAudPlayer::~XAudPlayer()
{
   if (m_stream)
      BASS_StreamFree(m_stream);
}

void XAudPlayer::Pause()
{
   if (m_stream)
      BASS_ChannelPause(m_stream);
}

void XAudPlayer::Unpause()
{
   if (m_stream)
      BASS_ChannelPlay(m_stream, 0);
}

int XAudPlayer::Tick()
{
   if (m_stream)
      return (BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PLAYING);
   else
      return 0;
}

void XAudPlayer::End()
{
   if (m_stream)
      BASS_ChannelPause(m_stream); //!! ?
}

int XAudPlayer::Init(char * const szFileName, const float volume)
{
   m_stream = BASS_StreamCreateFile(FALSE, szFileName, 0, 0, /*BASS_SAMPLE_LOOP*/0); //!! ?
   if (m_stream == NULL)
   {
      char bla[128];
      sprintf_s(bla, "BASS music/sound library cannot load %s", szFileName);
      MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
      return 0;
   }

   BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume*(float)(1.0/100.0));

   BASS_ChannelPlay(m_stream, 0);

   return 1;
}

void XAudPlayer::Volume(const float volume)
{
   if (m_stream)
      BASS_ChannelSetAttribute(m_stream, BASS_ATTRIB_VOL, volume*(float)(1.0/100.0));
}

#endif // NO_XAUDIO
