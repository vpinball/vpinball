#include <core/stdafx.h>

#include "VPinball.h"
#include "VPinballLib.h"

static VPinballLib::VPinball& s_vpinstance = VPinballLib::VPinball::GetInstance();

VPINBALLAPI const char* VPinballGetVersionStringFull()
{
   thread_local string version;
   version = s_vpinstance.GetVersionStringFull();
   return version.c_str();
}

VPINBALLAPI void VPinballInit(VPinballEventCallback callback)
{
   auto eventCallback = [callback](VPinballLib::Event event, void* data) -> void* {
      return callback((VPINBALL_EVENT)event, data);
   };

   s_vpinstance.Init(eventCallback);
}

VPINBALLAPI void VPinballLog(VPINBALL_LOG_LEVEL level, const char* pMessage)
{
   s_vpinstance.Log((VPinballLib::LogLevel)level, pMessage);
}

VPINBALLAPI void VPinballResetLog()
{
   s_vpinstance.ResetLog();
}

VPINBALLAPI int VPinballLoadValueInt(VPINBALL_SETTINGS_SECTION section, const char* pKey, int defaultValue)
{
   return s_vpinstance.LoadValueInt((VPinballLib::SettingsSection)section, pKey, defaultValue);
}

VPINBALLAPI float VPinballLoadValueFloat(VPINBALL_SETTINGS_SECTION section, const char* pKey, float defaultValue)
{
   return s_vpinstance.LoadValueFloat((VPinballLib::SettingsSection)section, pKey, defaultValue);
}

VPINBALLAPI const char* VPinballLoadValueString(VPINBALL_SETTINGS_SECTION section, const char* pKey, const char* pDefaultValue)
{
   thread_local string value;
   value = s_vpinstance.LoadValueString((VPinballLib::SettingsSection)section, pKey, pDefaultValue);
   return value.c_str();
}

VPINBALLAPI void VPinballSaveValueInt(VPINBALL_SETTINGS_SECTION section, const char* pKey, int value)
{
   s_vpinstance.SaveValueInt((VPinballLib::SettingsSection)section, pKey, value);
}

VPINBALLAPI void VPinballSaveValueFloat(VPINBALL_SETTINGS_SECTION section, const char* pKey, float value)
{
   s_vpinstance.SaveValueFloat((VPinballLib::SettingsSection)section, pKey, value);
}

VPINBALLAPI void VPinballSaveValueString(VPINBALL_SETTINGS_SECTION section, const char* pKey, const char* pValue)
{
   s_vpinstance.SaveValueString((VPinballLib::SettingsSection)section, pKey, pValue);
}

VPINBALLAPI VPINBALL_STATUS VPinballUncompress(const char* pSource)
{
   return (VPINBALL_STATUS)s_vpinstance.Uncompress(pSource);
}

VPINBALLAPI VPINBALL_STATUS VPinballCompress(const char* pSource, const char* pDestination)
{
   return (VPINBALL_STATUS)s_vpinstance.Compress(pSource, pDestination);
}

VPINBALLAPI void VPinballUpdateWebServer()
{
   s_vpinstance.UpdateWebServer();
}

VPINBALLAPI VPINBALL_STATUS VPinballResetIni()
{
   return (VPINBALL_STATUS)s_vpinstance.ResetIni();
}

VPINBALLAPI VPINBALL_STATUS VPinballLoad(const char* pSource)
{
   return (VPINBALL_STATUS)s_vpinstance.Load(pSource);
}

VPINBALLAPI VPINBALL_STATUS VPinballExtractScript(const char* pSource)
{
   return (VPINBALL_STATUS)s_vpinstance.ExtractScript(pSource);
}

VPINBALLAPI VPINBALL_STATUS VPinballPlay()
{
   return (VPINBALL_STATUS)s_vpinstance.Play();
}

VPINBALLAPI VPINBALL_STATUS VPinballStop()
{
   return (VPINBALL_STATUS)s_vpinstance.Stop();
}

VPINBALLAPI void VPinballSetPlayState(int enable)
{
   s_vpinstance.SetPlayState(enable);
}

VPINBALLAPI void VPinballToggleFPS()
{
   s_vpinstance.ToggleFPS();
}

VPINBALLAPI void VPinballGetTableOptions(VPinballTableOptions* pTableOptions)
{
   if (!pTableOptions)
      return;

   VPinballLib::TableOptions tableOptions;
   s_vpinstance.GetTableOptions(tableOptions);
   pTableOptions->globalEmissionScale = tableOptions.globalEmissionScale;
   pTableOptions->globalDifficulty = tableOptions.globalDifficulty;
   pTableOptions->exposure = tableOptions.exposure;
   pTableOptions->toneMapper = tableOptions.toneMapper;
   pTableOptions->musicVolume = tableOptions.musicVolume;
   pTableOptions->soundVolume = tableOptions.soundVolume;
}

VPINBALLAPI void VPinballSetTableOptions(VPinballTableOptions* pTableOptions)
{
   if (!pTableOptions)
      return;

   VPinballLib::TableOptions options;
   options.globalEmissionScale = pTableOptions->globalEmissionScale;
   options.globalDifficulty = pTableOptions->globalDifficulty;
   options.exposure = pTableOptions->exposure;
   options.toneMapper = pTableOptions->toneMapper;
   options.musicVolume = pTableOptions->musicVolume;
   options.soundVolume = pTableOptions->soundVolume;
   s_vpinstance.SetTableOptions(options);
}

VPINBALLAPI void VPinballSetDefaultTableOptions()
{
   return s_vpinstance.SetDefaultTableOptions();
}

