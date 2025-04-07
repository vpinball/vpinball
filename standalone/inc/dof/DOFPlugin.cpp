#include "core/stdafx.h"

#include "DOFPlugin.h"

void OnDOFLog(DOF_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      if (logLevel == DOF_LogLevel_INFO) {
         PLOGI << buffer;
      }
      else if (logLevel == DOF_LogLevel_ERROR) {
         PLOGE << buffer;
      }
      free(buffer);
   }
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
