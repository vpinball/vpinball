#pragma once

#include "libpinmame.h"

#include "vpinmame_i.h"
#include "VPinMAMEController.h"

class VPinMAMEGameSettings : public IGameSettings
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IGameSettings) {
         *ppv = reinterpret_cast<VPinMAMEGameSettings*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<VPinMAMEGameSettings*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<VPinMAMEGameSettings*>(this);
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
   VPinMAMEGameSettings(VPinMAMEController* pController, IGame* pGame);
   ~VPinMAMEGameSettings();

   STDMETHOD(get_Value)(/*[in]*/ BSTR sName, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Value)(/*[in]*/ BSTR sName, /*[in]*/ VARIANT newVal);
   STDMETHOD(Clear)();
   STDMETHOD(ShowSettingsDlg)(/* [in,defaultvalue(0)] */ LONG_PTR hParentWnd);
   STDMETHOD(SetDisplayPosition)(/*[in]*/ VARIANT newValX, /*[in]*/ VARIANT newValY,/* [in,defaultvalue(0)] */ LONG_PTR hWnd);

private:
   VPinMAMEController* m_pController;
   IGame* m_pGame;

   ULONG m_dwRef = 0;
};
