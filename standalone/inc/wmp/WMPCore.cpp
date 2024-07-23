#include "stdafx.h"
#include "WMPCore.h"

WMPCore::WMPCore()
{
   m_pAudioPlayer = new AudioPlayer();

   CComObject<WMPControls>::CreateInstance(&m_pControls);
   m_pControls->AddRef();
   m_pControls->m_pCore = this;

   CComObject<WMPSettings>::CreateInstance(&m_pSettings);
   m_pSettings->AddRef();
   m_pSettings->m_pCore = this;

   m_playState = wmppsUndefined;
}

WMPCore::~WMPCore()
{
   m_pControls->Release();
   m_pSettings->Release();

   delete m_pAudioPlayer;
}

STDMETHODIMP WMPCore::close()
{
   PLOGI.printf("player=%p, close", this);

   m_pAudioPlayer->MusicClose();

   m_szURL.clear();
   m_playState = wmppsUndefined;

   return S_OK;
}

STDMETHODIMP WMPCore::get_URL(BSTR *pbstrURL)
{
   WCHAR wzTmp[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, m_szURL.c_str(), -1, wzTmp, MAXSTRING);
   *pbstrURL = SysAllocString(wzTmp);

   return S_OK;
}

STDMETHODIMP WMPCore::put_URL(BSTR pbstrURL)
{
   m_szURL = normalize_path_separators(MakeString(pbstrURL));

   PLOGI.printf("player=%p, URL=%s", this, m_szURL.c_str());

   if (m_pAudioPlayer->SetMusicFile(m_szURL))
      m_playState = wmppsReady;
   else
      m_playState = wmppsUndefined;

   return S_OK;
}

STDMETHODIMP WMPCore::get_openState(WMPOpenState *pwmpos) { return E_NOTIMPL; }

STDMETHODIMP WMPCore::get_playState(WMPPlayState *pwmpps)
{
   if (m_playState != wmppsUndefined)
      m_playState = m_pAudioPlayer->MusicActive() ? wmppsPlaying : wmppsStopped;

   *pwmpps = m_playState;

   return S_OK;
}

STDMETHODIMP WMPCore::get_controls(IWMPControls **ppControl)
{
   if (!ppControl)
      return S_FALSE;

   return m_pControls->QueryInterface(IID_IWMPControls, (void**) ppControl);
}

STDMETHODIMP WMPCore::get_settings(IWMPSettings **ppSettings)
{
   if (!ppSettings)
      return S_FALSE;

   return m_pSettings->QueryInterface(IID_IWMPSettings, (void**) ppSettings);
}

STDMETHODIMP WMPCore::get_currentMedia(IWMPMedia **ppMedia) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::put_currentMedia(IWMPMedia *ppMedia) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_mediaCollection(IWMPMediaCollection **ppMediaCollection) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_playlistCollection(IWMPPlaylistCollection **ppPlaylistCollection) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_versionInfo(BSTR *pbstrVersionInfo) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::launchURL(BSTR bstrURL) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_network(IWMPNetwork **ppQNI) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_currentPlaylist(IWMPPlaylist **ppPL) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::put_currentPlaylist(IWMPPlaylist *ppPL) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_cdromCollection(IWMPCdromCollection **ppCdromCollection) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_closedCaption(IWMPClosedCaption **ppClosedCaption) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_isOnline(VARIANT_BOOL *pfOnline) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_Error(IWMPError **ppError) { return E_NOTIMPL; }
STDMETHODIMP WMPCore::get_status(BSTR *pbstrStatus) { return E_NOTIMPL; }