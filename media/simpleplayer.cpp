//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation, 1999 - 1999  All rights reserved.
//
// You have a royalty-free right to use, modify, reproduce and distribute
// the Sample Application Files (including modified versions) in any way 
// you determine to be useful, provided that you agree that Microsoft 
// Corporation provides no warrant or support, and has no obligation or
// liability resulting from the use of any Sample Application Files. 
//

#include "stdafx.h"

#ifdef SUPPORT_DRM
#include "WMAudioDRM.h"
#endif


///////////////////////////////////////////////////////////////////////////////
CSimplePlayer::CSimplePlayer()
{
    m_cRef = 1;
    m_cBuffersOutstanding = 0;

    m_pReader = NULL;
    m_hwo = NULL;

	m_pDSBuffer = NULL;
	m_pDSNotify = NULL;

	m_hNotificationEvent = NULL;

#ifdef SUPPORT_DRM
    m_pDRMCallback = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////////////
CSimplePlayer::~CSimplePlayer()
{
	// Release DirectSound interfaces
	SAFE_RELEASE( m_pDSNotify );
	SAFE_RELEASE( m_pDSBuffer );

	if (m_hNotificationEvent)
		{
		CloseHandle(m_hNotificationEvent);
		}

#ifdef SUPPORT_DRM
    if( NULL != m_pDRMCallback )
    {
        m_pDRMCallback->Release();
        m_pDRMCallback = NULL;
    }
#endif
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::QueryInterface(
    REFIID riid,
    void **ppvObject )
{
    return( E_NOINTERFACE );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CSimplePlayer::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CSimplePlayer::Release()
{
    ULONG uRet = InterlockedDecrement( &m_cRef );

    if( 0 == uRet )
    {
        delete this;
    }

    return uRet;
}

void CSimplePlayer::Stop()
	{
	if (m_pReader)
		{
		m_pReader->Stop();
		m_pReader->Release();
		m_pReader = NULL;
		m_pDSBuffer->Stop();
		}
	}

void CSimplePlayer::Pause()
	{
	if (m_pReader)
		{
		m_pDSBuffer->Stop();
		}
	}

void CSimplePlayer::Unpause()
	{
	if (m_pReader)
		{
		m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}
	}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::OnSample( 
        /* [in] */ const BYTE *pData,
        /* [in] */ DWORD cbData,
        /* [in] */ DWORD dwMsTime )
{
    VOID* pbBuffer  = NULL;
    DWORD dwBufferLength;

    VOID* pbBuffer2  = NULL;
    DWORD dwBufferLength2;

    // Lock the buffer down
	HRESULT hr;
    if( FAILED( hr = m_pDSBuffer->Lock( m_dwNextWriteOffset, cbData/*m_dwNotifySize*/, 
                                        &pbBuffer, &dwBufferLength, &pbBuffer2, &dwBufferLength2, 0L ) ) )
        return hr;

	// Fill the DirectSound buffer with WAV data
	memcpy(pbBuffer, pData, dwBufferLength);

	if (dwBufferLength < cbData)
		{
		memcpy(pbBuffer2, &pData[dwBufferLength], dwBufferLength2);
		}

    // Now unlock the buffer
    m_pDSBuffer->Unlock( pbBuffer,dwBufferLength, pbBuffer2, dwBufferLength2);
    pbBuffer = NULL;

    m_dwNextWriteOffset += cbData;//dwBufferLength; 
    m_dwNextWriteOffset %= m_dwBufferSize; // Circular buffer

	if (!fStartedPlay)
		{
		fStartedPlay = true;
		m_pDSBuffer->SetCurrentPosition(0);
		hr = m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}

    return S_OK;

    InterlockedIncrement( &m_cBuffersOutstanding ); //!! ??

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::Play(LPCWSTR pszUrl, HANDLE hCompletionEvent, HRESULT *phrCompletion, int volume)
{
    HRESULT hr;

	if (g_AudioCreateReaderFunc)
		{
		hr = (g_AudioCreateReaderFunc)(pszUrl, this, &m_pReader, NULL );
		}
	else
		{
		return E_FAIL;
		}
    
    if( FAILED( hr ) )
    {
        //printf( "failed to create audio reader (hr=%#X)\n", hr );
        return hr;
    }

    //
    // Set up for audio playback
    //
    hr = m_pReader->GetOutputFormat( &m_wfx, sizeof( m_WfxBuf ) );

    if( FAILED( hr ) )
    {
        //printf( "failed GetOutputFormat(), hr=0x%lX\n", hr );
        return hr;
    }

	CreateStreamingBuffer(&m_wfx);

	m_pDSBuffer->SetVolume(volume);

	fStartedPlay = false;

    //
    // Start reading the data (and rendering the audio)
    //
    hr = m_pReader->Start( );

    if( FAILED( hr ) )
    {
        //printf( "failed Start(), hr=0x%lX\n", hr );
        return hr;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::OnStatus( 
        /* [in] */ WMT_STATUS Status, 
        /* [in] */ HRESULT hr,
        /* [in] */ const VARIANT *pParam )
{
    switch( Status )
    {
    case WMT_ERROR:
        //printf( "OnStatus( WMT_ERROR )\n" );
        break;

    case WMT_BUFFERING_START:
        //printf( "OnStatus( WMT_BUFFERING START)\n" );
        break;

    case WMT_BUFFERING_STOP:
        //printf( "OnStatus( WMT_BUFFERING STOP)\n" );
        break;

    case WMT_EOF:
		{
        //printf( "OnStatus( WMT_EOF )\n" );

        //
        // cleanup and exit
        //

		// Create a notification event, for when the sound gets to the end
		// of the last buffered sequence
		if( SUCCEEDED( hr = m_pDSBuffer->QueryInterface( IID_IDirectSoundNotify, 
													  (VOID**)&m_pDSNotify ) ) )
			{
			m_pDSBuffer->Stop();
			m_aPosNotify.dwOffset     = m_dwNextWriteOffset;
			m_aPosNotify.hEventNotify = m_hNotificationEvent;

			if( SUCCEEDED(hr = m_pDSNotify->SetNotificationPositions(1, 
				&m_aPosNotify) ) )
				{
				m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
				// time out after length of buffer, since obviously we'd be done by then
				/*const DWORD foo =*/ WaitForSingleObjectEx(m_hNotificationEvent, 3000, FALSE);
				}
			}

		m_pDSBuffer->Stop();		

		IWMAudioReader *pReaderT = m_pReader;
		m_pReader = NULL;
		// This release line kills this thread, so it never runs anything after it
		//pReaderT->Stop();
		pReaderT->Release();

        /*if( 0 == m_cBuffersOutstanding )
        {
            //SetEvent( m_hCompletionEvent );
        }*/
		}
        break;

    case WMT_LOCATING:
        //printf( "OnStatus( WMT_LOCATING )\n" );
        break;

    case WMT_CONNECTING:
        //printf( "OnStatus( WMT_CONNECTING )\n" );
        break;

    case WMT_MISSING_CODEC:
		{
        //
        // find the guid for the missing codec
        //
        GUID    guidCodecID;

        hr = CLSIDFromString( pParam->bstrVal, &guidCodecID );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = DoCodecDownload( &guidCodecID );
        if( FAILED( hr ) )
        {
            return hr;
        }

        break;
		}
    }

    return( S_OK );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::DoCodecDownload( GUID* pguidCodecID )
{
	return S_OK;
}

HRESULT CSimplePlayer::CreateStreamingBuffer(WAVEFORMATEX *pwfx)
{
    // This samples works by dividing a 3 second streaming buffer into 
    // NUM_PLAY_NOTIFICATIONS (or 16) pieces.  it creates a notification for each
    // piece and when a notification arrives then it fills the circular streaming 
    // buffer with new wav data over the sound data which was just played

    // The size of wave data is in pWaveFileSound->m_ckIn
    DWORD nBlockAlign = pwfx->nBlockAlign;
    INT nSamplesPerSec = pwfx->nSamplesPerSec;

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
    dsbd.dwFlags       = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME;
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
