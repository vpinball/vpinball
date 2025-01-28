#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

class Roms
{
public:
   Roms();
   ~Roms();

   PSC_IMPLEMENT_REFCOUNT()

   //STDMETHOD(Audit)(/*[in]*/ VARIANT_BOOL fStrict);
   //STDMETHOD(get_StateDescription)(/*[out, retval]*/ BSTR *pVal);
   //STDMETHOD(get_State)(/*[out, retval]*/ LONG *pVal);
   //STDMETHOD(get_Available)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   //STDMETHOD(get_Count)(LONG *pnCount);
   //STDMETHOD(get_Item)(VARIANT *pKey, IRom **pRom);
   //STDMETHOD(get__NewEnum)(IUnknown **ppunkEnum);
};
