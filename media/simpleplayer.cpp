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
#include "..\main.h"

//#include <shellapi.h>
//#include "SimplePlayer.h"
//#include "URLLaunch.h"
//#include "codecdownload.h"

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

    m_fEof = FALSE;
    //m_pszUrl = NULL;

	m_pDSBuffer = NULL;
	m_pDSNotify = NULL;

	m_hNotificationEvent = NULL;
	//m_pWaveSoundRead = NULL;

#ifdef SUPPORT_DRM
    m_pDRMCallback = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////////////
CSimplePlayer::~CSimplePlayer()
{
    //_AssertE( 0 == m_cBuffersOutstanding );

    /*if( m_pReader != NULL )
    {
		_ASSERTE(m_cRef == 0);

        m_pReader->Release();
        m_pReader = NULL;
    }*/

	{
		/*if( NULL != m_pWaveSoundRead )
		{
			m_pWaveSoundRead->Close();
			SAFE_DELETE( g_pWaveSoundRead );
		}*/

		// Release DirectSound interfaces
		SAFE_RELEASE( m_pDSNotify );
		SAFE_RELEASE( m_pDSBuffer );
		//SAFE_RELEASE( m_pDS );
	}

    /*if( m_hwo != NULL )
    {
        waveOutClose( m_hwo );
    }*/

	/*if (m_pszUrl)
		{
		delete [] m_pszUrl;
		}*/

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

    return( uRet );
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
    HRESULT hr = S_OK;

	//printf(" New Sample of length %d and PS time %d\n", cbData, dwMsTime);

    /*LPWAVEHDR pwh = (LPWAVEHDR) new BYTE[ sizeof( WAVEHDR ) + cbData ];

    if( NULL == pwh )
    {
        printf( "OnSample OUT OF MEMORY! \n" );

        *m_phrCompletion = E_OUTOFMEMORY;
        //SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }*/

    /*pwh->lpData = (LPSTR)&pwh[1];
    pwh->dwBufferLength = cbData;
    pwh->dwBytesRecorded = cbData;
    pwh->dwUser = 0;
    pwh->dwLoops = 0;
    pwh->dwFlags = 0;

    CopyMemory( pwh->lpData, pData, cbData );*/

	
	   
	   
	
	//HRESULT hr;
    VOID* pbBuffer  = NULL;
    DWORD dwBufferLength;

    VOID* pbBuffer2  = NULL;
    DWORD dwBufferLength2;

    //UpdateProgress();

    // Lock the buffer down
    if( FAILED( hr = m_pDSBuffer->Lock( m_dwNextWriteOffset, cbData/*m_dwNotifySize*/, 
                                        &pbBuffer, &dwBufferLength, &pbBuffer2, &dwBufferLength2, 0L ) ) )
        return hr;

    // Fill the buffer with wav data 
    //if( FAILED( hr = WriteToBuffer( bLooped, pbBuffer, dwBufferLength ) ) )
        //return hr;

	// Fill the DirectSound buffer with WAV data
	CopyMemory(pbBuffer, pData, dwBufferLength);

	if (dwBufferLength < cbData)
		{
		CopyMemory(pbBuffer2, &pData[dwBufferLength], dwBufferLength2);
		}

    // Now unlock the buffer
    m_pDSBuffer->Unlock( pbBuffer,dwBufferLength, pbBuffer2, dwBufferLength2);
    pbBuffer = NULL;

	//m_pDSBuffer->SetCurrentPosition(0);

    // If the end was found, detrimine if there's more data to play 
    // and if not, stop playing
    /*if( m_bFoundEnd )
    {
        // We don't want to cut off the sound before it's done playing.
        // if doneplaying is set, the next notification event will post a stop message.
        if( m_pWaveSoundRead->m_ckInRiff.cksize > g_dwNotifySize )
        {
            if( g_dwProgress >= g_pWaveSoundRead->m_ckInRiff.cksize - g_dwNotifySize )
            {
                g_pDSBuffer->Stop();
            }
        }
        else // For short files.
        {
            if( g_dwProgress >= g_pWaveSoundRead->m_ckInRiff.cksize )
            {
                g_pDSBuffer->Stop();
            }
        }
    }*/

    m_dwNextWriteOffset += cbData;//dwBufferLength; 
    m_dwNextWriteOffset %= m_dwBufferSize; // Circular buffer

	/*HRESULT hrfoo = m_pDSBuffer->SetVolume(-10000);
	
	if (hrfoo != S_OK)
		{
		HDC hdcnull = GetDC(NULL);
		char szFoo[1024];
		int len = sprintf(szFoo, "%d %d", hrfoo, DSERR_CONTROLUNAVAIL);
		TextOut(hdcnull, 10, 10, szFoo, len);
		//PatBlt(hdcnull, 0,0,20,20,PATCOPY);
		ReleaseDC(NULL, hdcnull);
		}*/

	if (!fStartedPlay)
		{
		fStartedPlay = fTrue;
		m_pDSBuffer->SetCurrentPosition(0);
		hr = m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
		/*if( FAILED( hr = m_pDSBuffer->Play( 0, 0, DSBPLAY_LOOPING ) ) )
				return hr;*/
		}

    return S_OK;

    /*MMRESULT mmr;

    mmr = waveOutPrepareHeader( m_hwo, pwh, sizeof(WAVEHDR) );

    if( mmr != MMSYSERR_NOERROR )
    {
        printf( "failed to prepare wave buffer, error=%lu\n", mmr );
        *m_phrCompletion = E_UNEXPECTED;
        //SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }*/

    /*mmr = waveOutWrite( m_hwo, pwh, sizeof(WAVEHDR) );

    if( mmr != MMSYSERR_NOERROR )
    {
        delete pwh;

        printf( "failed to write wave sample, error=%lu\n", mmr );
        *m_phrCompletion = E_UNEXPECTED;
        //SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }*/

    InterlockedIncrement( &m_cBuffersOutstanding );

    return( S_OK );
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

//#endif

	//MessageBox(NULL, "Foo", "Foo", 0);
    
    if( FAILED( hr ) )
    {
        //printf( "failed to create audio reader (hr=%#X)\n", hr );
        return( hr );
    }

    //
    // It worked!  Display various attributes
    //
    /*WORD i, wAttrCnt;

    hr = m_pReader->GetAttributeCount( &wAttrCnt );
    if ( FAILED( hr ) )
    {
        printf(" GetAttributeCount Faied %x\n", hr );
        return( hr );
    }*/

    /*for ( i = 0; i < wAttrCnt ; i++ )
    {

        WCHAR  wName[512];
        WORD cbNamelen = sizeof ( wName );
        WMT_ATTR_DATATYPE type;
        BYTE pValue[512];
        WORD cbLength = sizeof( pValue );

        hr = m_pReader->GetAttributeByIndex( i, wName, &cbNamelen, &type, pValue, &cbLength );
        if ( FAILED( hr ) ) 
        {
            if ( hr == E_NOTIMPL )
            {
                continue;
            }
            printf( "GetAttributeByIndex Failed %x\n", hr );
            return( hr );
        }

        switch ( type )
        {
        case WMT_TYPE_DWORD:
            printf("%S:  %d\n", wName, *((DWORD *) pValue) );
            break;
        case WMT_TYPE_STRING:
            printf("%S:   %S\n", wName, (WCHAR *) pValue );
            break;
        case WMT_TYPE_BINARY:
            printf("%S:   Type = Binary of Length %d\n", wName, cbLength);
            break;
        case WMT_TYPE_BOOL:
            printf("%S:   %s\n", wName, ( * ( ( BOOL * ) pValue) ? "true" : "false" ) );
            break;
        default:
            break;
        }
    }*/


    //
    // Set up for audio playback
    //
    WAVEFORMATEX*   pbFormat = (WAVEFORMATEX*)new BYTE[1024];

    hr = m_pReader->GetOutputFormat( &m_wfx, sizeof( m_WfxBuf ) );

    if( FAILED( hr ) )
    {
        //printf( "failed GetOutputFormat(), hr=0x%lX\n", hr );
        return( hr );
    }

	CreateStreamingBuffer(&m_wfx);

	m_pDSBuffer->SetVolume(volume);

	/*if( FAILED( hr = m_pDSBuffer->Play( 0, 0, DSBPLAY_LOOPING ) ) )
        return hr;*/
    
    /*MMRESULT mmr;

    mmr = waveOutOpen( &m_hwo, 
                       WAVE_MAPPER, 
                       &m_wfx, 
                       (DWORD)WaveProc, 
                       (DWORD)this, 
                       CALLBACK_FUNCTION );

    if( mmr != MMSYSERR_NOERROR  )
    {
        printf( "failed to open wav output device, error=%lu\n", mmr );
        return( E_UNEXPECTED );
    }*/

	fStartedPlay = fFalse;

    //
    // Start reading the data (and rendering the audio)
    //
    hr = m_pReader->Start( );

    if( FAILED( hr ) )
    {
        //printf( "failed Start(), hr=0x%lX\n", hr );
        return( hr );
    }

    return( hr );
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

        m_fEof = TRUE;

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
				DWORD foo = WaitForSingleObjectEx(m_hNotificationEvent, 3000, FALSE);
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

    /*case WMT_NO_RIGHTS:
        {
            LPWSTR pszEscapedURL = NULL;

            hr = MakeEscapedURL( m_pszUrl, &pszEscapedURL );

            if( SUCCEEDED( hr ) )
            {
                WCHAR szURL[ 0x1000 ];

                swprintf( szURL, L"%s&filename=%s&embedded=false", pParam->bstrVal, pszEscapedURL );

                hr = LaunchURL( szURL );

                if( FAILED( hr ) )
                {
                    printf( "Unable to launch web browser to retrieve playback license (err = %#X)\n", hr );
                }

                delete [] pszEscapedURL;
            }
        }
        break;*/

    case WMT_MISSING_CODEC:

        //
        // find the guid for the missing codec
        //
        GUID    guidCodecID;

        hr = CLSIDFromString( pParam->bstrVal, &guidCodecID );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        hr = DoCodecDownload( &guidCodecID );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        break;

        break;
    };

    return( S_OK );
}


