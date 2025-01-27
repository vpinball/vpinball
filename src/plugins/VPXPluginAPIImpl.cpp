// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPlugin.h"
#include "VPXPluginAPIImpl.h"

///////////////////////////////////////////////////////////////////////////////
// General information API

void VPXPluginAPIImpl::GetTableInfo(VPXTableInfo* info)
{
   assert(g_pplayer); // Only allowed in game
   info->path = g_pplayer->m_ptable->m_szFileName.c_str();
   info->tableWidth = g_pplayer->m_ptable->m_right;
   info->tableHeight = g_pplayer->m_ptable->m_bottom;
}


///////////////////////////////////////////////////////////////////////////////
// User Input API

float VPXPluginAPIImpl::GetOption(const char* pageId, const char* optionId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step,
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

void* VPXPluginAPIImpl::PushNotification(const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_liveUI->PushNotification(msg, lengthMs);
   // FIXME implement
   return nullptr;
}

void VPXPluginAPIImpl::UpdateNotification(const void* handle, const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   // FIXME implement
}


///////////////////////////////////////////////////////////////////////////////
// View API

void VPXPluginAPIImpl::DisableStaticPrerendering(const BOOL disable)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_renderer->DisableStaticPrePass(disable);
}

void VPXPluginAPIImpl::GetActiveViewSetup(VPXViewSetupDef* view)
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

void VPXPluginAPIImpl::SetActiveViewSetup(VPXViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   viewSetup.mViewX = view->viewX;
   viewSetup.mViewY = view->viewY;
   viewSetup.mViewZ = view->viewZ;
   g_pplayer->m_renderer->InitLayout();
}


///////////////////////////////////////////////////////////////////////////////
// Script support for plugin API

void VPXPluginAPIImpl::RegisterScriptClass(ScriptClassDef* classDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptClass(classDef);
}

void VPXPluginAPIImpl::RegisterScriptTypeAlias(const char* name, const char* aliasedType)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptTypeAlias(name, aliasedType);
}

void VPXPluginAPIImpl::RegisterScriptArray(ScriptArrayDef *arrayDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptArray(arrayDef);
}


///////////////////////////////////////////////////////////////////////////////
// API to support overriding legacy COM objects

void VPXPluginAPIImpl::SetCOMObjectOverride(const char* className, const char* pluginId, const char* classId)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   // FIXME remove when plugin is unloaded
   pi.m_scriptCOMObjectOverrides.push_back(ScriptCOMObjectOverride { className, pluginId, classId });
}

#include <regex>
string VPXPluginAPIImpl::ApplyScriptCOMObjectOverrides(string& script) const
{
   if (m_scriptCOMObjectOverrides.empty())
      return script;
   std::regex re(R"(CreateObject\(\s*\"(.*)\"\s*\))");
   std::smatch res;
   string::const_iterator searchStart(script.cbegin());
   std::stringstream result;
   while (std::regex_search(searchStart, script.cend(), res, re))
   {
      result << res.prefix().str();
      const string className = res[1].str();
      auto match = std::find_if(m_scriptCOMObjectOverrides.cbegin(), m_scriptCOMObjectOverrides.cend(), [className](const ScriptCOMObjectOverride& over) { return over.className == className; });
      if (match != m_scriptCOMObjectOverrides.cend())
      {
         PLOGI << "Legacy COM script object " << className << " overriden with class " << match->classId << " from plugin " << match->pluginId;
         result << "CreatePluginObject(\"" << match->pluginId << "\", \"" << match->classId << "\")";
      }
      else
      {
         result << res.str();
      }
      searchStart = res.suffix().first;
   }
   result << std::string(searchStart, script.cend());
   return result.str();
}

IDispatch* VPXPluginAPIImpl::CreateCOMPluginObject(const string& pluginId, const string& classId)
{
   // FIXME we are not separating type library per plugin, therefore collision may occur
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   ScriptTypeNameDef type { classId.c_str(), 0 };
   ScriptClassDef* scriptClass = pi.m_dynamicTypeLibrary.ResolveClass(type);
   void* pScriptObject = scriptClass->CreateObject();
   if (pScriptObject != nullptr)
      return new DynamicDispatch(&pi.m_dynamicTypeLibrary, scriptClass, pScriptObject);
   return nullptr;
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
            msg.entries[msg.count].id = (me.m_pinMameEndpointId << 16) | frame->displayId;
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
      msg.entries[msg.count].id = (me.m_vpxEndpointId << 16) | 0x00;
      msg.entries[msg.count].format = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888;
      msg.entries[msg.count].width = g_pplayer->m_dmdFrame->width();
      msg.entries[msg.count].height = g_pplayer->m_dmdFrame->height();
      msg.count++;
   }
}

