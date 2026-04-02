#include "../common.h"
#include "PinMAMEAPI.h"
#include "plugins/LoggingPlugin.h"
#include "../Server.h"

namespace B2SLegacy {

PinMAMEAPI::PinMAMEAPI(MsgPluginAPI* msgApi, uint32_t endpointId, Server* server, ScriptClassDef* serverClassDef)
   : m_server(server),
     m_serverClassDef(serverClassDef),
     m_controllerClassProxy(msgApi, endpointId, "PinMAME_", "PinMAME_Controller", "B2SLegacy_", serverClassDef),
     m_controllerProxy(m_controllerClassProxy),
     m_changedLampsIndex(-1),
     m_changedSolenoidsIndex(-1),
     m_changedGIStringsIndex(-1),
     m_changedLEDsIndex(-1),
     m_setSwitchIndex(-1)
{
   for (unsigned int i = 0; i < serverClassDef->nMembers; i++) {
      const char* name = serverClassDef->members[i].name.name;
      unsigned int nArgs = serverClassDef->members[i].nArgs;

      if (name == "ChangedLamps"sv && nArgs == 0)
         m_changedLampsIndex = i;
      else if (name == "ChangedSolenoids"sv && nArgs == 0)
         m_changedSolenoidsIndex = i;
      else if (name == "ChangedGIStrings"sv && nArgs == 0)
         m_changedGIStringsIndex = i;
      else if (name == "ChangedLEDs"sv && nArgs == 4)
         m_changedLEDsIndex = i;
      else if (name == "Switch"sv && nArgs == 2)
         m_setSwitchIndex = i;
   }
   
   assert(m_changedLampsIndex >= 0);
   assert(m_changedSolenoidsIndex >= 0);
   assert(m_changedGIStringsIndex >= 0);
   assert(m_changedLEDsIndex >= 0);
   assert(m_setSwitchIndex >= 0);
}

PinMAMEAPI::~PinMAMEAPI()
{
}

ScriptArray* PinMAMEAPI::GetChangedLamps()
{
   ScriptVariant ret;
   ret.vArray = nullptr;
   m_controllerProxy.ForwardCall(m_server, m_changedLampsIndex, nullptr, &ret);
   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedSolenoids()
{
   ScriptVariant ret;
   ret.vArray = nullptr;
   m_controllerProxy.ForwardCall(m_server, m_changedSolenoidsIndex, nullptr, &ret);
   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedGIStrings()
{
   ScriptVariant ret;
   ret.vArray = nullptr;
   m_controllerProxy.ForwardCall(m_server, m_changedGIStringsIndex, nullptr, &ret);
   return ret.vArray;
}

ScriptArray* PinMAMEAPI::GetChangedLEDs()
{
   ScriptVariant ret;
   ret.vArray = nullptr;
   ScriptVariant args[4];
   args[0].vInt = 0xFFFFFFFF;
   args[1].vInt = 0xFFFFFFFF;
   args[2].vInt = 0;
   args[3].vInt = 0;
   m_controllerProxy.ForwardCall(m_server, m_changedLEDsIndex, args, &ret);
   return ret.vArray;
}

void PinMAMEAPI::SetSwitch(int switchId, bool value)
{
   ScriptVariant args[2];
   args[0].vInt = switchId;
   args[1].vBool = value ? 1 : 0;
   m_controllerProxy.ForwardCall(m_server, m_setSwitchIndex, args, nullptr);
}

void PinMAMEAPI::HandleCall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   const char* methodName = m_serverClassDef->members[memberIndex].name.name;

   m_controllerProxy.ForwardCall(m_server, memberIndex, pArgs, pRet);

   if (methodName == "GameName"sv) {
      if (pArgs) {
         string gameName;
         if (pArgs[0].vString.string)
            gameName = pArgs[0].vString.string;
         LOGI("B2SLegacy: Setting GameName to '" + gameName + "' in B2S settings");
         m_server->GetB2SSettings()->SetGameName(gameName);
         m_server->GetB2SSettings()->SetB2SName(""s);
      }
   }
   else if (methodName == "Run"sv)
      m_server->Run(0);
   else if (methodName == "Stop"sv)
      m_server->Stop();
   else if (methodName == "ChangedLamps"sv)
      m_server->GetChangedLamps(pRet);
   else if (methodName == "ChangedSolenoids"sv)
      m_server->GetChangedSolenoids(pRet);
   else if (methodName == "ChangedGIStrings"sv)
      m_server->GetChangedGIStrings(pRet);
   else if (methodName == "ChangedLEDs"sv)
      m_server->GetChangedLEDs(pRet);
   else if (methodName == "GetMech"sv) {
      if (pArgs && pRet)
         m_server->CheckGetMech(pArgs[0].vInt, pRet->vInt);
   }
}

}
