// license:GPLv3+

#include "MsgPlugin.h"
#include "VPXPlugin.h"
#include "ScriptablePlugin.h"
#include <string.h>

MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;
ScriptablePluginAPI* scriptApi = nullptr;

unsigned int endpointId;

void get_Property1(void* me, int, ScriptVariant* value) { value->vInt = 42; }
void get_Property2(void* me, int, ScriptVariant* value) { value->vFloat = 42.5f; }
void put_Property2(void* me, int, ScriptVariant* value) { }
void AddRef(void* me, int, ScriptVariant*, ScriptVariant*) { }
void Release(void* me, int, ScriptVariant*, ScriptVariant*) { }

ScriptClassDef helloScriptClass { { "DummyClass" }, []() { return static_cast<void*>(new int[4]); }, 4,
   {
      { { "AddRef" },    { "ulong" }, nullptr,       nullptr,       AddRef  },
      { { "Release" },   { "ulong" }, nullptr,       nullptr,       Release },
      { { "Property1" }, { "int" },   get_Property1, nullptr,       nullptr },
      { { "Property2" }, { "float" }, get_Property2, put_Property2, nullptr }
   } };

MSGPI_EXPORT void PluginLoad(const unsigned int sessionId, MsgPluginAPI* api)
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

MSGPI_EXPORT void PluginUnload()
{
   vpxApi = nullptr;
   msgApi = nullptr;
}
