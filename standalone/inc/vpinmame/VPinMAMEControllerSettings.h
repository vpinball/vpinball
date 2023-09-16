#pragma once

#pragma push_macro("_WIN64")
#pragma push_macro("DELETE")
#pragma push_macro("CALLBACK")
#undef _WIN64
#undef DELETE
#undef CALLBACK
#include "libpinmame.h"
#pragma pop_macro("CALLBACK")
#pragma pop_macro("DELETE")
#pragma pop_macro("_WIN64")

#include "vpinmame_i.h"

class VPinMAMEControllerSettings : public IControllerSettings
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IControllerSettings) {
         *ppv = reinterpret_cast<VPinMAMEControllerSettings*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<VPinMAMEControllerSettings*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<VPinMAMEControllerSettings*>(this);
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
      m_dwRef--;

      if (m_dwRef == 0)
         delete this;

      return m_dwRef;
   }

   STDMETHOD(GetTypeInfoCount)(UINT *pCountTypeInfo) { *pCountTypeInfo = 0; return S_OK;  }
   STDMETHOD(GetTypeInfo)(UINT iInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   VPinMAMEControllerSettings();
   ~VPinMAMEControllerSettings();

   STDMETHOD(get_InstallDir)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Value)(/*[in]*/ BSTR sName, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Value)(/*[in]*/ BSTR sName, /*[in]*/ VARIANT newVal);
   STDMETHOD(Clear)();
   STDMETHOD(ShowSettingsDlg)(LONG_PTR hParentWnd);

private:
   ULONG m_dwRef = 0;
};