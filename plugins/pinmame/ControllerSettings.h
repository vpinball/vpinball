#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

class ControllerSettings final
{
public:
   ControllerSettings();
   ~ControllerSettings();

   PSC_IMPLEMENT_REFCOUNT()

   //STDMETHOD(get_InstallDir)(/*[out, retval]*/ BSTR *pVal);
   //STDMETHOD(get_Value)(/*[in]*/ BSTR sName, /*[out, retval]*/ VARIANT *pVal);
   //STDMETHOD(put_Value)(/*[in]*/ BSTR sName, /*[in]*/ VARIANT newVal);
   //STDMETHOD(Clear)();
   //STDMETHOD(ShowSettingsDlg)(LONG_PTR hParentWnd);
};
