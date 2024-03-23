#pragma once

#include "../b2s/plugin/Plugin.h"

#include "DOF/DOF.h"

class DOFPlugin : public Plugin
{
public:
   DOFPlugin();
   ~DOFPlugin();

   const std::string& GetName() const override;
   void PluginInit(const string& szTableFilename, const string& szRomName) override;
   void PluginFinish() override;
   void DataReceive(char type, int number, int value) override;

private:
   DOF::DOF* m_pDOF;
};