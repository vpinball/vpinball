#include "StdAfx.h"

XAudPlayer::XAudPlayer()
	{
	m_pDSBuffer = NULL;
	m_pDSNotify = NULL;
	m_decoder = NULL;
	m_hNotificationEvent = NULL;
	m_fEndData = false;
	m_lastplaypos = 0;
	}

XAudPlayer::~XAudPlayer()
	{
	SAFE_RELEASE(m_pDSBuffer);
	SAFE_RELEASE(m_pDSNotify);

	if (m_hNotificationEvent)
		{
		CloseHandle(m_hNotificationEvent);
		}

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

	char buf[10000];
    unsigned char mp3_buffer[4096];

	int status = decoder_decode(m_decoder, buf);

	if (status == XA_ERROR_TIMEOUT) // Need more input
		{
		const int nb_read = fread(mp3_buffer, 1, 4096, file);

		if (nb_read == 0)
			{
			if (feof(file))
				{
				fprintf(stderr, "end of file\n");
				status = 0xffff;
				}
			else
				{
				fprintf(stderr, "cannot read bytes from input\n");
				status = 0xffff;
				}
			}

		if (status != 0xffff)
			{
			/* feed the input buffer */
			decoder_input_send_message(m_decoder, XA_MEMORY_INPUT_MESSAGE_FEED, 
									   mp3_buffer, nb_read);

			
			status = decoder_decode(m_decoder, buf);
			}
		}

	if (status == XA_SUCCESS)
		{
		/*HRESULT hr =*/ m_pDSBuffer->SetFrequency(m_decoder->status->info.frequency);

		const unsigned int cbData = m_decoder->output_buffer->nb_samples*m_decoder->output_buffer->bytes_per_sample*m_decoder->output_buffer->nb_channels;

		VOID* pbBuffer  = NULL;
		DWORD dwBufferLength;

		VOID* pbBuffer2  = NULL;
		DWORD dwBufferLength2;

		HRESULT hr;
		if( FAILED( hr = m_pDSBuffer->Lock(m_dwNextWriteOffset, cbData/*m_dwNotifySize*/, 
											&pbBuffer, &dwBufferLength, &pbBuffer2, &dwBufferLength2, 0L) ) )
			return 0;

		memcpy(pbBuffer, buf, dwBufferLength);

		if (dwBufferLength < cbData)
			{
			memcpy(pbBuffer2, &buf[dwBufferLength], dwBufferLength2);
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
		return 1;
		}

	return 1;
	}

void XAudPlayer::End()
	{
	m_pDSBuffer->Stop();
	}

int XAudPlayer::Init(char *szFileName, int volume)
	{
	m_fStarted = false;

    /* open the mp3 file (name passed as program argument */
    if (fopen_s(&file, szFileName, "rb")) {
        fprintf(stderr, "cannot open input file\n");
        return 0;
    }
    
    /* create a decoder */
    if (decoder_new(&m_decoder) != XA_SUCCESS) {
        fprintf(stderr, "cannot create decoder");
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

	CreateBuffer(volume);//

    return 1;
	}

HRESULT XAudPlayer::CreateBuffer(int volume)
	{
	/*const int status =*/ decoder_decode(m_decoder, NULL);
	
	WAVEFORMATEX wfex;
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = 2;//m_decoder->output_buffer->nb_channels;
	wfex.nSamplesPerSec = 22050;//44100; Bogus frequency value - the real value gets set as data is decompressed - look for SetFrequency
	wfex.wBitsPerSample = 16;//m_decoder->output_buffer->bytes_per_sample;
	wfex.cbSize = 0;
	wfex.nBlockAlign = (wfex.nChannels * wfex.wBitsPerSample) / (WORD)8;
	wfex.nAvgBytesPerSec = wfex.nBlockAlign * wfex.nSamplesPerSec;

	CreateStreamingBuffer(&wfex);

	m_pDSBuffer->SetCurrentPosition(0);

	m_pDSBuffer->SetVolume(volume);

	return S_OK;
	}

#include "Mmsystem.h"

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
    m_dwNotifySize = nSamplesPerSec * 3 * nBlockAlign;// / NUM_PLAY_NOTIFICATIONS;
    m_dwNotifySize -= m_dwNotifySize % nBlockAlign;

    // The buffersize should approximately 3 seconds of wav data
    m_dwBufferSize  = m_dwNotifySize;// * NUM_PLAY_NOTIFICATIONS;
    m_dwProgress    = 0;
    m_dwLastPos     = 0;

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
    if( FAILED( hr = g_pvp->m_pds.m_pDS->CreateSoundBuffer( &dsbd, &m_pDSBuffer, NULL ) ) )
        return hr;

	m_dwNextWriteOffset = 0; 
    m_dwProgress = 0;
    m_dwLastPos  = 0;

	m_pDSBuffer->SetCurrentPosition(0);

	m_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    return S_OK;
}
