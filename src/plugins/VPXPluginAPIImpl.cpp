// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPlugin.h"
#include "VPXPluginAPIImpl.h"

///////////////////////////////////////////////////////////////////////////////
// General information API

void GetTableInfo(VPXTableInfo* info)
{
   assert(g_pplayer); // Only allowed in game
   info->path = g_pplayer->m_ptable->m_szFileName.c_str();
   info->tableWidth = g_pplayer->m_ptable->m_right;
   info->tableHeight = g_pplayer->m_ptable->m_bottom;
}


///////////////////////////////////////////////////////////////////////////////
// User Input API

float GetOption(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step,
   const float defaultValue, const VPXPluginAPI::OptionUnit unit, const char** values)
{
   // TODO handle showMask flag
   // TODO handle core VPX setting pages
   if (strcmp(pageId, VPX_TWEAK_VIEW) == 0)
   {
      return 0.f;
   }
   if (strcmp(pageId, VPX_TWEAK_TABLE) == 0)
   {
      return 0.f;
   }
   else
   {
      Settings& settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
      const std::string sectionName = "Plugin."s + pageId;
      Settings::Section section = settings.GetSection(sectionName);
      std::vector<std::string> literals;
      if (values != nullptr)
      {
         const int nSteps = 1 + (int)(roundf((maxValue - minValue) / step));
         for (int i = 0; i < nSteps; i++)
            literals.push_back(values[i]);
      }
      settings.RegisterSetting(section, optionId, showMask, optionName, minValue, maxValue, step, defaultValue, (Settings::OptionUnit)unit, literals);
      const float value = settings.LoadValueWithDefault(section, optionId, defaultValue);
      return clamp(minValue + step * roundf((value - minValue) / step), minValue, maxValue);
   }
}

void* PushNotification(const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_liveUI->PushNotification(msg, lengthMs);
   // FIXME implement
   return nullptr;
}

void UpdateNotification(const void* handle, const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   // FIXME implement
}


///////////////////////////////////////////////////////////////////////////////
// View API

void DisableStaticPrerendering(const BOOL disable)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_renderer->DisableStaticPrePass(disable);
}

void GetActiveViewSetup(VPXViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   const ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   view->viewMode = viewSetup.mMode;
   view->sceneScaleX = viewSetup.mSceneScaleX;
   view->sceneScaleY = viewSetup.mSceneScaleY;
   view->sceneScaleZ = viewSetup.mSceneScaleZ;
   view->viewX = viewSetup.mViewX;
   view->viewY = viewSetup.mViewY;
   view->viewZ = viewSetup.mViewZ;
   view->lookAt = viewSetup.mLookAt;
   view->viewportRotation = viewSetup.mViewportRotation;
   view->FOV = viewSetup.mFOV;
   view->layback = viewSetup.mLayback;
   view->viewHOfs = viewSetup.mViewHOfs;
   view->viewVOfs = viewSetup.mViewVOfs;
   view->windowTopZOfs = viewSetup.mWindowTopZOfs;
   view->windowBottomZOfs = viewSetup.mWindowBottomZOfs;
   view->screenWidth = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
   view->screenHeight = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
   view->screenInclination = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenInclination"s, 0.0f);
   view->realToVirtualScale = viewSetup.GetRealToVirtualScale(g_pplayer->m_ptable);
}

void SetActiveViewSetup(VPXViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   viewSetup.mViewX = view->viewX;
   viewSetup.mViewY = view->viewY;
   viewSetup.mViewZ = view->viewZ;
   g_pplayer->m_renderer->InitLayout();
}


///////////////////////////////////////////////////////////////////////////////
// Binding between plugin API and VBScript managed COM objects (needed for
// backward compatibility while COM objects are slowly migrated to plugin)

#include "CorePlugin.h"
#include "PinMamePlugin.h"

void VPXPluginAPIImpl::PinMameOnEnd(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   auto msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   unsigned int msg = msgApi.GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END);
   msgApi.BroadcastMsg(pi.m_pinMameEndpointId, msg, nullptr);
   msgApi.ReleaseMsgID(msg);
}

