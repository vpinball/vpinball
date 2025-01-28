// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPlugin.h"
#include "VPXPluginAPIImpl.h"

///////////////////////////////////////////////////////////////////////////////
// General information API

void VPXPluginAPIImpl::GetTableInfo(VPXTableInfo* info)
{
   // Only valid in game
   if (g_pplayer != nullptr)
   {
      info->path = g_pplayer->m_ptable->m_szFileName.c_str();
      info->tableWidth = g_pplayer->m_ptable->m_right;
      info->tableHeight = g_pplayer->m_ptable->m_bottom;
   }
   else
   {
      memset(info, 0, sizeof(VPXTableInfo));
   }
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
// Shared logging support for plugin API

void VPXPluginAPIImpl::PluginLog(unsigned int level, const char* message)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   switch (level)
   {
   case LPI_LVL_DEBUG: PLOGD.printf(message); break;
   case LPI_LVL_INFO: PLOGI.printf(message); break;
   case LPI_LVL_ERROR: PLOGE.printf(message); break;
   default: assert(false); PLOGE << "Invalid plugin log message level";
   }
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

void VPXPluginAPIImpl::SubmitTypeLibrary()
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.ResolveAllClasses();
}

void VPXPluginAPIImpl::OnScriptError(unsigned int type, const char* message)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   // FIXME implement in DynamicDispatch
}


///////////////////////////////////////////////////////////////////////////////
// API to support overriding legacy COM objects

void VPXPluginAPIImpl::SetCOMObjectOverride(const char* className, const ScriptClassDef* classDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   // FIXME remove when classDef is unregistered
   // FIXME check that classDef has been registered in the type library ?
   string classId(className);
   StrToLower(classId);
   if (classDef == nullptr)
      pi.m_scriptCOMObjectOverrides.erase(classId);
   else
      pi.m_scriptCOMObjectOverrides[classId] = classDef;
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
      string className = res[1].str();
      StrToLower(className);
      const auto& overrideEntry = m_scriptCOMObjectOverrides.find(className);
      if (overrideEntry != m_scriptCOMObjectOverrides.end())
      {
         PLOGI << "COM script object " << className << " overriden to be provided by a plugin";
         result << "CreatePluginObject(\"" << className << "\")";
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

IDispatch* VPXPluginAPIImpl::CreateCOMPluginObject(const string& classId)
{
   // FIXME we are not separating type library per plugin, therefore collision may occur
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   string className(classId);
   StrToLower(className);
   const auto& overrideEntry = m_scriptCOMObjectOverrides.find(className);
   if (overrideEntry == m_scriptCOMObjectOverrides.end())
   {
      PLOGE << "Asked to create object of type " << classId << " which is not registered";
      return nullptr;

   }
   const ScriptClassDef* classDef = overrideEntry->second;
   if (classDef->CreateObject == nullptr)
   {
      PLOGE << "Asked to create object of type " << classId << " which is registered without a factory method";
      return nullptr;
   }
   void* pScriptObject = classDef->CreateObject();
   if (pScriptObject == nullptr)
   {
      PLOGE << "Failed to create object of class " << classId;
      return nullptr;
   }
   return new DynamicDispatch(&pi.m_dynamicTypeLibrary, classDef, pScriptObject);
}


///////////////////////////////////////////////////////////////////////////////
// Expose VPX contributions through plugin API

#include "CorePlugin.h"

void VPXPluginAPIImpl::ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData)
{
   if (g_pplayer == nullptr)
      return;

   // Report main script DMD (we do not report ancialliary DMD directly set on flashers, but only the main table one)
   // TODO supported RGB frame format are either sRGB888 or sRGB565, not sRGBA8888, therefore RGB frame can not be broadcasted on the plugin bus for the time being
   GetDmdSrcMsg& msg = *static_cast<GetDmdSrcMsg*>(msgData);
   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);
   if (g_pplayer->m_dmdFrame && msg.count < msg.maxEntryCount && g_pplayer->m_dmdFrame->m_format == BaseTexture::BW)
   {
      msg.entries[msg.count].id = { me.m_vpxEndpointId, 0 };
      msg.entries[msg.count].format = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_GETDMD_FORMAT_LUM8 : CTLPI_GETDMD_FORMAT_SRGB888;
      msg.entries[msg.count].width = g_pplayer->m_dmdFrame->width();
      msg.entries[msg.count].height = g_pplayer->m_dmdFrame->height();
      msg.count++;
   }
}

void VPXPluginAPIImpl::ControllerOnGetRenderDMD(const unsigned int msgId, void* userData, void* msgData)
{
   if (g_pplayer == nullptr)
      return;
   GetDmdMsg* msg = static_cast<GetDmdMsg*>(msgData);
   if (msg->frame != nullptr) // Already answered
      return;

   // Script DMD
   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);
   if ((msg->dmdId.id.endpointId == me.m_vpxEndpointId) && (msg->dmdId.id.resId == 0) 
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
   if (g_pplayer == nullptr)
      return;
   GetRawDmdMsg* msg = static_cast<GetRawDmdMsg*>(msgData);
   if (msg->frame != nullptr) // Already answered
      return;

   // Script DMD
   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);
   if ((msg->dmdId.endpointId == me.m_vpxEndpointId) && (msg->dmdId.resId == 0)
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

   m_vpxEndpointId = MsgPluginManager::GetInstance().NewEndpointId();
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &OnGetVPXPluginAPI, nullptr);

   // Logging API
   m_loggingApi.Log = PluginLog;
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API), &OnGetLoggingPluginAPI, nullptr);

   // Scriptable API
   m_scriptableApi.RegisterScriptClass = RegisterScriptClass;
   m_scriptableApi.RegisterScriptTypeAlias = RegisterScriptTypeAlias;
   m_scriptableApi.RegisterScriptArrayType = RegisterScriptArray;
   m_scriptableApi.SubmitTypeLibrary = SubmitTypeLibrary;
   m_scriptableApi.SetCOMObjectOverride = SetCOMObjectOverride;
   m_scriptableApi.OnError = OnScriptError;
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API), &OnGetScriptablePluginAPI, nullptr);

   // Generic controller API
   m_getRenderDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   m_getIdentifyDmdMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_IDENTIFY_MSG);
   msgApi.SubscribeMsg(m_vpxEndpointId, m_getRenderDmdMsgId, &ControllerOnGetRenderDMD, this);
   msgApi.SubscribeMsg(m_vpxEndpointId, m_getIdentifyDmdMsgId, &ControllerOnGetIdentifyDMD, this);
   msgApi.SubscribeMsg(m_vpxEndpointId, msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG), &ControllerOnGetDMDSrc, this);
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

void VPXPluginAPIImpl::OnGetLoggingPluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   LoggingPluginAPI** pResult = static_cast<LoggingPluginAPI**>(msgData);
   *pResult = &pi.m_loggingApi;
}
