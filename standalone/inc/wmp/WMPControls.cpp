#include "stdafx.h"
#include "WMPControls.h"
#include "WMPCore.h"

WMPControls::WMPControls()
{
}

WMPControls::~WMPControls()
{
}

STDMETHODIMP WMPControls::get_isAvailable(BSTR bstrItem, VARIANT_BOOL *pIsAvailable) { return E_NOTIMPL; }

STDMETHODIMP WMPControls::play()
{
    PLOGI.printf("player=%p, play", m_pCore);

    m_pCore->m_pAudioPlayer->MusicPlay();

    return S_OK;
}

STDMETHODIMP WMPControls::stop()
{
    PLOGI.printf("player=%p, stop", m_pCore);

    m_pCore->m_pAudioPlayer->MusicStop();

    return S_OK;
}

STDMETHODIMP WMPControls::pause()
{
    PLOGI.printf("player=%p, pause", m_pCore);

    m_pCore->m_pAudioPlayer->MusicPause();

    return S_OK;
}

STDMETHODIMP WMPControls::fastForward() { return E_NOTIMPL; }
STDMETHODIMP WMPControls::fastReverse() { return E_NOTIMPL; }

STDMETHODIMP WMPControls::get_currentPosition(double *pdCurrentPosition)
{
    *pdCurrentPosition = m_pCore->m_pAudioPlayer->GetMusicPosition();

    return S_OK;
}

STDMETHODIMP WMPControls::put_currentPosition(double pdCurrentPosition)
{
    PLOGI.printf("player=%p, position=%f", this, pdCurrentPosition);

    m_pCore->m_pAudioPlayer->SetMusicPosition(pdCurrentPosition);

    return S_OK;
}

STDMETHODIMP WMPControls::get_currentPositionString(BSTR *pbstrCurrentPosition) { return E_NOTIMPL; }
STDMETHODIMP WMPControls::next() { return E_NOTIMPL; }
STDMETHODIMP WMPControls::previous() { return E_NOTIMPL; }
STDMETHODIMP WMPControls::currentItem(IWMPMedia **ppIWMPMedia) { return E_NOTIMPL; }
STDMETHODIMP WMPControls::put_currentItem(IWMPMedia *ppIWMPMedia) { return E_NOTIMPL; }
STDMETHODIMP WMPControls::get_currentMarker(LONG *plMarker) { return E_NOTIMPL; }
STDMETHODIMP WMPControls::put_currentMarker(LONG plMarker) { return E_NOTIMPL; }
STDMETHODIMP WMPControls::playItem(IWMPMedia *pIWMPMedia) { return E_NOTIMPL; }