#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

namespace PinMAME {

class Settings final
{
public:
   Settings();
   ~Settings();

   PSC_IMPLEMENT_REFCOUNT()

   int GetValue(const string& key) const { return 0; }
   void SetValue(const string& key, int v) { }
};

}