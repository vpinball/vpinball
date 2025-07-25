// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPluginAPIImpl.h"

///////////////////////////////////////////////////////////////////////////////
// General information API

void VPXPluginAPIImpl::GetVpxInfo(VPXInfo* info)
{
   if (g_pvp != nullptr)
   {
      info->path = g_pvp->m_myPath.c_str();
      info->prefPath = g_pvp->m_myPrefPath.c_str();
   }
   else
   {
      memset(info, 0, sizeof(VPXInfo));
   }
}

void VPXPluginAPIImpl::GetTableInfo(VPXTableInfo* info)
{
   // Only valid in game
   if (g_pplayer != nullptr)
   {
      info->path = g_pplayer->m_ptable->m_filename.c_str();
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
      Settings::Section section = Settings::GetSection(sectionName);
      std::vector<std::string> literals;
      if (values != nullptr)
      {
         const int nSteps = 1 + (int)(roundf((maxValue - minValue) / step));
         literals.reserve(nSteps);
         for (int i = 0; i < nSteps; i++)
            literals.push_back(values[i]);
      }
      settings.RegisterSetting(section, optionId, showMask, optionName, minValue, maxValue, step, defaultValue, (Settings::OptionUnit)unit, literals);
      const float value = settings.LoadValueWithDefault(section, optionId, defaultValue);
      return clamp(minValue + step * roundf((value - minValue) / step), minValue, maxValue);
   }
}

unsigned int VPXPluginAPIImpl::PushNotification(const char* msg, const int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   return g_pplayer->m_liveUI->PushNotification(msg, lengthMs);
}

void VPXPluginAPIImpl::UpdateNotification(const unsigned int handle, const char* msg, const int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_liveUI->PushNotification(msg, lengthMs, handle);
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
   view->screenWidth = g_pplayer->m_ptable->m_settings.LoadValueFloat(Settings::Player, "ScreenWidth"s);
   view->screenHeight = g_pplayer->m_ptable->m_settings.LoadValueFloat(Settings::Player, "ScreenHeight"s);
   view->screenInclination = g_pplayer->m_ptable->m_settings.LoadValueFloat(Settings::Player, "ScreenInclination"s);
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
// Input API

void VPXPluginAPIImpl::GetInputState(uint64_t* keyState, float* nudgeX, float* nudgeY, float* plunger)
{
   const Vertex2D& nudge = g_pplayer->m_pininput.GetNudge();
   *nudgeX = nudge.x;
   *nudgeY = nudge.y;
   *plunger = g_pplayer->m_pininput.GetPlungerPos();
   *keyState = g_pplayer->m_pininput.GetInputState().actionState;
}

void VPXPluginAPIImpl::SetInputState(const uint64_t keyState, const float nudgeX, const float nudgeY, const float plunger)
{
   PinInput::InputState state;
   state.actionState = keyState;
   g_pplayer->m_pininput.SetInputState(state);
   g_pplayer->m_pininput.SetNudge(Vertex2D(nudgeX, nudgeY));
   g_pplayer->m_pininput.SetPlungerPos(plunger);
}

///////////////////////////////////////////////////////////////////////////////
// Rendering

// We define VPXTexture as a pointer to a VPXTextureBlock holding a reference counted BaseTexture pointer
// This is needed since BaseTexture are referenced both wy their creator and by the pending GPU update (to avoid needing an expensive data copy)
struct VPXTextureBlock
{
   std::shared_ptr<BaseTexture> tex;
   VPXTextureInfo info;
};

static void UpdateVPXTextureInfo(VPXTextureBlock* tex)
{
   tex->info.width = tex->tex->width();
   tex->info.height = tex->tex->height();
   switch (tex->tex->m_format)
   {
   case BaseTexture::BW: tex->info.format = VPXTextureFormat::VPXTEXFMT_BW; break;
   case BaseTexture::SRGB: tex->info.format = VPXTextureFormat::VPXTEXFMT_sRGB8; break;
   case BaseTexture::SRGBA: tex->info.format = VPXTextureFormat::VPXTEXFMT_sRGBA8; break;
   case BaseTexture::SRGB565: tex->info.format = VPXTextureFormat::VPXTEXFMT_sRGB565; break;
   default: break; // FIXME what should we do ? reject texture ? have an unknown data format ?
   }
   tex->info.data = tex->tex->data();
}

std::shared_ptr<BaseTexture> VPXPluginAPIImpl::GetTexture(VPXTexture texture) const
{
   VPXTextureBlock* tex = reinterpret_cast<VPXTextureBlock*>(texture);
   return tex->tex;
}

void VPXPluginAPIImpl::UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const uint8_t* image)
{
   VPXTextureBlock** tex = reinterpret_cast<VPXTextureBlock**>(texture);
   if (*tex == nullptr)
      *tex = new VPXTextureBlock();
   switch (format)
   {
   case VPXTextureFormat::VPXTEXFMT_BW: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::BW, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGB8: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGB, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGBA8: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGBA, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGB565: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGB565, image); break;
   default: assert(false);
   }
   UpdateVPXTextureInfo(*tex);
}

