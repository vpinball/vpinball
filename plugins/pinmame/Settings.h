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

   int GetValue(const string& key) const
   {
      const string k = string_to_lower(key);
      const auto it = m_values.find(k);
      if (it != m_values.end())
         return it->second;
      int v;
      if (TryGetSettingDefault(k, v))
         return v;
      LOGW(std::format("Unknown setting '{}', returning 0", key));
      return 0;
   }
   void SetValue(const string& key, int v) { m_values[string_to_lower(key)] = v; }

private:
   std::unordered_map<string, int> m_values;
};

}
