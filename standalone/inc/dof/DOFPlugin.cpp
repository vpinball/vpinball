#include "stdafx.h"

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
      switch(logLevel) {
         case DOF_LogLevel_INFO:
            PLOGI << buffer;
            break;
         case DOF_LogLevel_WARN:
            PLOGW << buffer;
            break;
         case DOF_LogLevel_ERROR:
            PLOGE << buffer;
            break;
         default:
            break;
      }
      free(buffer);
   }
}

DOFPlugin::DOFPlugin() : Plugin()
{
   DOF::Config* pConfig = DOF::Config::GetInstance();
   pConfig->SetLogCallback(OnDOFLog);
   pConfig->SetBasePath(g_pvp->m_szMyPrefPath.c_str());

   m_pDOF = new DOF::DOF();
}

DOFPlugin::~DOFPlugin()
{
   if (m_pDOF)
      delete m_pDOF;
}

const std::string& DOFPlugin::GetName() const
{
   static std::string name = "DOFPlugin";
   return name;
}

void DOFPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
   m_pDOF->Init(szTableFilename.c_str(), szRomName.c_str());
}

void DOFPlugin::PluginFinish()
{
   m_pDOF->Finish();
}

void DOFPlugin::DataReceive(char type, int number, int value)
{
   m_pDOF->DataReceive(type, number, value);
}
