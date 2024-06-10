#pragma once

#include "../flexdmd_i.h"

class Actor;

class ActionFactory : public IDispatch
{
public:
   STDMETHOD(QueryInterface)(REFIID iid, void** ppv) {
      if (iid == IID_IActionFactory) {
         *ppv = reinterpret_cast<ActionFactory*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IDispatch) {
         *ppv = reinterpret_cast<ActionFactory*>(this);
         AddRef();
         return S_OK;
      } else if (iid == IID_IUnknown) {
         *ppv = reinterpret_cast<ActionFactory*>(this);
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
   ActionFactory(Actor* pTarget);
   ~ActionFactory();

   STDMETHOD(Wait)(single secondsToWait, IUnknown **pRetVal);
   STDMETHOD(Delayed)(single secondsToWait, IUnknown *action, IUnknown **pRetVal);
   STDMETHOD(Parallel)(ICompositeAction **pRetVal);
   STDMETHOD(Sequence)(ICompositeAction **pRetVal);
   STDMETHOD(Repeat)(IUnknown *action, LONG count, IUnknown **pRetVal);
   STDMETHOD(Blink)(single secondsShow, single secondsHide, LONG Repeat, IUnknown **pRetVal);
   STDMETHOD(Show)(VARIANT_BOOL visible, IUnknown **pRetVal);
   STDMETHOD(AddTo)(IGroupActor *parent, IUnknown **pRetVal);
   STDMETHOD(RemoveFromParent)(IUnknown **pRetVal);
   STDMETHOD(AddChild)(IUnknown *child, IUnknown **pRetVal);
   STDMETHOD(RemoveChild)(IUnknown *child, IUnknown **pRetVal);
   STDMETHOD(Seek)(single pos, IUnknown **pRetVal);
   STDMETHOD(MoveTo)(single x, single y, single duration, ITweenAction **pRetVal);

private:
   Actor* m_pTarget;

   ULONG m_dwRef = 0;
};