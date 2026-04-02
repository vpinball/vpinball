// license:GPLv3+

#include "common.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/LoggingPlugin.h"
#include "plugins/VPXPlugin.h"

#include "WMPCore.h"
#include "WMPControls.h"  
#include "WMPSettings.h"

namespace WMP {

#define PSC_VAR_SET_WMP_Core(variant, value) PSC_VAR_SET_object(WMPCore, variant, value)
#define PSC_VAR_SET_WMP_Controls(variant, value) PSC_VAR_SET_object(WMPControls, variant, value)
#define PSC_VAR_SET_WMP_Settings(variant, value) PSC_VAR_SET_object(WMPSettings, variant, value)

PSC_CLASS_START(WMP_Core, WMPCore)
   PSC_FUNCTION0(void, Close)
   PSC_PROP_RW(string, URL)
   PSC_PROP_R(int32, OpenState)
   PSC_PROP_R(int32, PlayState)
   PSC_PROP_R(WMP_Controls, Controls)
   PSC_PROP_R(WMP_Settings, Settings)
   PSC_PROP_R(string, VersionInfo)
   PSC_PROP_R(bool, IsOnline)
   PSC_PROP_R(string, Status)
PSC_CLASS_END()

PSC_CLASS_START(WMP_Controls, WMPControls)
   PSC_FUNCTION0(void, Play)
   PSC_FUNCTION0(void, Stop)
   PSC_FUNCTION0(void, Pause)
   PSC_FUNCTION0(void, FastForward)
   PSC_FUNCTION0(void, FastReverse)
   PSC_PROP_RW(double, CurrentPosition)
   PSC_PROP_R(string, CurrentPositionString)
   PSC_FUNCTION0(void, Next)
   PSC_FUNCTION0(void, Previous)
   PSC_FUNCTION1(bool, GetIsAvailable, string)
PSC_CLASS_END()

PSC_CLASS_START(WMP_Settings, WMPSettings)
   PSC_PROP_RW(bool, AutoStart)
   PSC_PROP_RW(bool, Mute)
   PSC_PROP_RW(int32, Volume)
   PSC_PROP_RW(double, Rate)
   PSC_PROP_RW(int32, Balance)
   PSC_PROP_RW(int32, PlayCount)
   PSC_PROP_RW(string, BaseURL)
   PSC_PROP_RW(string, DefaultFrame)
   PSC_PROP_RW(bool, InvokeURLs)
   PSC_PROP_RW(bool, EnableErrorDialogs)
   PSC_FUNCTION1(bool, GetIsAvailable, string)
   PSC_FUNCTION1(bool, GetMode, string)
   PSC_FUNCTION2(void, SetMode, string, bool)
PSC_CLASS_END()

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId = 0;
static unsigned int onAudioUpdateId = 0;

PSC_ERROR_IMPLEMENT(scriptApi);

LPI_IMPLEMENT_CPP // Implement shared log support

}

using namespace WMP;

MSGPI_EXPORT void MSGPIAPI WMPPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   endpointId = sessionId;
   msgApi = const_cast<MsgPluginAPI*>(api);

   LPISetup(endpointId, msgApi);

   const unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);

   if (scriptApi == nullptr) {
      LOGE("Failed to get script API"s);
      return;
   }

   auto regLambda = [](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   RegisterWMP_Core(regLambda);
   RegisterWMP_Controls(regLambda);
   RegisterWMP_Settings(regLambda);

   scriptApi->SubmitTypeLibrary(endpointId);

   WMP_Core_SCD->CreateObject = []() -> void*
   {
      return static_cast<void*>(new WMPCore(msgApi, endpointId, onAudioUpdateId));
   };

   scriptApi->SetCOMObjectOverride("WMPlayer.OCX", WMP_Core_SCD);

   LOGI("WMP Plugin loaded successfully"s);
}

MSGPI_EXPORT void MSGPIAPI WMPPluginUnload()
{
   if (scriptApi != nullptr)
   {
      scriptApi->SetCOMObjectOverride("WMPlayer.OCX", nullptr);
      auto regLambda = [](ScriptClassDef* scd) { scriptApi->UnregisterScriptClass(scd); };
      UnregisterWMP_Core(regLambda);
      UnregisterWMP_Controls(regLambda);
      UnregisterWMP_Settings(regLambda);
      scriptApi = nullptr;
   }

   if (msgApi && onAudioUpdateId != 0) {
      msgApi->ReleaseMsgID(onAudioUpdateId);
      onAudioUpdateId = 0;
   }

   vpxApi = nullptr;
   msgApi = nullptr;

   LOGI("WMP Plugin unloaded"s);
}
