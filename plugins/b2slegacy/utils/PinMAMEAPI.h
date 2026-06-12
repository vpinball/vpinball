#pragma once

#include "../common.h"

namespace B2SLegacy {

class Server;

class PinMAMEAPI {
public:
   PinMAMEAPI(MsgPluginAPI* msgApi, uint32_t endpointId, Server* server, ScriptClassDef* serverClassDef);
   ~PinMAMEAPI();

   ScriptArray* GetChangedLamps();
   ScriptArray* GetChangedSolenoids();
   ScriptArray* GetChangedGIStrings();
   ScriptArray* GetChangedLEDs();
   void SetSwitch(int switchId, bool value);

   void HandleCall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet);

private:
   Server* m_server = nullptr;
   ScriptClassDef* m_serverClassDef = nullptr;
   ScriptablePlugin::ScriptClassProxy m_controllerClassProxy;
   ScriptablePlugin::ScriptObjectProxy m_controllerProxy;

   int m_changedLampsIndex = -1;
   int m_changedSolenoidsIndex = -1;
   int m_changedGIStringsIndex = -1;
   int m_changedLEDsIndex = -1;
   int m_setSwitchIndex = -1;
};

}
