#pragma once

#include "libpinmame.h"

#include "vpinmame_i.h"
#include "VPinMAMEController.h"

class VPinMAMEGame;

class VPinMAMEGames : public IGames
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IGames) {
         *ppv = reinterpret_cast<VPinMAMEGames*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<VPinMAMEGames*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<VPinMAMEGames*>(this);
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
   VPinMAMEGames(VPinMAMEController* pController);
   ~VPinMAMEGames();

   STDMETHOD(get_Count)(LONG* pnCount);
   STDMETHOD(get_Item)(VARIANT *pKey, IGame **pGame);
   STDMETHOD(get__NewEnum)(IUnknown** ppunkEnum);

   static void PINMAMECALLBACK GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData);

private:
   PinmameGame m_pinmameGame;
   VPinMAMEController* m_pController;

   ULONG m_dwRef = 0;
};
