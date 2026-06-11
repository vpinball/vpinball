// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"
#include "SettingsDefaults.h"

#include <unordered_map>

namespace PinMAME {

class GameSettings final
{
public:
   GameSettings();
   ~GameSettings();

   PSC_IMPLEMENT_REFCOUNT()

   void Clear() { m_values.clear(); }
   // FIXME implement a dynamically typed variant
   //ScriptVariant GetValue(const string& name) const { return {}; /* Not yet implemented */ }
   //void PutValue(const string& name, ScriptVariant v) { /* Not yet implemented */ }
   int GetValue(const string& key) const
   {
      const string k = string_to_lower(key);
      const auto it = m_values.find(k);
      if (it != m_values.end())
         return it->second;
      int v;
      if (TryGetSettingDefault(k, v))
         return v;
      LOGW(std::format("Unknown game setting '{}', returning 0", key));
      return 0;
   }
   void SetValue(const string& key, int v) { m_values[string_to_lower(key)] = v; }
   void SetDisplayPosition(float newValX, float newValY, void* hWnd = nullptr) { /* Not yet implemented */ }
   void ShowSettingsDlg(void* hParentWnd = nullptr) { /* Not yet implemented */ }

private:
   std::unordered_map<string, int> m_values;
};

}
