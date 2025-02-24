// license:GPLv3+

#include "MsgPlugin.h"
#include "VPXPlugin.h"
#include "ScriptablePlugin.h"
#include <cstring>

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;

static uint32_t endpointId;

static float property2 = 42.5f;

void get_Property1(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { pRet->vInt = 42; }
void get_Property2(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { pRet->vFloat = property2; }
void put_Property2(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { property2 = pArgs[0].vFloat; }
void AddRef(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { }
void Release(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { }

ScriptClassDef helloScriptClass { { "DummyClass" }, []() { return static_cast<void*>(new int[4]); }, 4,
   {
      { { "AddRef" },    { "ulong" }, 0, {}, AddRef  },
      { { "Release" },   { "ulong" }, 0, {}, Release },
      { { "Property1" }, { "int" },   0, {}, get_Property1 },
      { { "Property2" }, { "float" }, 0, {}, get_Property2 },
      { { "Property2" }, { "void" }, 1, { { "float" } }, put_Property2 },
   } };

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   const unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);
   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);
   scriptApi->RegisterScriptClass(&helloScriptClass);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   vpxApi = nullptr;
   msgApi = nullptr;
}
