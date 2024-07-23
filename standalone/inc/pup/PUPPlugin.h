#pragma once

#include "../b2s/plugin/Plugin.h"

class PUPManager;

class PUPPlugin : public Plugin
{
public:
   PUPPlugin();
   ~PUPPlugin();

   const std::string& GetName() const override;
   void PluginInit(const string& szTableFilename, const string& szRomName) override;
   void PluginFinish() override;
   void DataReceive(char type, int number, int value) override;
   
private:
   PUPManager* m_pManager;
};