#pragma once

#include "libpinmame.h"

#include "vpinmame_i.h"
#include "VPinMAMEController.h"

class VPinMAMEGame : public IGame
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IGame) {
         *ppv = reinterpret_cast<VPinMAMEGame*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<VPinMAMEGame*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<VPinMAMEGame*>(this);
         AddRef();
         return S_OK;
      } else {
         *ppv = NULL;
         return E_NOINTERFACE;
      }
   }

   STDMETHOD_(ULONG, AddRef)() {
      m_dwRef++;
      return m_dwRef;
   }

   STDMETHOD_(ULONG, Release)() {
      ULONG dwRef = --m_dwRef;

      if (dwRef == 0)
         delete this;

      return dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   VPinMAMEGame(VPinMAMEController* pController, PinmameGame* pPinmameGame);
   ~VPinMAMEGame();

   STDMETHOD(get_IsSupported)(/* [out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Manufacturer)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Year)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Description)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_CloneOf)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Roms)(/*[out, retval]*/ IRoms* *pVal);
   STDMETHOD(ShowInfoDlg)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_Settings)(/*[out, retval]*/ IGameSettings * *pVal);

   string GetName() { return string(m_pinmameGame.name); }

private:
   PinmameGame m_pinmameGame;
   VPinMAMEController* m_pController;

   ULONG m_dwRef = 0;
};