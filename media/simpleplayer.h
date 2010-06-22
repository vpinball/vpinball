#pragma once
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
#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

#include "wmaudiosdk.h"

#include <mmsystem.h>

///////////////////////////////////////////////////////////////////////////////
class CSimplePlayer : 
    public IWMAudioReadCallback
{
public:
    CSimplePlayer();
    ~CSimplePlayer();
    
    virtual HRESULT Play(LPCWSTR pszUrl, HANDLE hCompletionEvent, HRESULT *phrCompletion, int volume);

	void Pause();
	void Unpause();

    // IUnknown
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void **ppvObject );

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IWMAudioReadCallback
public:
    virtual HRESULT STDMETHODCALLTYPE OnSample( 
        /* [in] */ const BYTE * pData,
        /* [in] */ DWORD cbData,
        /* [in] */ DWORD dwMsTime
                        );

    virtual HRESULT STDMETHODCALLTYPE OnStatus( 
        /* [in] */ WMT_STATUS Status, 
        /* [in] */ HRESULT hr,
        /* [in] */ const VARIANT *pParam
                        );

public:

	void Stop();
 
    HRESULT DoCodecDownload( GUID*  pCodecID );

    LONG    m_cRef;
    LONG    m_cBuffersOutstanding;
    HANDLE  m_hCompletionEvent;

    IWMAudioReader *m_pReader;
    HWAVEOUT m_hwo;

    HRESULT *m_phrCompletion;

    union
    {
        WAVEFORMATEX m_wfx;
        BYTE m_WfxBuf[1024];
    };

	bool fStartedPlay;

#ifdef SUPPORT_DRM
    IWMAudioReadCallback *m_pDRMCallback;
#endif

	HRESULT CreateStreamingBuffer(WAVEFORMATEX *pwfx);

//#define NUM_PLAY_NOTIFICATIONS  16

	LPDIRECTSOUNDBUFFER m_pDSBuffer;
	LPDIRECTSOUNDNOTIFY m_pDSNotify;

	DSBPOSITIONNOTIFY   m_aPosNotify;  
	HANDLE              m_hNotificationEvent;

	DWORD               m_dwBufferSize;
	DWORD               m_dwNotifySize;
	DWORD               m_dwNextWriteOffset;
	DWORD               m_dwProgress;
	DWORD               m_dwLastPos;
};

#endif // SIMPLEPLAYER_H
