#include "core/stdafx.h"

#include "DOFPlugin.h"

void OnDOFLog(DOF_LogLevel logLevel, const char* format, va_list args)
{
   char buffer[4096];
   vsnprintf(buffer, sizeof(buffer), format, args);

   PLOGI.printf("%s", buffer);
}

DOFPlugin::DOFPlugin() : Plugin()
{
   DOF::Config* pConfig = DOF::Config::GetInstance();
   pConfig->SetLogCallback(OnDOFLog);

   m_pDOF = new DOF::DOF();
}

DOFPlugin::~DOFPlugin()
{
   delete m_pDOF;
}

const std::string& DOFPlugin::GetName() const
{
   static const std::string name = "DOFPlugin"s;
   return name;
}

void DOFPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
}

void DOFPlugin::PluginFinish()
{
}

void DOFPlugin::DataReceive(char type, int number, int value)
{
   m_pDOF->DataReceive(type, number, value);
}