///////////////////////////////////////////////////////////////////////////////
/*void CSimplePlayer::OnWaveOutMsg( UINT uMsg, DWORD dwParam1, DWORD dwParam2 )
{
    if( WOM_DONE == uMsg )
    {
        waveOutUnprepareHeader( m_hwo, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR) );
        
        delete (void*)dwParam1;

        InterlockedDecrement( &m_cBuffersOutstanding );

        if( m_fEof && ( 0 == m_cBuffersOutstanding ) )
        {
            //SetEvent( m_hCompletionEvent );
        }
    }
}*/


///////////////////////////////////////////////////////////////////////////////
/*void CALLBACK CSimplePlayer::WaveProc( 
                                HWAVEOUT hwo, 
                                UINT uMsg, 
                                DWORD dwInstance, 
                                DWORD dwParam1, 
                                DWORD dwParam2 )
{
    CSimplePlayer *pThis = (CSimplePlayer*)dwInstance;

    pThis->OnWaveOutMsg( uMsg, dwParam1, dwParam2 );
}*/


///////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::DoCodecDownload( GUID* pguidCodecID )
{
    /*HRESULT hr;

    CoInitialize( NULL );

    //
    // Create a DownloadCallback object to manage the codec download
    // process for us (see codecdownload.h/cpp)
    //
    CDownloadCallback*  pCallback = new CDownloadCallback();
    if( NULL == pCallback )
    {
        CoUninitialize();
        return( E_OUTOFMEMORY );
    }

    IBindStatusCallback *pBSCallback;
    hr = pCallback->QueryInterface( IID_IBindStatusCallback, (void **) &pBSCallback );
    assert( hr == S_OK );

    //
    // Do a LoadLibrary()/GetProcAddress() to grab the urlmon DLL and some functions out of it.
    // We do this so that we can dynamically bind to the internet stuff and not require it to 
    // be a part of our base SDK download package.  By dynamically binding, we still allow
    // all the other functions to work even if the codec download stuff can't.
    // 
    HINSTANCE   hinstUrlmonDLL;

    hinstUrlmonDLL = LoadLibrary( "urlmon.dll" );
    if( NULL == hinstUrlmonDLL )
    {
        CoUninitialize();
        return( E_FAIL );
    }

    //
    // CreateAsyncBindCtx to create the bind context
    //
    FARPROC     lpfnCreateAsyncBindCtx = NULL;

    lpfnCreateAsyncBindCtx = GetProcAddress( hinstUrlmonDLL, "CreateAsyncBindCtx" );
    if( NULL == lpfnCreateAsyncBindCtx )
    {
        FreeLibrary( hinstUrlmonDLL );
        CoUninitialize();
        return( E_FAIL );
    }

    IBindCtx*   pBindCtx = NULL;

    hr = (*((HRESULT(STDAPICALLTYPE *)(DWORD,IBindStatusCallback*,IEnumFORMATETC*,IBindCtx**))lpfnCreateAsyncBindCtx))( 
                        0, 
                        pBSCallback, 
                        NULL, 
                        &pBindCtx );
    if( FAILED( hr ) ) 
    {
        FreeLibrary( hinstUrlmonDLL );
    	CoUninitialize();
        return hr;
    }

    //
    // CoGetClassObjectFromURL to grab the object from the across the network
    //
    FARPROC     lpfnCoGetClassObjectFromURL = NULL;

    lpfnCoGetClassObjectFromURL = GetProcAddress( hinstUrlmonDLL, "CoGetClassObjectFromURL" );
    if( NULL == lpfnCoGetClassObjectFromURL )
    {
        FreeLibrary( hinstUrlmonDLL );
        CoUninitialize();
        return( E_FAIL );
    }

    IUnknown*   pUnkObject = NULL;

    hr = (*((HRESULT(STDAPICALLTYPE *)(REFCLSID,LPCWSTR,DWORD,DWORD,LPCWSTR,LPBINDCTX,DWORD,LPVOID,REFIID,VOID**))lpfnCoGetClassObjectFromURL))(
                        *pguidCodecID,          // the clsid we want
                        NULL,                   // the URL we're downloading from.  NULL means go to the registry to find the URL
                        0x1,                    // major version number.  ACM codecs aren't versioned, so we send zero for "any"
                        0x0,                    // minor version number.
                        NULL,                   // mime type
				        pBindCtx,               // bind ctx used for downloading/installing
                        CLSCTX_INPROC,          // execution context
                        NULL,                   // reserved, must be NULL
                        IID_IUnknown,           // interface to obtain
				        (void **)&pUnkObject);  // pointer to store the new object

    if( hr == S_ASYNCHRONOUS ) 
    {
        //
        // Turn an asynchronous call into a synchronous call by
        // sitting here and waiting for the call to finish.
        //
	    for (;;) 
        {
	        HANDLE ev = pCallback->m_evFinished;
	        
	        DWORD dwResult = MsgWaitForMultipleObjects(
			                         1,
			                         &ev,
			                         FALSE,
			                         INFINITE,
			                         QS_ALLINPUT );

	        if( dwResult == WAIT_OBJECT_0 )
            {
		        break;
            }
	        
	        assert( dwResult == WAIT_OBJECT_0 + 1 );

            //
	        // Eat messages and go round again
            //
	        MSG Message;
	        while( PeekMessage( &Message,NULL,0,0,PM_REMOVE ) ) 
            {
		        TranslateMessage(&Message);
		        DispatchMessage(&Message);
	        }
	    }

	    hr = pCallback->m_hrBinding;
    }

    //
    // Do some cleaning
    //
    if( pBindCtx )
    {
        pBindCtx->Release();
        pBindCtx = NULL;
    }

    if( pBSCallback )
    {
        pBSCallback->Release();
        pBSCallback = NULL;
    }

    if( pUnkObject )
    {
        pUnkObject->Release();
        pBindCtx = NULL;
    }

    FreeLibrary( hinstUrlmonDLL );

    //
    // We treat REGDB_E_CLASSNOTREG as a success because ACM components are
    // not COM objects -- therefore they'll get downloaded just fine, but the
    // call to instantiate them will fail with REGDB_E_CLASSNOTREG.
    //
    if( REGDB_E_CLASSNOTREG == hr )
    {
        hr = S_OK;
    }

    CoUninitialize();
    return( hr );*/
	return S_OK;
}

