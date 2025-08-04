// license:GPLv3+

#include "common.h"
#include "ScriptablePlugin.h"
#include "LoggingPlugin.h"
#include "VPXPlugin.h"

#include "WMPCore.h"
#include "WMPControls.h"  
#include "WMPSettings.h"

namespace WMP {

#define PSC_VAR_SET_WMPCore(variant, value) PSC_VAR_SET_object(WMPCore, variant, value)
#define PSC_VAR_SET_WMPControls(variant, value) PSC_VAR_SET_object(WMPControls, variant, value)
#define PSC_VAR_SET_WMPSettings(variant, value) PSC_VAR_SET_object(WMPSettings, variant, value)

PSC_CLASS_START(WMPCore)
   PSC_FUNCTION0(WMPCore, void, Close)
   PSC_PROP_RW(WMPCore, string, URL)
   PSC_PROP_R(WMPCore, int32, OpenState)
   PSC_PROP_R(WMPCore, int32, PlayState)
   PSC_PROP_R(WMPCore, WMPControls, Controls)
   PSC_PROP_R(WMPCore, WMPSettings, Settings)
   PSC_PROP_R(WMPCore, string, VersionInfo)
   PSC_PROP_R(WMPCore, bool, IsOnline)
   PSC_PROP_R(WMPCore, string, Status)
PSC_CLASS_END(WMPCore)

PSC_CLASS_START(WMPControls)
   PSC_FUNCTION0(WMPControls, void, Play)
   PSC_FUNCTION0(WMPControls, void, Stop)
   PSC_FUNCTION0(WMPControls, void, Pause)
   PSC_FUNCTION0(WMPControls, void, FastForward)
   PSC_FUNCTION0(WMPControls, void, FastReverse)
   PSC_PROP_RW(WMPControls, double, CurrentPosition)
   PSC_PROP_R(WMPControls, string, CurrentPositionString)
   PSC_FUNCTION0(WMPControls, void, Next)
   PSC_FUNCTION0(WMPControls, void, Previous)
   PSC_FUNCTION1(WMPControls, bool, GetIsAvailable, string)
PSC_CLASS_END(WMPControls)

PSC_CLASS_START(WMPSettings)
   PSC_PROP_RW(WMPSettings, bool, AutoStart)
   PSC_PROP_RW(WMPSettings, bool, Mute)
   PSC_PROP_RW(WMPSettings, int32, Volume)
   PSC_PROP_RW(WMPSettings, double, Rate)
   PSC_PROP_RW(WMPSettings, int32, Balance)
   PSC_PROP_RW(WMPSettings, int32, PlayCount)
   PSC_PROP_RW(WMPSettings, string, BaseURL)
   PSC_PROP_RW(WMPSettings, string, DefaultFrame)
   PSC_PROP_RW(WMPSettings, bool, InvokeURLs)
   PSC_PROP_RW(WMPSettings, bool, EnableErrorDialogs)
   PSC_FUNCTION1(WMPSettings, bool, GetIsAvailable, string)
   PSC_FUNCTION1(WMPSettings, bool, GetMode, string)
   PSC_FUNCTION2(WMPSettings, void, SetMode, string, bool)
PSC_CLASS_END(WMPSettings)

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId = 0;
static unsigned int onAudioUpdateId = 0;

PSC_ERROR_IMPLEMENT(scriptApi);

LPI_IMPLEMENT

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
      LOGE("Failed to get script API");
      return;
   }

   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   RegisterWMPCoreSCD(regLambda);
   RegisterWMPControlsSCD(regLambda);
   RegisterWMPSettingsSCD(regLambda);

   scriptApi->SubmitTypeLibrary();

   WMPCore_SCD->CreateObject = []() -> void*
   {
      return static_cast<void*>(new WMPCore(msgApi, endpointId, onAudioUpdateId));
   };

   scriptApi->SetCOMObjectOverride("WMPlayer.OCX", WMPCore_SCD);

   LOGI("WMP Plugin loaded successfully");
}

MSGPI_EXPORT void MSGPIAPI WMPPluginUnload()
{
   if (scriptApi != nullptr)
      scriptApi->SetCOMObjectOverride("WMPlayer.OCX", nullptr);

   if (msgApi && onAudioUpdateId != 0) {
      msgApi->ReleaseMsgID(onAudioUpdateId);
      onAudioUpdateId = 0;
   }

   vpxApi = nullptr;
   scriptApi = nullptr;
   msgApi = nullptr;

   LOGI("WMP Plugin unloaded");
}
