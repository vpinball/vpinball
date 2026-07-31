// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"
#include "SettingsDefaults.h"

#include <unordered_map>

namespace PinMAME {

class Settings final
{
public:
   Settings();
   ~Settings();

   PSC_IMPLEMENT_REFCOUNT()

   int GetValue(const string& key) const;
   void SetValue(const string& key, int v);

private:
   std::unordered_map<string, int> m_values;
};

}