VPXTexture VPXPluginAPIImpl::CreateTexture(uint8_t* rawData, int size)
{
   // BGFX allows to create texture from any thread and other rendering backends are single threaded
   // assert(std::this_thread::get_id() == VPXPluginAPIImpl::GetInstance().m_apiThread);
   VPXTextureBlock* tex = new VPXTextureBlock();
   tex->tex = BaseTexture::CreateFromData(rawData, size);
   UpdateVPXTextureInfo(tex);
   return reinterpret_cast<VPXTexture>(tex);
}

VPXTextureInfo* VPXPluginAPIImpl::GetTextureInfo(VPXTexture texture)
{
   //assert(std::this_thread::get_id() == VPXPluginAPIImpl::GetInstance().m_apiThread);
   VPXTextureBlock* tex = reinterpret_cast<VPXTextureBlock*>(texture);
   return tex ? &tex->info : nullptr;
}

void VPXPluginAPIImpl::DeleteTexture(VPXTexture texture)
{
   MsgPluginManager::GetInstance().GetMsgAPI().RunOnMainThread(0, 
      [](void* context) {
      VPXTextureBlock* tex = reinterpret_cast<VPXTextureBlock*>(context);
      if (tex)
      {
         if (tex->tex && g_pplayer && g_pplayer->GetCloseState() != Player::CS_CLOSED)
            g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(tex->tex.get());
         tex->tex = nullptr;
         delete tex;
      }
   }, texture);
}


///////////////////////////////////////////////////////////////////////////////
// Shared logging support for plugin API

void VPXPluginAPIImpl::PluginLog(unsigned int level, const char* message)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   switch (level)
   {
   case LPI_LVL_DEBUG: PLOGD << message; break;
   case LPI_LVL_INFO: PLOGI << message; break;
   case LPI_LVL_WARN: PLOGW << message; break;
   case LPI_LVL_ERROR: PLOGE << message; break;
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

ScriptClassDef* VPXPluginAPIImpl::GetClassDef(const char* typeName)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   return pi.m_dynamicTypeLibrary.ResolveClass(typeName);
}

///////////////////////////////////////////////////////////////////////////////
// API to support overriding legacy COM objects

void VPXPluginAPIImpl::SetCOMObjectOverride(const char* className, const ScriptClassDef* classDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   // FIXME remove when classDef is unregistered
   // FIXME check that classDef has been registered in the type library ?
   const string classId(lowerCase(className));
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
   std::regex re(R"(CreateObject\(\s*\"(.*)\"\s*\))", std::regex::icase);
   std::smatch res;
   string::const_iterator searchStart(script.cbegin());
   std::stringstream result;
   while (std::regex_search(searchStart, script.cend(), res, re))
   {
      result << res.prefix().str();
      const string className = lowerCase(res[1].str());
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
   const string className(lowerCase(classId));
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
   DynamicDispatch* dd = new DynamicDispatch(&pi.m_dynamicTypeLibrary, classDef, pScriptObject);
   PSC_RELEASE(classDef, pScriptObject);
   return dd;
}


///////////////////////////////////////////////////////////////////////////////
// Expose VPX contributions through plugin API

#include "plugins/ControllerPlugin.h"

void VPXPluginAPIImpl::OnGameStart()
{
   assert(m_dmdSources.empty());
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();

   unsigned int onDisplayGetSrcMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   msgApi.SubscribeMsg(GetVPXEndPointId(), onDisplayGetSrcMsgId, &ControllerOnGetDMDSrc, this);
   msgApi.ReleaseMsgID(onDisplayGetSrcMsgId);

   unsigned int onGameStartMsgId = msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START);
   msgApi.BroadcastMsg(GetVPXEndPointId(), onGameStartMsgId, nullptr);
   msgApi.ReleaseMsgID(onGameStartMsgId);

   m_onDisplaySrcChg = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChg, nullptr);
}

