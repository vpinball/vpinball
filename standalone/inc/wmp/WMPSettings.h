#pragma once

#include "wmp_i.h"

class WMPCore;

class WMPSettings :
   public IDispatchImpl<IWMPSettings, &IID_IWMPSettings, &LIBID_WMPLib>,
   public CComObjectRoot,
   public CComCoClass<WMPSettings, &CLSID_WindowsMediaPlayer>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   WMPSettings();
   ~WMPSettings();

   BEGIN_COM_MAP(WMPSettings)
      COM_INTERFACE_ENTRY(IWMPSettings)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_isAvailable)(BSTR bstrItem, VARIANT_BOOL *pIsAvailable);
   STDMETHOD(get_autoStart)(VARIANT_BOOL *pfAutoStart);
   STDMETHOD(put_autoStart)(VARIANT_BOOL pfAutoStart);
   STDMETHOD(get_baseURL)(BSTR *pbstrBaseURL);
   STDMETHOD(put_baseURL)(BSTR pbstrBaseURL);
   STDMETHOD(get_defaultFrame)(BSTR *pbstrDefaultFrame);
   STDMETHOD(put_defaultFrame)(BSTR pbstrDefaultFrame);
   STDMETHOD(get_invokeURLs)(VARIANT_BOOL *pfInvokeURLs);
   STDMETHOD(put_invokeURLs)(VARIANT_BOOL pfInvokeURLs);
   STDMETHOD(get_mute)(VARIANT_BOOL *pfMute);
   STDMETHOD(put_mute)(VARIANT_BOOL pfMute);
   STDMETHOD(get_playCount)(LONG *plCount);
   STDMETHOD(put_playCount)(LONG plCount);
   STDMETHOD(get_rate)(double *pdRate);
   STDMETHOD(put_rate)(double pdRate);
   STDMETHOD(get_balance)(LONG *plBalance);
   STDMETHOD(put_balance)(LONG plBalance);
   STDMETHOD(get_volume)(LONG *plVolume);
   STDMETHOD(put_volume)(LONG plVolume);
   STDMETHOD(getMode)(BSTR bstrMode, VARIANT_BOOL *pvarfMode);
   STDMETHOD(setMode)(BSTR bstrMode, VARIANT_BOOL varfMode);
   STDMETHOD(get_enableErrorDialogs)(VARIANT_BOOL *pfEnableErrorDialogs);
   STDMETHOD(put_enableErrorDialogs)(VARIANT_BOOL pfEnableErrorDialogs);

public:
   LONG m_volume;

   WMPCore* m_pCore;
};