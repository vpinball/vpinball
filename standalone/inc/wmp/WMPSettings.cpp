#include "stdafx.h"
#include "WMPSettings.h"
#include "WMPCore.h"

WMPSettings::WMPSettings()
{
}

WMPSettings::~WMPSettings()
{
}

STDMETHODIMP WMPSettings::get_isAvailable(BSTR bstrItem, VARIANT_BOOL *pIsAvailable) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_autoStart(VARIANT_BOOL *pfAutoStart) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_autoStart(VARIANT_BOOL pfAutoStart) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_baseURL(BSTR *pbstrBaseURL) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_baseURL(BSTR pbstrBaseURL) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_defaultFrame(BSTR *pbstrDefaultFrame) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_defaultFrame(BSTR pbstrDefaultFrame) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_invokeURLs(VARIANT_BOOL *pfInvokeURLs) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_invokeURLs(VARIANT_BOOL pfInvokeURLs) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_mute(VARIANT_BOOL *pfMute) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_mute(VARIANT_BOOL pfMute) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_playCount(LONG *plCount) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_playCount(LONG plCount) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_rate(double *pdRate) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_rate(double pdRate) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_balance(LONG *plBalance) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_balance(LONG plBalance) { return E_NOTIMPL; }

STDMETHODIMP WMPSettings::get_volume(LONG *plVolume)
{
   *plVolume = m_volume;

   return S_OK;
}

STDMETHODIMP WMPSettings::put_volume(LONG plVolume)
{
   m_volume = plVolume;

   PLOGI.printf("player=%p, volume=%ld", this, plVolume);

   m_pCore->m_pAudioPlayer->MusicVolume(plVolume/100.);

   return S_OK;
}

STDMETHODIMP WMPSettings::getMode(BSTR bstrMode, VARIANT_BOOL *pvarfMode) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::setMode(BSTR bstrMode, VARIANT_BOOL varfMode) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::get_enableErrorDialogs(VARIANT_BOOL *pfEnableErrorDialogs) { return E_NOTIMPL; }
STDMETHODIMP WMPSettings::put_enableErrorDialogs(VARIANT_BOOL pfEnableErrorDialogs) { return E_NOTIMPL; }