void VPXPluginAPIImpl::ControllerOnGetRenderDMD(const unsigned int msgId, void* userData, void* msgData)
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
   if ((msg->dmdId.id >> 16) == me.m_pinMameEndpointId)
   {
      PinMame::core_tDisplayState* state = (msgId == me.m_getIdentifyDmdMsgId) ? g_pplayer->m_pStateMappedMem->rawDMDState : g_pplayer->m_pStateMappedMem->displayState;
      if (state == nullptr)
         return;
      PinMame::core_tFrameState* frame = (PinMame::core_tFrameState*)((UINT8*)state + sizeof(PinMame::core_tDisplayState));
      for (unsigned int index = 0; index < state->nDisplays; index++)
      {
         if ((msg->dmdId.id & 0x0FFFF) == frame->displayId)
         {
            if ((msg->dmdId.width == frame->width) && (msg->dmdId.height == frame->height) && (msg->dmdId.format == frame->dataFormat))
            {
               msg->frameId = frame->frameId;
               msg->frame = frame->frameData;
            }
            return;
         }
         frame = (PinMame::core_tFrameState*)((UINT8*)frame + frame->structSize);
      }
      return;
   }

   // Script DMD
   if (((msg->dmdId.id >> 16) == me.m_vpxEndpointId) && ((msg->dmdId.id & 0x0FFFF) == 0) 
      && (msg->dmdId.format == g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888)
      && (msg->dmdId.width == g_pplayer->m_dmdFrame->width()) && (msg->dmdId.height == g_pplayer->m_dmdFrame->height())
      && g_pplayer->m_dmdFrame && g_pplayer->m_dmdFrame->m_format == BaseTexture::BW) // RGB is not yet supported
   {
      msg->frameId = g_pplayer->m_dmdFrameId;
      msg->frame = g_pplayer->m_dmdFrame->data();
   }
}

void VPXPluginAPIImpl::ControllerOnGetIdentifyDMD(const unsigned int msgId, void* userData, void* msgData)
{
   if (g_pplayer == nullptr || g_pplayer->m_pStateMappedMem == nullptr || g_pplayer->m_pStateMappedMem->versionID != 1)
      return;

   GetRawDmdMsg* msg = static_cast<GetRawDmdMsg*>(msgData);
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
            msg->format == frame->dataFormat;
            msg->width = frame->width;
            msg->height = frame->height;
            msg->frameId = frame->frameId;
            msg->frame = frame->frameData;
            return;
         }
         frame = (PinMame::core_tFrameState*)((UINT8*)frame + frame->structSize);
      }
      return;
   }

   // Script DMD
   if (((msg->dmdId >> 16) == me.m_vpxEndpointId) && ((msg->dmdId & 0x0FFFF) == 0)
      && g_pplayer->m_dmdFrame
      && g_pplayer->m_dmdFrame->m_format == BaseTexture::BW) // RGB is not yet supported
   {
      msg->format = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888;
      msg->width = g_pplayer->m_dmdFrame->width();
      msg->height = g_pplayer->m_dmdFrame->height();
      msg->frameId = g_pplayer->m_dmdFrameId;
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
   // Message host
   auto msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
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

   // VPX API
   m_api.GetTableInfo = GetTableInfo;

   m_api.GetOption = GetOption;
   m_api.PushNotification = PushNotification;
   m_api.UpdateNotification = UpdateNotification;

   m_api.DisableStaticPrerendering = DisableStaticPrerendering;
   m_api.GetActiveViewSetup = GetActiveViewSetup;
   m_api.SetActiveViewSetup = SetActiveViewSetup;

   m_api.SetCOMObjectOverride = SetCOMObjectOverride;

   m_vpxEndpointId = MsgPluginManager::GetInstance().NewEndpointId();
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &OnGetVPXPluginAPI, nullptr);

   // Scriptable API
   m_scriptableApi.RegisterScriptClass = RegisterScriptClass;
   m_scriptableApi.RegisterScriptTypeAlias = RegisterScriptTypeAlias;
   m_scriptableApi.RegisterScriptArrayType = RegisterScriptArray;
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API), &OnGetScriptablePluginAPI, nullptr);

   // Generic controller bridge
   m_pinMameEndpointId = MsgPluginManager::GetInstance().NewEndpointId();
   m_getRenderDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   m_getIdentifyDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_IDENTIFY_MSG);
   msgApi.SubscribeMsg(m_pinMameEndpointId, m_getRenderDmdMsgId, &ControllerOnGetRenderDMD, this);
   msgApi.SubscribeMsg(m_pinMameEndpointId, m_getIdentifyDmdMsgId, &ControllerOnGetIdentifyDMD, this);
   msgApi.SubscribeMsg(m_pinMameEndpointId, msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG), &ControllerOnGetDMDSrc, this);
}

void VPXPluginAPIImpl::OnGetVPXPluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   VPXPluginAPI** pResult = static_cast<VPXPluginAPI**>(msgData);
   *pResult = &pi.m_api;
}

void VPXPluginAPIImpl::OnGetScriptablePluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   ScriptablePluginAPI** pResult = static_cast<ScriptablePluginAPI**>(msgData);
   *pResult = &pi.m_scriptableApi;
}

