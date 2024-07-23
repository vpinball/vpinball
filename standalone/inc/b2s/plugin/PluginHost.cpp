#include "stdafx.h"

#include "PluginHost.h"
#include "Plugin.h"

PluginHost* PluginHost::m_pInstance = NULL;

PluginHost* PluginHost::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new PluginHost();

   return m_pInstance;
}

PluginHost::PluginHost()
{
}

PluginHost::~PluginHost()
{
}

void PluginHost::RegisterPlugin(Plugin* pPlugin)
{
   if (!pPlugin)
      return;

   PLOGI.printf("Registering plugin: name=%s", pPlugin->GetName().c_str());

   m_plugins.push_back(pPlugin);
}

void PluginHost::UnregisterAllPlugins()
{
   for (Plugin* pPlugin : m_plugins)
      delete pPlugin;

   m_plugins.clear();
}

void PluginHost::PluginInit(const string& szTableFilename, const string& szRomName)
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PluginInit(szTableFilename, szRomName);
   }
}

void PluginHost::PluginFinish()
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PluginFinish();
   }
}

void PluginHost::PinMameRun()
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PinMameRun();
   }
}

void PluginHost::PinMamePause()
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PinMamePause();
   }
}

void PluginHost::PinMameContinue()
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PinMameContinue();
   }
}

void PluginHost::PinMameStop()
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->PinMameStop();
   }
}

void PluginHost::DataReceive(char type, int number, int value)
{
   for (Plugin* pPlugin : m_plugins) {
      if (pPlugin->GetStatus() == PluginStatusEnum_Active)
         pPlugin->DataReceive(type, number, value);
   }
}

void PluginHost::DataReceive(char type, SAFEARRAY* psa)
{
   if (!psa)
      return;

   LONG uCount = 0;
   LONG lBound;
   LONG uBound;

   if (SUCCEEDED(SafeArrayGetLBound(psa, 1, &lBound))) {
      if (SUCCEEDED(SafeArrayGetUBound(psa, 1, &uBound)))
         uCount = uBound - lBound + 1;
   }

   LONG ix[2];
   VARIANT varValue;
   int number;
   int value;

   if (type == 'D') {
      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         int number = V_I4(&varValue);
         VariantClear(&varValue);

         ix[1] = 2;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         int value = V_I4(&varValue);
         VariantClear(&varValue);

         for (Plugin* pPlugin : m_plugins) {
            if (pPlugin->GetStatus() == PluginStatusEnum_Active) {
               pPlugin->DataReceive(type, number, value);
            }
         }
      }
   }
   else {
      for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
         ix[1] = 0;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         number = V_I4(&varValue);
         VariantClear(&varValue);

         ix[1] = 1;
         VariantInit(&varValue);
         SafeArrayGetElement(psa, ix, &varValue);
         value = V_I4(&varValue);
         VariantClear(&varValue);

         for (Plugin* pPlugin : m_plugins) {
            if (pPlugin->GetStatus() == PluginStatusEnum_Active) {
               pPlugin->DataReceive(type, number, value);
            }
         }
      }
   }
}

void PluginHost::DisablePup()
{
   for (Plugin* pPlugin : m_plugins) {
      if (string_contains_case_insensitive(pPlugin->GetName(), "pinup")) {
         pPlugin->PluginFinish();
         pPlugin->SetStatus(PluginStatusEnum_Disabled);
      }
   }
}