void VPXPluginAPIImpl::OnGameEnd()
{
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();

   unsigned int onDisplayGetSrcMsgId = msgApi.GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   msgApi.UnsubscribeMsg(onDisplayGetSrcMsgId, &ControllerOnGetDMDSrc);
   msgApi.ReleaseMsgID(onDisplayGetSrcMsgId);

   m_dmdSources.clear();

   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChg, nullptr);
   msgApi.ReleaseMsgID(m_onDisplaySrcChg);

   unsigned int onGameEndMsgId = MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END);
   msgApi.BroadcastMsg(GetVPXEndPointId(), onGameEndMsgId, nullptr);
   MsgPluginManager::GetInstance().GetMsgAPI().ReleaseMsgID(onGameEndMsgId);
}

void VPXPluginAPIImpl::UpdateDMDSource(Flasher* flasher, bool isAdd)
{
   if (flasher)
   {
      if (isAdd)
      {
         if (std::ranges::find(m_dmdSources, flasher) != m_dmdSources.end())
            return;
         m_dmdSources.push_back(flasher);
      }
      else
      {
         if (std::ranges::find(m_dmdSources, flasher) == m_dmdSources.end())
            return;
         RemoveFromVectorSingle(m_dmdSources, flasher);
      }
   }

   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChg, nullptr);
}

DisplayFrame VPXPluginAPIImpl::ControllerOnGetRenderDMD(const CtlResId id)
{
   VPXPluginAPIImpl& me = VPXPluginAPIImpl::GetInstance();

   if ((g_pplayer == nullptr) || (id.endpointId != me.m_vpxPlugin->m_endpointId))
      return { 0, nullptr };

   DisplayFrame result = { 0, nullptr };
   std::shared_ptr<BaseTexture> dmdFrame;
   if (id.resId == 0)
   {
      result.frameId = g_pplayer->m_dmdFrameId;
      dmdFrame = g_pplayer->m_dmdFrame;
   }
   else if (id.resId <= me.m_dmdSources.size())
   {
      const auto& dmdSrc = me.m_dmdSources[id.resId - 1];
      result.frameId = dmdSrc->m_dmdFrameId;
      dmdFrame = dmdSrc->m_dmdFrame;
   }
   if (dmdFrame == nullptr)
      return { 0, nullptr };

   switch (dmdFrame->m_format)
   {
   case BaseTexture::BW: result.frame = dmdFrame->data(); break;
   case BaseTexture::SRGB: result.frame = dmdFrame->data(); break;
   case BaseTexture::SRGBA: result.frame = dmdFrame->GetAlias(BaseTexture::SRGB)->data(); break;
   default: assert(false); return { 0, nullptr };  // Not yet supported
   }

   return result;
}

