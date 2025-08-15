#pragma once

#include "../common.h"

namespace B2SLegacy {

class Server;

class PinMAMEAPI {
public:
   PinMAMEAPI(Server* server, ScriptClassDef* pinmameClassDef);
   ~PinMAMEAPI();

   ScriptArray* GetChangedLamps();
   ScriptArray* GetChangedSolenoids();
   ScriptArray* GetChangedGIStrings();
   ScriptArray* GetChangedLEDs();
   void SetSwitch(int switchId, bool value);

   void HandleCall(int memberIndex, int memberStartIndex, ScriptVariant* pArgs, ScriptVariant* pRet);

private:
   Server* m_server;
   ScriptClassDef* m_pinmameClassDef;
   void* m_pinmameInstance;

   int m_changedLampsIndex;
   int m_changedSolenoidsIndex;
   int m_changedGIStringsIndex;
   int m_changedLEDsIndex;
   int m_setSwitchIndex;
};

}