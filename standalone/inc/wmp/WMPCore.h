#pragma once

#include "wmp_i.h"

#include "WMPControls.h"
#include "WMPSettings.h"

class WMPCore :
   public IDispatchImpl<IWMPCore, &IID_IWMPCore, &LIBID_WMPLib>,
   public CComObjectRoot,
   public CComCoClass<WMPCore, &CLSID_WindowsMediaPlayer>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   WMPCore();
   ~WMPCore();

   BEGIN_COM_MAP(WMPCore)
      COM_INTERFACE_ENTRY(IWMPCore)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(close)();
   STDMETHOD(get_URL)(BSTR *pbstrURL);
   STDMETHOD(put_URL)(BSTR pbstrURL);
   STDMETHOD(get_openState)(WMPOpenState *pwmpos);
   STDMETHOD(get_playState)(WMPPlayState *pwmpps);
   STDMETHOD(get_controls)(IWMPControls **ppControl);
   STDMETHOD(get_settings)(IWMPSettings **ppSettings);
   STDMETHOD(get_currentMedia)(IWMPMedia **ppMedia);
   STDMETHOD(put_currentMedia)(IWMPMedia *ppMedia);
   STDMETHOD(get_mediaCollection)(IWMPMediaCollection **ppMediaCollection);
   STDMETHOD(get_playlistCollection)(IWMPPlaylistCollection **ppPlaylistCollection);
   STDMETHOD(get_versionInfo)(BSTR *pbstrVersionInfo);
   STDMETHOD(launchURL)(BSTR bstrURL);
   STDMETHOD(get_network)(IWMPNetwork **ppQNI);
   STDMETHOD(get_currentPlaylist)(IWMPPlaylist **ppPL);
   STDMETHOD(put_currentPlaylist)(IWMPPlaylist *ppPL);
   STDMETHOD(get_cdromCollection)(IWMPCdromCollection **ppCdromCollection);
   STDMETHOD(get_closedCaption)(IWMPClosedCaption **ppClosedCaption);
   STDMETHOD(get_isOnline)(VARIANT_BOOL *pfOnline);
   STDMETHOD(get_Error)(IWMPError **ppError);
   STDMETHOD(get_status)(BSTR *pbstrStatus);

public:
   AudioPlayer* m_pAudioPlayer;
   string m_szURL;
   WMPPlayState m_playState;

private:
   CComObject<WMPControls>* m_pControls;
   CComObject<WMPSettings>* m_pSettings;
};