HRESULT CSimplePlayer::CreateStreamingBuffer(WAVEFORMATEX *pwfx)
{
    HRESULT hr; 

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
    m_bFoundEnd     = FALSE;
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
    if( FAILED( hr = g_pvp->m_pds.m_pDS->CreateSoundBuffer( &dsbd, &m_pDSBuffer, NULL ) ) )
        return hr;

    /*for( INT i = 0; i < NUM_PLAY_NOTIFICATIONS; i++ )
    {
        m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
        m_aPosNotify[i].hEventNotify = m_hNotificationEvents[0];             
    }
    
    m_aPosNotify[i].dwOffset     = DSBPN_OFFSETSTOP;
    m_aPosNotify[i].hEventNotify = m_hNotificationEvents[1];*/

    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    /*if( FAILED( hr = m_pDSNotify->SetNotificationPositions( NUM_PLAY_NOTIFICATIONS + 1, 
                                                            m_aPosNotify ) ) )
        return hr;*/

	{
	VOID*   pbBuffer = NULL;
    //DWORD   dwBufferLength;

    m_bFoundEnd = FALSE;
    m_dwNextWriteOffset = 0; 
    m_dwProgress = 0;
    m_dwLastPos  = 0;

	m_pDSBuffer->SetCurrentPosition(0);

	// Lock the buffer down, 
    /*if( FAILED( hr = m_pDSBuffer->Lock( 0, m_dwBufferSize, 
                                        &pbBuffer, &dwBufferLength, 
                                        NULL, NULL, 0L ) ) )
        return hr;

    // Now unlock the buffer
    m_pDSBuffer->Unlock( pbBuffer, dwBufferLength, NULL, 0 );*/

	m_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    /*if( NULL == g_pDSBuffer )
        return E_FAIL;*/

    //m_dwNextWriteOffset = dwBufferLength; 
    //m_dwNextWriteOffset %= m_dwBufferSize; // Circular buffer
	}

    return S_OK;
}

