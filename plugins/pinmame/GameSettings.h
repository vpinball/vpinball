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

   void Clear();
   int GetValue(const string& key) const;
   void SetValue(const string& key, int v);
   // FIXME implement a dynamically typed variant
   //ScriptVariant GetValue(const string& name) const { return {}; /* Not yet implemented */ }
   //void PutValue(const string& name, ScriptVariant v) { /* Not yet implemented */ }

   void SetDisplayPosition(float newValX, float newValY, void* hWnd = nullptr) { LOGW("Game.GameSettings.SetDisplayPosition is deprecated (display position is defined in user settings, not through script)."); }
   void ShowSettingsDlg(void* hParentWnd = nullptr) { LOGW("Game.GameSettings.ShowSettingsDlg is deprecated (settings are managed by the host application, not through script)."); }

private:
   std::unordered_map<string, int> m_values;
};

}