void VPXPluginAPIImpl::ControllerOnGetDMDSrc(const unsigned int msgId, void* userData, void* msgData)
{
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   VPXPluginAPIImpl& me = *static_cast<VPXPluginAPIImpl*>(userData);

   // Main DMD defined from script
   if (g_pplayer && g_pplayer->m_dmdFrame)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = { 0 };
         msg.entries[msg.count].id = { me.m_vpxPlugin->m_endpointId, 0 };
         msg.entries[msg.count].width = g_pplayer->m_dmdFrame->width();
         msg.entries[msg.count].height = g_pplayer->m_dmdFrame->height();
         msg.entries[msg.count].frameFormat = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_DISPLAY_FORMAT_LUM8 : CTLPI_DISPLAY_FORMAT_SRGB888;
         msg.entries[msg.count].GetRenderFrame = ControllerOnGetRenderDMD;
      }
      msg.count++;
   }

   // Ancilliary DMDs defined on flasher objects from script
   for (int i = 0; i < me.m_dmdSources.size(); i++)
   {
      const auto& dmdSrc = me.m_dmdSources[i];
      assert(dmdSrc->m_dmdFrame);
      assert(dmdSrc->m_dmdFrame->m_format == BaseTexture::BW || dmdSrc->m_dmdFrame->m_format == BaseTexture::SRGB);
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = { 0 };
         msg.entries[msg.count].id = { me.m_vpxPlugin->m_endpointId, static_cast<uint32_t>(i + 1) };
         msg.entries[msg.count].width = dmdSrc->m_dmdFrame->width();
         msg.entries[msg.count].height = dmdSrc->m_dmdFrame->height();
         msg.entries[msg.count].frameFormat = dmdSrc->m_dmdFrame->m_format == BaseTexture::BW ? CTLPI_DISPLAY_FORMAT_LUM8 : CTLPI_DISPLAY_FORMAT_SRGB888;
         msg.entries[msg.count].GetRenderFrame = ControllerOnGetRenderDMD;
      }
      msg.count++;
   }
}


///////////////////////////////////////////////////////////////////////////////
// 

VPXPluginAPIImpl& VPXPluginAPIImpl::GetInstance()
{
   static VPXPluginAPIImpl instance;
   return instance;
}

VPXPluginAPIImpl::VPXPluginAPIImpl() : m_apiThread(std::this_thread::get_id())
{
   // Message host
   const auto& msgApi = MsgPluginManager::GetInstance().GetMsgAPI();
   MsgPluginManager::GetInstance().SetSettingsHandler([](const char* name_space, const char* name, char* valueBuf, unsigned int valueBufSize)
      {
         const Settings& settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
         const std::string sectionName = "Plugin."s + name_space;
         Settings::Section section = Settings::GetSection(sectionName);
         std::string buffer;
         valueBuf[0] = '\0';
         if (settings.LoadValue(section, name, buffer))
         {
            #ifdef _MSC_VER
            strncpy_s(valueBuf, valueBufSize, buffer.c_str(), valueBufSize - 1);
            #else
            strncpy(valueBuf, buffer.c_str(), valueBufSize - 1);
            #endif
         }
      });

   // VPX API
   m_api.GetVpxInfo = GetVpxInfo;
   m_api.GetTableInfo = GetTableInfo;

   m_api.GetOption = GetOption;
   m_api.PushNotification = PushNotification;
   m_api.UpdateNotification = UpdateNotification;

   m_api.DisableStaticPrerendering = DisableStaticPrerendering;
   m_api.GetActiveViewSetup = GetActiveViewSetup;
   m_api.SetActiveViewSetup = SetActiveViewSetup;

   m_api.GetInputState = GetInputState;
   m_api.SetInputState = SetInputState;

   m_api.CreateTexture = CreateTexture;
   m_api.UpdateTexture = UpdateTexture;
   m_api.GetTextureInfo = GetTextureInfo;
   m_api.DeleteTexture = DeleteTexture;

   m_vpxPlugin = MsgPluginManager::GetInstance().RegisterPlugin("vpx", "VPX", "Visual Pinball X", "", "", "https://github.com/vpinball/vpinball", 
         [](const uint32_t pluginId, const MsgPluginAPI* api) {},
         []() {});
   m_vpxPlugin->Load(&MsgPluginManager::GetInstance().GetMsgAPI());
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &OnGetVPXPluginAPI, nullptr);

   // Logging API
   m_loggingApi.Log = PluginLog;
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, msgApi.GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API), &OnGetLoggingPluginAPI, nullptr);

   // Scriptable API
   m_scriptableApi.RegisterScriptClass = RegisterScriptClass;
   m_scriptableApi.RegisterScriptTypeAlias = RegisterScriptTypeAlias;
   m_scriptableApi.RegisterScriptArrayType = RegisterScriptArray;
   m_scriptableApi.SubmitTypeLibrary = SubmitTypeLibrary;
   m_scriptableApi.SetCOMObjectOverride = SetCOMObjectOverride;
   m_scriptableApi.OnError = OnScriptError;
   m_scriptableApi.GetClassDef = GetClassDef;
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, msgApi.GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API), &OnGetScriptablePluginAPI, nullptr);
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
