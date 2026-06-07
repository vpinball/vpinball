// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include <cstring>
#include <cstdlib>

namespace HelloScript {

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static unsigned int getVpxApiMsgId, getScriptApiMsgId;

static uint32_t endpointId;

static float property2 = 42.5f;

void get_Property1(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { pRet->vInt = 42; }
void get_Property2(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { pRet->vFloat = property2; }
void put_Property2(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { property2 = pArgs[0].vFloat; }
void AddRef(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { }
void Release(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { }

// Example: return a 1D string array with 3 elements.
// String elements are packed as ScriptString values, each carrying its own Release hook so the
// host frees plugin-owned strings through the documented per-element lifecycle.
void get_StringArray1D(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   static const char* strings[] = { "hello", "world", "vpx" };
   const unsigned int count = 3;
   const size_t dataSize = count * sizeof(ScriptString);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 1 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = count;
   ScriptString* pData = reinterpret_cast<ScriptString*>(&array->lengths[1]);
   for (unsigned int i = 0; i < count; i++)
   {
      const size_t n = strlen(strings[i]) + 1;
      char* s = new char[n];
      memcpy(s, strings[i], n);
      pData[i] = { [](ScriptString* str) { delete[] str->string; }, s };
   }
   pRet->vArray = array;
}

// Example: return a 2D string array (3 rows x 2 cols) — e.g. ID/value pairs
void get_StringArray2D(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   static const char* strings[] = { "l-1", "on", "l-2", "off", "coil1", "pulse" };
   const unsigned int rows = 3, cols = 2;
   const size_t dataSize = rows * cols * sizeof(ScriptString);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 2 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = rows;
   array->lengths[1] = cols;
   ScriptString* pData = reinterpret_cast<ScriptString*>(&array->lengths[2]);
   for (unsigned int i = 0; i < rows * cols; i++)
   {
      const size_t n = strlen(strings[i]) + 1;
      char* s = new char[n];
      memcpy(s, strings[i], n);
      pData[i] = { [](ScriptString* str) { delete[] str->string; }, s };
   }
   pRet->vArray = array;
}

// Example: return a 1D bool array with 4 elements
void get_BoolArray1D(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   const unsigned int count = 4;
   const size_t dataSize = count * sizeof(char);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 1 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = count;
   char* pData = reinterpret_cast<char*>(&array->lengths[1]);
   pData[0] = 1;
   pData[1] = 0;
   pData[2] = 1;
   pData[3] = 1;
   pRet->vArray = array;
}

// Example: return a 2D bool array (3 rows x 2 cols)
void get_BoolArray2D(void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   const unsigned int rows = 3, cols = 2;
   const size_t dataSize = rows * cols * sizeof(char);
   ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + 2 * sizeof(unsigned int) + dataSize));
   array->Release = [](ScriptArray* me) { free(me); };
   array->lengths[0] = rows;
   array->lengths[1] = cols;
   char* pData = reinterpret_cast<char*>(&array->lengths[2]);
   pData[0] = 1; pData[1] = 0;
   pData[2] = 0; pData[3] = 1;
   pData[4] = 1; pData[5] = 1;
   pRet->vArray = array;
}

// Array type definitions for string and bool arrays
static ScriptArrayDef stringArray1DDef = { { "HelloStringArray1D" }, { "string" }, 1, { 0 } };
static ScriptArrayDef stringArray2DDef = { { "HelloStringArray2D" }, { "string" }, 2, { 0, 0 } };
static ScriptArrayDef boolArray1DDef = { { "HelloBoolArray1D" }, { "bool" }, 1, { 0 } };
static ScriptArrayDef boolArray2DDef = { { "HelloBoolArray2D" }, { "bool" }, 2, { 0, 0 } };

ScriptClassDef helloScriptClass { { "DummyClass" }, []() { return static_cast<void*>(new int[4]); }, 9,
   {
      { { "AddRef" },        { "ulong" },               0, {}, AddRef  },
      { { "Release" },       { "ulong" },               0, {}, Release },
      { { "Property1" },     { "int" },                 0, {}, get_Property1 },
      { { "Property2" },     { "float" },               0, {}, get_Property2 },
      { { "Property2" },     { "void" },                1, { { "float" } }, put_Property2 },
      { { "StringArray1D" }, { "HelloStringArray1D" },   0, {}, get_StringArray1D },
      { { "StringArray2D" }, { "HelloStringArray2D" },   0, {}, get_StringArray2D },
      { { "BoolArray1D" },   { "HelloBoolArray1D" },     0, {}, get_BoolArray1D },
      { { "BoolArray2D" },   { "HelloBoolArray2D" },     0, {}, get_BoolArray2D },
   } };

}

using namespace HelloScript;

MSGPI_EXPORT void MSGPIAPI HelloScriptPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   getVpxApiMsgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiMsgId, &vpxApi);
   getScriptApiMsgId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiMsgId, &scriptApi);
   scriptApi->RegisterScriptArrayType(&stringArray1DDef);
   scriptApi->RegisterScriptArrayType(&stringArray2DDef);
   scriptApi->RegisterScriptArrayType(&boolArray1DDef);
   scriptApi->RegisterScriptArrayType(&boolArray2DDef);
   scriptApi->RegisterScriptClass(&helloScriptClass);
   scriptApi->SubmitTypeLibrary(endpointId);
}

MSGPI_EXPORT void MSGPIAPI HelloScriptPluginUnload()
{
   scriptApi->UnregisterScriptClass(&helloScriptClass);
   scriptApi->UnregisterScriptArrayType(&stringArray1DDef);
   scriptApi->UnregisterScriptArrayType(&stringArray2DDef);
   scriptApi->UnregisterScriptArrayType(&boolArray1DDef);
   scriptApi->UnregisterScriptArrayType(&boolArray2DDef);
   msgApi->ReleaseMsgID(getVpxApiMsgId);
   msgApi->ReleaseMsgID(getScriptApiMsgId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
