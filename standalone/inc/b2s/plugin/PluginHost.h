#pragma once

class Plugin;

class PluginHost final
{
public:
   ~PluginHost();

   static PluginHost* GetInstance();

   void RegisterPlugin(Plugin* pPlugin);
   void UnregisterAllPlugins();

   void PluginInit(const string& szTableFilename, const string& szRomName);
   void PluginFinish();
   void PinMameRun();
   void PinMamePause();
   void PinMameContinue();
   void PinMameStop();
   void DataReceive(char type, int number, int value);
   void DataReceive(char type, SAFEARRAY* psa);
   void DisablePup();

private:
   PluginHost();

   static PluginHost* m_pInstance;
   vector<Plugin*> m_plugins;
};