VPINBALLAPI void VPinballResetTableOptions()
{
   return s_vpinstance.ResetTableOptions();
}

VPINBALLAPI void VPinballSaveTableOptions()
{
   return s_vpinstance.SaveTableOptions();
}

VPINBALLAPI int VPinballGetCustomTableOptionsCount()
{
   return s_vpinstance.GetCustomTableOptionsCount();
}

VPINBALLAPI void VPinballGetCustomTableOption(int index, VPinballCustomTableOption* pCustomTableOption)
{
   if (!pCustomTableOption)
      return;

   VPinballLib::CustomTableOption customTableOption;
   s_vpinstance.GetCustomTableOption(index, customTableOption);
   pCustomTableOption->section = (VPINBALL_SETTINGS_SECTION)customTableOption.section;
   pCustomTableOption->id = customTableOption.id;
   pCustomTableOption->name = customTableOption.name;
   pCustomTableOption->showMask = customTableOption.showMask;
   pCustomTableOption->minValue = customTableOption.minValue;
   pCustomTableOption->maxValue = customTableOption.maxValue;
   pCustomTableOption->step = customTableOption.step;
   pCustomTableOption->defaultValue = customTableOption.defaultValue;
   pCustomTableOption->unit = (VPINBALL_OPTION_UNIT)customTableOption.unit;
   pCustomTableOption->literals = customTableOption.literals;
   pCustomTableOption->value = customTableOption.value;
}

VPINBALLAPI void VPinballSetCustomTableOption(VPinballCustomTableOption* pCustomTableOption)
{
   if (!pCustomTableOption)
      return;

   int count = VPinballGetCustomTableOptionsCount();
   for (int i = 0; i < count; ++i) {
      VPinballCustomTableOption existingOption;
      VPinballGetCustomTableOption(i, &existingOption);

      if (strcmp(existingOption.id, pCustomTableOption->id) == 0) {
         VPinballLib::CustomTableOption customTableOption;
         customTableOption.section = (VPinballLib::SettingsSection)existingOption.section;
         customTableOption.id = existingOption.id;
         customTableOption.value = pCustomTableOption->value;
         s_vpinstance.SetCustomTableOption(customTableOption);
         break;
      }
   }
}

VPINBALLAPI void VPinballSetDefaultCustomTableOptions()
{
   return s_vpinstance.SetDefaultCustomTableOptions();
}

VPINBALLAPI void VPinballResetCustomTableOptions()
{
   return s_vpinstance.ResetCustomTableOptions();
}

VPINBALLAPI void VPinballSaveCustomTableOptions()
{
   return s_vpinstance.SaveCustomTableOptions();
}

VPINBALLAPI void VPinballGetViewSetup(VPinballViewSetup* pViewSetup)
{
   if (!pViewSetup)
      return;

   VPinballLib::ViewSetup viewSetup;
   s_vpinstance.GetViewSetup(viewSetup);
   pViewSetup->viewMode = viewSetup.viewMode;
   pViewSetup->sceneScaleX = viewSetup.sceneScaleX;
   pViewSetup->sceneScaleY = viewSetup.sceneScaleY;
   pViewSetup->sceneScaleZ = viewSetup.sceneScaleZ;
   pViewSetup->viewX = viewSetup.viewX;
   pViewSetup->viewY = viewSetup.viewY;
   pViewSetup->viewZ = viewSetup.viewZ;
   pViewSetup->lookAt = viewSetup.lookAt;
   pViewSetup->viewportRotation = viewSetup.viewportRotation;
   pViewSetup->fov = viewSetup.fov;
   pViewSetup->layback = viewSetup.layback;
   pViewSetup->viewHOfs = viewSetup.viewHOfs;
   pViewSetup->viewVOfs = viewSetup.viewVOfs;
   pViewSetup->windowTopZOfs = viewSetup.windowTopZOfs;
   pViewSetup->windowBottomZOfs = viewSetup.windowBottomZOfs;
}

VPINBALLAPI void VPinballSetViewSetup(VPinballViewSetup* pViewSetup)
{
   if (!pViewSetup)
      return;

   VPinballLib::ViewSetup viewSetup;
   viewSetup.viewMode = pViewSetup->viewMode;
   viewSetup.sceneScaleX = pViewSetup->sceneScaleX;
   viewSetup.sceneScaleY = pViewSetup->sceneScaleY;
   viewSetup.sceneScaleZ = pViewSetup->sceneScaleZ;
   viewSetup.viewX = pViewSetup->viewX;
   viewSetup.viewY = pViewSetup->viewY;
   viewSetup.viewZ = pViewSetup->viewZ;
   viewSetup.lookAt = pViewSetup->lookAt;
   viewSetup.viewportRotation = pViewSetup->viewportRotation;
   viewSetup.fov = pViewSetup->fov;
   viewSetup.layback = pViewSetup->layback;
   viewSetup.viewHOfs = pViewSetup->viewHOfs;
   viewSetup.viewVOfs = pViewSetup->viewVOfs;
   viewSetup.windowTopZOfs = pViewSetup->windowTopZOfs;
   viewSetup.windowBottomZOfs = pViewSetup->windowBottomZOfs;
   s_vpinstance.SetViewSetup(viewSetup);
}

VPINBALLAPI void VPinballSetDefaultViewSetup()
{
   return s_vpinstance.SetDefaultViewSetup();
}

VPINBALLAPI void VPinballResetViewSetup()
{
   return s_vpinstance.ResetViewSetup();
}

VPINBALLAPI void VPinballSaveViewSetup()
{
   return s_vpinstance.SaveViewSetup();
}

