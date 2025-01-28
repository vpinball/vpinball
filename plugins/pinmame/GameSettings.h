#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

class GameSettings
{
public:
   GameSettings();
   ~GameSettings();

   PSC_IMPLEMENT_REFCOUNT()

   void Clear() { /* Not yet implemented */ }
   // FIXME implement a dynamically typed variant
   ScriptVariant GetValue(const string& name) const { return {}; /* Not yet implemented */ }
   void PutValue(const string& name, ScriptVariant v) { /* Not yet implemented */ }
   void SetDisplayPosition(float newValX, float newValY, void* hWnd = nullptr) { /* Not yet implemented */ }
   void ShowSettingsDlg(void* hParentWnd = nullptr) { /* Not yet implemented */ }
};
