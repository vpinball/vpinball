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
         }else
         {
            PLOGW << "Failed to get cGameName property from script, can't broadcast onGameStart event";
         }
      }
      pScriptDispatch->Release();
   }
}

void VPXPluginAPIImpl::ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData)
{
   if (g_pplayer == nullptr)
      return;

   GetDmdSrcMsg& msg = *static_cast<GetDmdSrcMsg*>(msgData);
   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);
   
   // Report all DMD displays from PinMame state block
   if (g_pplayer->m_pStateMappedMem != nullptr && g_pplayer->m_pStateMappedMem->versionID == 1 && g_pplayer->m_pStateMappedMem->displayState != nullptr)
   {
      PinMame::core_tDisplayState* state = g_pplayer->m_pStateMappedMem->displayState;
      PinMame::core_tFrameState* frame = (PinMame::core_tFrameState*)((UINT8*)state + sizeof(PinMame::core_tDisplayState));
      for (unsigned int index = 0; index < state->nDisplays; index++)
      {
         if (msg.count < msg.maxEntryCount)
         {
            msg.entries[msg.count].dmdId = (me.m_pinMameEndpointId << 16) | frame->displayId;
            msg.entries[msg.count].format = frame->dataFormat;
            msg.entries[msg.count].width = frame->width;
            msg.entries[msg.count].height = frame->height;
            msg.count++;
         }
         frame = (PinMame::core_tFrameState*)((UINT8*)frame + frame->structSize);
      }
   }

   // Report main script DMD (we do not report ancialliary DMD directly set on flashers, but only the main table one)
   // TODO supported RGB frame format are either sRGB888 or sRGB565, not sRGBA8888, therefore RGB frame can not be broadcasted on the plugin bus for the time being
   if (g_pplayer->m_dmdFrame && msg.count < msg.maxEntryCount && g_pplayer->m_dmdFrame->m_format == BaseTexture::BW)
   {
      msg.entries[msg.count].dmdId = (me.m_vpxEndpointId << 16) | 0x00;
      msg.entries[msg.count].format = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888;
      msg.entries[msg.count].width = g_pplayer->m_dmdFrame->width();
      msg.entries[msg.count].height = g_pplayer->m_dmdFrame->height();
      msg.count++;
   }
}

void VPXPluginAPIImpl::ControllerOnGetDMD(const unsigned int msgId, void* userData, void* msgData)
{
   if (g_pplayer == nullptr
    || g_pplayer->m_pStateMappedMem == nullptr
    || g_pplayer->m_pStateMappedMem->versionID != 1)
      return;
      
   GetDmdMsg* msg = static_cast<GetDmdMsg*>(msgData);
   if (msg->frame != nullptr) // Already answered
      return;

   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);

   // PinMame DMD shared through state block
   if ((msg->dmdId >> 16) == me.m_pinMameEndpointId)
   {
      PinMame::core_tDisplayState* state = (msgId == me.m_getIdentifyDmdMsgId) ? g_pplayer->m_pStateMappedMem->rawDMDState : g_pplayer->m_pStateMappedMem->displayState;
      if (state == nullptr)
         return;
      PinMame::core_tFrameState* frame = (PinMame::core_tFrameState*)((UINT8*)state + sizeof(PinMame::core_tDisplayState));
      for (unsigned int index = 0; index < state->nDisplays; index++)
      {
         if ((msg->dmdId & 0x0FFFF) == frame->displayId)
         {
            // If asked for a fixed size render frame with a different size from what we have, don't answer
            if ((msgId == me.m_getRenderDmdMsgId) && (msg->requestFlags & CTLPI_GETDMD_FLAG_RENDER_SIZE_REQ) && ((msg->width != frame->width) || (msg->height != frame->height)))
               return;

            msg->frameId = frame->frameId;
            msg->format = frame->dataFormat;
            msg->width = frame->width;
            msg->height = frame->height;
            msg->frame = frame->frameData;
            return;
         }
         frame = (PinMame::core_tFrameState*)((UINT8*)frame + frame->structSize);
      }
      return;
   }

   // Script DMD
   if (((msg->dmdId >> 16) == me.m_vpxEndpointId) && ((msg->dmdId & 0x0FFFF) == 0) && g_pplayer->m_dmdFrame && g_pplayer->m_dmdFrame->m_format == BaseTexture::BW)
   {
      msg->frameId = g_pplayer->m_dmdFrameId;
      msg->format = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888;
      msg->width = g_pplayer->m_dmdFrame->width();
      msg->height = g_pplayer->m_dmdFrame->height();
      msg->frame = g_pplayer->m_dmdFrame->data();
   }
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
         valueBuf[0] = '\0';
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
   m_getRenderDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   m_getIdentifyDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_IDENTIFY_MSG);
   msgApi.SubscribeMsg(m_pinMameEndpointId, m_getRenderDmdMsgId, &ControllerOnGetDMD, this);
   msgApi.SubscribeMsg(m_pinMameEndpointId, m_getIdentifyDmdMsgId, &ControllerOnGetDMD, this);
   msgApi.SubscribeMsg(m_pinMameEndpointId, msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG), &ControllerOnGetDMDSrc, this);

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
