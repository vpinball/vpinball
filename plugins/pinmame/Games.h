#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

namespace PinMAME {

class Games final
{
public:
   Games();
   ~Games();

   PSC_IMPLEMENT_REFCOUNT()

   //STDMETHOD(get_Count)(LONG* pnCount);
   //STDMETHOD(get_Item)(VARIANT* pKey, IGame** pGame);
   //STDMETHOD(get__NewEnum)(IUnknown** ppunkEnum);
};

}