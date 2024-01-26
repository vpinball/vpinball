#pragma once

#include "wmp_i.h"

class WMPCore;

class WMPControls :
   public IDispatchImpl<IWMPControls, &IID_IWMPControls, &LIBID_WMPLib>,
   public CComObjectRoot,
   public CComCoClass<WMPControls, &CLSID_WindowsMediaPlayer>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   WMPControls();
   ~WMPControls();

   BEGIN_COM_MAP(WMPControls)
      COM_INTERFACE_ENTRY(IWMPControls)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_isAvailable)(BSTR bstrItem,VARIANT_BOOL *pIsAvailable);
   STDMETHOD(play)();
   STDMETHOD(stop)();
   STDMETHOD(pause)();
   STDMETHOD(fastForward)();
   STDMETHOD(fastReverse)();
   STDMETHOD(get_currentPosition)(double *pdCurrentPosition);
   STDMETHOD(put_currentPosition)(double pdCurrentPosition);
   STDMETHOD(get_currentPositionString)(BSTR *pbstrCurrentPosition);
   STDMETHOD(next)();
   STDMETHOD(previous)();
   STDMETHOD(currentItem)(IWMPMedia **ppIWMPMedia);
   STDMETHOD(put_currentItem)(IWMPMedia *ppIWMPMedia);
   STDMETHOD(get_currentMarker)(LONG *plMarker);
   STDMETHOD(put_currentMarker)(LONG plMarker);
   STDMETHOD(playItem)(IWMPMedia *pIWMPMedia);

public:
   WMPCore* m_pCore;
};
