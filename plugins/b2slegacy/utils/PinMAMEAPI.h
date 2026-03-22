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
   Server* m_server;
   ScriptClassDef* m_serverClassDef;
   ScriptablePlugin::ScriptClassProxy m_controllerClassProxy;
   ScriptablePlugin::ScriptObjectProxy m_controllerProxy;

   int m_changedLampsIndex;
   int m_changedSolenoidsIndex;
   int m_changedGIStringsIndex;
   int m_changedLEDsIndex;
   int m_setSwitchIndex;
};

}
