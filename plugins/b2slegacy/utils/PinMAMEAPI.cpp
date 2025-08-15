#include "../common.h"
#include "PinMAMEAPI.h"
#include "LoggingPlugin.h"
#include "../Server.h"

namespace B2SLegacy {

PinMAMEAPI::PinMAMEAPI(Server* server, ScriptClassDef* pinmameClassDef)
   : m_server(server),
     m_pinmameClassDef(pinmameClassDef),
     m_pinmameInstance(nullptr),
     m_changedLampsIndex(-1),
     m_changedSolenoidsIndex(-1),
     m_changedGIStringsIndex(-1),
     m_changedLEDsIndex(-1),
     m_setSwitchIndex(-1)
{
   m_server->SetPinMAMEApi(this);

   if (!m_pinmameClassDef) {
      LOGE("No PinMAME Class Definition");
      return;
   }

   for (unsigned int i = 0; i < m_pinmameClassDef->nMembers; i++) {
      const char* name = m_pinmameClassDef->members[i].name.name;
      unsigned int nArgs = m_pinmameClassDef->members[i].nArgs;

      if (strcmp(name, "ChangedLamps") == 0 && nArgs == 0)
         m_changedLampsIndex = i;
      else if (strcmp(name, "ChangedSolenoids") == 0 && nArgs == 0)
         m_changedSolenoidsIndex = i;
      else if (strcmp(name, "ChangedGIStrings") == 0 && nArgs == 0)
         m_changedGIStringsIndex = i;
      else if (strcmp(name, "ChangedLEDs") == 0 && nArgs == 4)
         m_changedLEDsIndex = i;
      else if (strcmp(name, "Switch") == 0 && nArgs == 2)
         m_setSwitchIndex = i;
   }

   LOGI("PinMAMEAPI: Creating PinMAME Controller instance");
   m_pinmameInstance = m_pinmameClassDef->CreateObject();
}

PinMAMEAPI::~PinMAMEAPI()
{
}

ScriptArray* PinMAMEAPI::GetChangedLamps()
{
   if (m_changedLampsIndex == -1 || !m_pinmameInstance)
      return nullptr;

   ScriptVariant ret;
   ret.vArray = nullptr;
   m_pinmameClassDef->members[m_changedLampsIndex].Call(
      m_pinmameInstance, m_changedLampsIndex, nullptr, &ret);

   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedSolenoids()
{
   if (m_changedSolenoidsIndex == -1 || !m_pinmameInstance)
      return nullptr;

   ScriptVariant ret;
   ret.vArray = nullptr;
   m_pinmameClassDef->members[m_changedSolenoidsIndex].Call(
      m_pinmameInstance, m_changedSolenoidsIndex, nullptr, &ret);

   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedGIStrings()
{
   if (m_changedGIStringsIndex == -1 || !m_pinmameInstance)
      return nullptr;

   ScriptVariant ret;
   ret.vArray = nullptr;
   m_pinmameClassDef->members[m_changedGIStringsIndex].Call(
      m_pinmameInstance, m_changedGIStringsIndex, nullptr, &ret);

   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedLEDs()
{
   if (m_changedLEDsIndex == -1 || !m_pinmameInstance)
      return nullptr;

   ScriptVariant ret;
   ret.vArray = nullptr;
   ScriptVariant args[4];
   args[0].vInt = 0xFFFFFFFF;
   args[1].vInt = 0xFFFFFFFF;
   args[2].vInt = 0;
   args[3].vInt = 0;

   m_pinmameClassDef->members[m_changedLEDsIndex].Call(
      m_pinmameInstance, m_changedLEDsIndex, args, &ret);

   return ret.vArray;
}

void PinMAMEAPI::SetSwitch(int switchId, bool value)
{
   if (m_setSwitchIndex == -1 || !m_pinmameInstance)
      return;

   ScriptVariant args[2];
   args[0].vInt = switchId;
   args[1].vBool = value;

   m_pinmameClassDef->members[m_setSwitchIndex].Call(
      m_pinmameInstance, m_setSwitchIndex, args, nullptr);
}

void PinMAMEAPI::HandleCall(int memberIndex, int memberStartIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   if (!m_pinmameClassDef || !m_pinmameInstance || !m_server)
      return;

   const int index = memberIndex - memberStartIndex;
   const char* methodName = m_pinmameClassDef->members[index].name.name;

   m_pinmameClassDef->members[index].Call(m_pinmameInstance, index, pArgs, pRet);

   if (strcmp(methodName, "GameName") == 0) {
      if (pArgs) {
         string gameName;
         if (pArgs[0].vString.string)
            gameName = string(pArgs[0].vString.string);
         LOGI("B2SLegacy: Setting GameName to '%s' in B2S settings", gameName.c_str());
         m_server->GetB2SSettings()->SetGameName(gameName);
         m_server->GetB2SSettings()->SetB2SName("");
      }
   }
   else if (strcmp(methodName, "Run") == 0)
      m_server->Run(0);
   else if (strcmp(methodName, "Stop") == 0)
      m_server->Stop();
   else if (strcmp(methodName, "ChangedLamps") == 0)
      m_server->GetChangedLamps(pRet);
   else if (strcmp(methodName, "ChangedSolenoids") == 0)
      m_server->GetChangedSolenoids(pRet);
   else if (strcmp(methodName, "ChangedGIStrings") == 0)
      m_server->GetChangedGIStrings(pRet);
   else if (strcmp(methodName, "ChangedLEDs") == 0)
      m_server->GetChangedLEDs(pRet);
   else if (strcmp(methodName, "GetMech") == 0) {
      if (pArgs && pRet)
         m_server->CheckGetMech(pArgs[0].vInt, pRet->vInt);
   }
}

}