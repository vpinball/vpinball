#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

class Games
{
public:
   Games();
   ~Games();

   PSC_IMPLEMENT_REFCOUNT()

   //STDMETHOD(get_Count)(LONG* pnCount);
   //STDMETHOD(get_Item)(VARIANT* pKey, IGame** pGame);
   //STDMETHOD(get__NewEnum)(IUnknown** ppunkEnum);
};