void VPXPluginAPIImpl::PinMameOnStart()
{
   assert(g_pplayer);
   IDispatch* pScriptDispatch = NULL;
   if (g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(NULL, &pScriptDispatch) == S_OK)
   {
      DISPID dispid;
      LPOLESTR name = (LPOLESTR)L"cGameName";
      if (pScriptDispatch->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid) == S_OK)
      {
         DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
         VARIANT varResult;
         VariantInit(&varResult);
         if (pScriptDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispparamsNoArgs, &varResult, NULL, NULL) == S_OK)
         {
            if (varResult.vt == VT_BSTR)
            {
               char buf[MAXTOKEN];
               WideCharToMultiByteNull(CP_ACP, 0, varResult.bstrVal, -1, buf, MAXTOKEN, nullptr, nullptr);
               auto msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
               unsigned int msg = msgApi.GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START);
               msgApi.BroadcastMsg(m_pinMameEndpointId, msg, static_cast<void*>(buf));
               msgApi.ReleaseMsgID(msg);
               msg = msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END);
               msgApi.SubscribeMsg(m_pinMameEndpointId, msg, &PinMameOnEnd, nullptr);
               msgApi.ReleaseMsgID(msg);
            }
         }
      }
      pScriptDispatch->Release();
   }
}

void ControllerOnGetDMD(const unsigned int msgId, void* userData, void* msgData)
{
   GetDmdMsg* msg = static_cast<GetDmdMsg*>(msgData);

   if (g_pplayer == nullptr)
      return;
   if (g_pplayer->m_pStateMappedMem == nullptr || g_pplayer->m_pStateMappedMem->versionID != 1)
      return;
   if (msg->frame != nullptr)
      return;

   PinMame::core_tDisplayState* state = (msg->requestFlags & 1 /* GETDMD_RENDER_FRAME*/) ? g_pplayer->m_pStateMappedMem->displayState : g_pplayer->m_pStateMappedMem->rawDMDState;
   if (state == nullptr)
      return;

   if (msg->dmdId != -1) // TODO implement other DMDs and LED matrices
      return;

   bool found = false;
   PinMame::core_tFrameState* frame = (PinMame::core_tFrameState*)((UINT8*)state + sizeof(PinMame::core_tDisplayState));
   for (unsigned int index = 0; index < state->nDisplays; index++)
   {
      if (frame->width >= 128) // Main DMD
      {
         found = true;
         break;
      }
      frame = (PinMame::core_tFrameState*)((UINT8*)frame + frame->structSize);
   }
   if (!found)
      return;

   msg->frameId = frame->frameId;
   msg->format = frame->dataFormat;
   msg->width = frame->width;
   msg->height = frame->height;
   msg->frame = frame->frameData;
}


///////////////////////////////////////////////////////////////////////////////
// 

VPXPluginAPIImpl& VPXPluginAPIImpl::GetInstance()
{
   static VPXPluginAPIImpl instance;
   return instance;
}

VPXPluginAPIImpl::VPXPluginAPIImpl()
{
   MsgPluginManager::GetInstance().SetSettingsHandler([](const char* name_space, const char* name, char* valueBuf, unsigned int valueBufSize)
      {
         Settings& settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
         const std::string sectionName = "Plugin."s + name_space;
         Settings::Section section = settings.GetSection(sectionName);
         std::vector<std::string> literals;
         std::string buffer;
         valueBuf[0] = 0;
         if (settings.LoadValue(section, name, buffer))
         {
            #ifdef _MSC_VER
            strncpy_s(valueBuf, valueBufSize, buffer.c_str(), valueBufSize);
            #else
            strncpy(valueBuf, buffer.c_str(), valueBufSize);
            #endif
         }
      });

   auto msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   m_vpxEndpointId = MsgPluginManager::GetInstance().NewEndpointId();
   m_pinMameEndpointId = MsgPluginManager::GetInstance().NewEndpointId();
   // VPX API
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &OnGetPluginAPI, nullptr);
   // Generic controller
   msgApi.SubscribeMsg(m_pinMameEndpointId, msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_MSG_GET_DMD), &ControllerOnGetDMD, nullptr);

   m_api.GetTableInfo = GetTableInfo;

   m_api.GetOption = GetOption;
   m_api.PushNotification = PushNotification;
   m_api.UpdateNotification = UpdateNotification;

   m_api.DisableStaticPrerendering = DisableStaticPrerendering;
   m_api.GetActiveViewSetup = GetActiveViewSetup;
   m_api.SetActiveViewSetup = SetActiveViewSetup;
}

void VPXPluginAPIImpl::OnGetPluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   VPXPluginAPI** pResult = static_cast<VPXPluginAPI**>(msgData);
   *pResult = &pi.m_api;
}
