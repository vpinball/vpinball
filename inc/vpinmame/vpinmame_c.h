#pragma once
#if !defined(VPINMAME_C_H__INCLUDED_)
#define VPINMAME_C_H__INCLUDED_

#pragma GCC visibility push(hidden)

class VPinMAMEController :
   public IDispatchImpl<IController, &IID_IController, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEController, &CLSID_Controller>
{
public:
   static robin_hood::unordered_map<wstring, int> m_nameIDMap;
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
};

#pragma GCC visibility pop

#endif // !defined(VPINMAME_C_H__INCLUDED_)
