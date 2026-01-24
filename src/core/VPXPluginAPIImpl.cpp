// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPluginAPIImpl.h"

///////////////////////////////////////////////////////////////////////////////
// General information API

void MSGPIAPI VPXPluginAPIImpl::GetVpxInfo(VPXInfo* info)
{
   if (g_pvp != nullptr)
   {
      // statics as they need to survive as C string after this function returns
      static string path;
      path = g_pvp->GetAppPath(VPinball::AppSubFolder::Root).string() + PATH_SEPARATOR_CHAR;
      static string prefPath;
      prefPath = g_pvp->GetAppPath(VPinball::AppSubFolder::Preferences).string() + PATH_SEPARATOR_CHAR;
      info->path = path.c_str();
      info->prefPath = prefPath.c_str();
   }
   else
   {
      memset(info, 0, sizeof(VPXInfo));
   }
}

void MSGPIAPI VPXPluginAPIImpl::GetTableInfo(VPXTableInfo* info)
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

unsigned int MSGPIAPI VPXPluginAPIImpl::PushNotification(const char* msg, const int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   return g_pplayer->m_liveUI->PushNotification(msg, lengthMs);
}

void MSGPIAPI VPXPluginAPIImpl::UpdateNotification(const unsigned int handle, const char* msg, const int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_liveUI->PushNotification(msg, lengthMs, handle);
}


///////////////////////////////////////////////////////////////////////////////
// View API

void MSGPIAPI VPXPluginAPIImpl::DisableStaticPrerendering(const BOOL disable)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_renderer->DisableStaticPrePass(disable);
}

void MSGPIAPI VPXPluginAPIImpl::GetActiveViewSetup(VPXViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   const ViewSetup& viewSetup = g_pplayer->m_ptable->GetViewSetup();
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
   view->screenWidth = g_pplayer->m_ptable->m_settings.GetPlayer_ScreenWidth();
   view->screenHeight = g_pplayer->m_ptable->m_settings.GetPlayer_ScreenHeight();
   view->screenInclination = g_pplayer->m_ptable->m_settings.GetPlayer_ScreenInclination();
   view->realToVirtualScale = viewSetup.GetRealToVirtualScale(g_pplayer->m_ptable);
}

void MSGPIAPI VPXPluginAPIImpl::SetActiveViewSetup(VPXViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   ViewSetup& viewSetup = g_pplayer->m_ptable->GetViewSetup();
   viewSetup.mViewX = view->viewX;
   viewSetup.mViewY = view->viewY;
   viewSetup.mViewZ = view->viewZ;
   g_pplayer->m_renderer->InitLayout();
}


///////////////////////////////////////////////////////////////////////////////
// Input API

void MSGPIAPI VPXPluginAPIImpl::SetActionState(const VPXAction actionId, const int isPressed)
{
   if (!g_pplayer)
      return; // No game in progress

   VPXPluginAPIImpl& me = VPXPluginAPIImpl::GetInstance();
   auto it = me.m_actionMap.find(actionId);
   if (it == me.m_actionMap.end())
      return; // action not mapped

   const std::unique_ptr<InputAction>& action = g_pplayer->m_pininput.GetInputActions()[it->second.first];
   if (it->second.second == -1)
      it->second.second = action->NewDirectStateSlot();
   action->SetDirectState(it->second.second, isPressed != 0);
}

void MSGPIAPI VPXPluginAPIImpl::SetNudgeState(const int stateMask, const float nudgeAccelerationX, const float nudgeAccelerationY)
{
   if (!g_pplayer)
      return; // No game in progress

   g_pplayer->m_pininput.SetNudge((stateMask & 1) != 0, nudgeAccelerationX, nudgeAccelerationY);
}

void MSGPIAPI VPXPluginAPIImpl::SetPlungerState(const int stateMask, const float plungerPos, const float plungerSpeed)
{
   if (!g_pplayer)
      return; // No game in progress

   g_pplayer->m_pininput.SetPlungerPos((stateMask & 1) == 0x01, plungerPos);
   g_pplayer->m_pininput.SetPlungerSpeed((stateMask & 3) == 0x03, plungerSpeed); // With speed and overriden
}


///////////////////////////////////////////////////////////////////////////////
// Game State

double MSGPIAPI VPXPluginAPIImpl::GetGameTime()
{
   return g_pplayer ? g_pplayer->m_time_sec : 0.0;
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
   case BaseTexture::BW_FP32: tex->info.format = VPXTextureFormat::VPXTEXFMT_BW32F; break;
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

void MSGPIAPI VPXPluginAPIImpl::UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image)
{
   VPXTextureBlock** tex = reinterpret_cast<VPXTextureBlock**>(texture);
   if (*tex == nullptr)
      *tex = new VPXTextureBlock();
   switch (format)
   {
   case VPXTextureFormat::VPXTEXFMT_BW32F: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::BW_FP32, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGB8: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGB, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGBA8: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGBA, image); break;
   case VPXTextureFormat::VPXTEXFMT_sRGB565: BaseTexture::Update((*tex)->tex, width, height, BaseTexture::SRGB565, image); break;
   default: assert(false);
   }
   UpdateVPXTextureInfo(*tex);
}

VPXTexture MSGPIAPI VPXPluginAPIImpl::CreateTexture(uint8_t* rawData, int size)
{
   // BGFX allows to create texture from any thread and other rendering backends are single threaded
   // assert(std::this_thread::get_id() == VPXPluginAPIImpl::GetInstance().m_apiThread);
   VPXTextureBlock* tex = new VPXTextureBlock();
   tex->tex = BaseTexture::CreateFromData(rawData, size);
   if (tex->tex == nullptr)
      return nullptr;
   UpdateVPXTextureInfo(tex);
   return reinterpret_cast<VPXTexture>(tex);
}

VPXTextureInfo* MSGPIAPI VPXPluginAPIImpl::GetTextureInfo(VPXTexture texture)
{
   //assert(std::this_thread::get_id() == VPXPluginAPIImpl::GetInstance().m_apiThread);
   VPXTextureBlock* tex = reinterpret_cast<VPXTextureBlock*>(texture);
   return tex ? &tex->info : nullptr;
}

void MSGPIAPI VPXPluginAPIImpl::DeleteTexture(VPXTexture texture)
{
   MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().RunOnMainThread(
      VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), 0, 
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

void MSGPIAPI VPXPluginAPIImpl::PluginLog(unsigned int level, const char* message)
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

void MSGPIAPI VPXPluginAPIImpl::RegisterScriptClass(ScriptClassDef* classDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptClass(classDef);
}

void MSGPIAPI VPXPluginAPIImpl::RegisterScriptTypeAlias(const char* name, const char* aliasedType)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptTypeAlias(name, aliasedType);
}

void MSGPIAPI VPXPluginAPIImpl::RegisterScriptArray(ScriptArrayDef* arrayDef)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.RegisterScriptArray(arrayDef);
}

void MSGPIAPI VPXPluginAPIImpl::SubmitTypeLibrary()
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   pi.m_dynamicTypeLibrary.ResolveAllClasses();
}

void MSGPIAPI VPXPluginAPIImpl::OnScriptError(unsigned int type, const char* message)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   // FIXME implement in DynamicDispatch
}

ScriptClassDef* MSGPIAPI VPXPluginAPIImpl::GetClassDef(const char* typeName)
{
   VPXPluginAPIImpl& pi = VPXPluginAPIImpl::GetInstance();
   return pi.m_dynamicTypeLibrary.ResolveClass(typeName);
}

///////////////////////////////////////////////////////////////////////////////
// API to support overriding legacy COM objects

void MSGPIAPI VPXPluginAPIImpl::SetCOMObjectOverride(const char* className, const ScriptClassDef* classDef)
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
string VPXPluginAPIImpl::ApplyScriptCOMObjectOverrides(const string& script) const
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
// Settings API

void VPXPluginAPIImpl::UpdateSetting(const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef)
{
   if (action == MsgPI::MsgPluginManager::SettingAction::UnregisterAll)
   {
      // We keep the property definition in the settings, but we remove reference to the plugin owned memory block with live data
      m_pluginSettings.erase(
         std::remove_if(m_pluginSettings.begin(), m_pluginSettings.end(), [&pluginId](const PluginSetting& x) { return x.pluginId == pluginId; }), m_pluginSettings.end());
      return;
   }

   const auto item = std::ranges::find_if(
      m_pluginSettings, [&pluginId, &settingDef](const PluginSetting& setting) { return setting.pluginId == pluginId && setting.setting->propId == settingDef->propId; });

   // Register property and get or set value
   Settings& settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
   const bool asTableOverride = g_pplayer != nullptr;
   const std::string sectionName = "Plugin."s + pluginId;
   switch (settingDef->type)
   {
   case MSGPI_SETTING_TYPE_FLOAT:
   {
      const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description,
         false,
         settingDef->floatDef.minVal, settingDef->floatDef.maxVal, settingDef->floatDef.step, settingDef->floatDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->floatDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
         settingDef->floatDef.Set(settings.GetFloat(newId));
      break;
   }

   case MSGPI_SETTING_TYPE_INT:
      if (settingDef->intDef.values)
      {
         vector<string> values;
         for (int i = settingDef->intDef.minVal; i <= settingDef->intDef.maxVal; i++)
            values.emplace_back(settingDef->intDef.values[i - settingDef->intDef.minVal]);
         const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(
            sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->intDef.minVal, settingDef->intDef.defVal, values));
         if (item == m_pluginSettings.end())
            m_pluginSettings.emplace_back(pluginId, newId, settingDef);
         else
         {
            item->propId = newId;
            item->setting = settingDef;
         }
         if (action == MsgPI::MsgPluginManager::SettingAction::Save)
            settings.Set(newId, settingDef->intDef.Get(), asTableOverride);
         else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
            settingDef->intDef.Set(settings.GetInt(newId));
      }
      else
      {
         const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(
            sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->intDef.minVal, settingDef->intDef.maxVal, settingDef->intDef.defVal));
         if (item == m_pluginSettings.end())
            m_pluginSettings.emplace_back(pluginId, newId, settingDef);
         else
         {
            item->propId = newId;
            item->setting = settingDef;
         }
         if (action == MsgPI::MsgPluginManager::SettingAction::Save)
            settings.Set(newId, settingDef->intDef.Get(), asTableOverride);
         else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
            settingDef->intDef.Set(settings.GetInt(newId));
      }
      break;

   case MSGPI_SETTING_TYPE_BOOL:
   {
      const auto newId = Settings::GetRegistry().Register(
         std::make_unique<VPX::Properties::BoolPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->boolDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->boolDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
         settingDef->boolDef.Set(settings.GetBool(newId));
      break;
   }

   case MSGPI_SETTING_TYPE_STRING:
   {
      const auto newId = Settings::GetRegistry().Register(
         std::make_unique<VPX::Properties::StringPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->stringDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->stringDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
      {
         const string& value = settings.GetString(newId);
         settingDef->stringDef.Set(value.c_str());
      }
      break;
   }

   }
}


///////////////////////////////////////////////////////////////////////////////
// Expose VPX contributions through plugin API

#include "plugins/ControllerPlugin.h"

void VPXPluginAPIImpl::OnGameStart()
{
   assert(m_dmdSources.empty());
   const auto& msgApi = MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();

   msgApi.SubscribeMsg(GetVPXEndPointId(), m_onDisplayGetSrcMsgId, &ControllerOnGetDMDSrc, this);

   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onGameStartMsgId, nullptr);

   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChgMsgId, nullptr);

   const InputManager& inputManager = g_pplayer->m_pininput;
   m_actionMap[VPXACTION_LeftFlipper] = { inputManager.GetLeftFlipperActionId(), -1 };
   m_actionMap[VPXACTION_RightFlipper] = { inputManager.GetRightFlipperActionId(), -1 };
   m_actionMap[VPXACTION_StagedLeftFlipper] = { inputManager.GetStagedLeftFlipperActionId(), -1 };
   m_actionMap[VPXACTION_StagedRightFlipper] = { inputManager.GetStagedRightFlipperActionId(), -1 };
   m_actionMap[VPXACTION_LeftMagnaSave] = { inputManager.GetLeftMagnaActionId(), -1 };
   m_actionMap[VPXACTION_RightMagnaSave] = { inputManager.GetRightMagnaActionId(), -1 };
   m_actionMap[VPXACTION_LaunchBall] = { inputManager.GetLaunchBallActionId(), -1 };
   m_actionMap[VPXACTION_LeftNudge] = { inputManager.GetLeftNudgeActionId(), -1 };
   m_actionMap[VPXACTION_CenterNudge] = { inputManager.GetCenterNudgeActionId(), -1 };
   m_actionMap[VPXACTION_RightNudge] = { inputManager.GetRightNudgeActionId(), -1 };
   m_actionMap[VPXACTION_Tilt] = { inputManager.GetTiltActionId(), -1 };
   m_actionMap[VPXACTION_AddCredit] = { inputManager.GetAddCreditActionId(0), -1 };
   m_actionMap[VPXACTION_AddCredit2] = { inputManager.GetAddCreditActionId(1), -1 };
   m_actionMap[VPXACTION_StartGame] = { inputManager.GetStartActionId(), -1 };
   m_actionMap[VPXACTION_Lockbar] = { inputManager.GetLockbarActionId(), -1 };
   //m_actionMap[VPXACTION_Pause] = { inputManager.GetPauseActionId(), -1 };
   m_actionMap[VPXACTION_PerfOverlay] = { inputManager.GetLeftFlipperActionId(), -1 };
   m_actionMap[VPXACTION_ExitInteractive] = { inputManager.GetExitInteractiveActionId(), -1 };
   m_actionMap[VPXACTION_ExitGame] = { inputManager.GetExitGameActionId(), -1 };
   //m_actionMap[VPXACTION_InGameUI] = { inputManager.GetIn(), -1 };
   m_actionMap[VPXACTION_VolumeDown] = { inputManager.GetVolumeDownActionId(), -1 };
   m_actionMap[VPXACTION_VolumeUp] = { inputManager.GetVolumeUpActionId(), -1 };
   //m_actionMap[VPXACTION_VRRecenter] = { inputManager.GetVRRecenterActionId(), -1 };
   //m_actionMap[VPXACTION_VRUp] = { inputManager.GetVRUpActionId(), -1 };
   //m_actionMap[VPXACTION_VRDown] = { inputManager.GetVRDownActionId(), -1 };
}

void VPXPluginAPIImpl::OnGameEnd()
{
   const auto& msgApi = MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();

   msgApi.UnsubscribeMsg(m_onDisplayGetSrcMsgId, &ControllerOnGetDMDSrc);

   m_dmdSources.clear();

   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChgMsgId, nullptr);

   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onGameEndMsgId, nullptr);

   m_actionMap.clear();
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

   const auto& msgApi = MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();
   msgApi.BroadcastMsg(GetVPXEndPointId(), m_onDisplaySrcChgMsgId, nullptr);
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
   case BaseTexture::BW_FP32: result.frame = dmdFrame->data(); break;
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
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { me.m_vpxPlugin->m_endpointId, 0 } };
         msg.entries[msg.count].width = g_pplayer->m_dmdFrame->width();
         msg.entries[msg.count].height = g_pplayer->m_dmdFrame->height();
         msg.entries[msg.count].frameFormat = g_pplayer->m_dmdFrame->m_format == BaseTexture::BW_FP32 ? CTLPI_DISPLAY_FORMAT_LUM32F : CTLPI_DISPLAY_FORMAT_SRGB888;
         msg.entries[msg.count].GetRenderFrame = ControllerOnGetRenderDMD;
      }
      msg.count++;
   }

   // Ancillary DMDs defined on flasher objects from script
   for (size_t i = 0; i < me.m_dmdSources.size(); i++)
   {
      const auto& dmdSrc = me.m_dmdSources[i];
      assert(dmdSrc->m_dmdFrame);
      assert(dmdSrc->m_dmdFrame->m_format == BaseTexture::BW_FP32 || dmdSrc->m_dmdFrame->m_format == BaseTexture::SRGB);
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { me.m_vpxPlugin->m_endpointId, static_cast<uint32_t>(i + 1) } };
         msg.entries[msg.count].width = dmdSrc->m_dmdFrame->width();
         msg.entries[msg.count].height = dmdSrc->m_dmdFrame->height();
         msg.entries[msg.count].frameFormat = dmdSrc->m_dmdFrame->m_format == BaseTexture::BW_FP32 ? CTLPI_DISPLAY_FORMAT_LUM32F : CTLPI_DISPLAY_FORMAT_SRGB888;
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

VPXPluginAPIImpl::VPXPluginAPIImpl()
   : m_apiThread(std::this_thread::get_id())
   , m_getVPXAPIMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API))
   , m_onGameStartMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START))
   , m_onGameEndMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END))
   , m_getLoggingAPIMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API))
   , m_getScriptingAPIMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API))
   , m_onDisplaySrcChgMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG))
   , m_onDisplayGetSrcMsgId(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG))
{
   // Message host
   const auto& msgApi = MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();
   MsgPI::MsgPluginManager::GetInstance().SetSettingsHandler(
      [this](const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef) { UpdateSetting(pluginId, action, settingDef); });

   // VPX API
   m_api.GetVpxInfo = GetVpxInfo;
   m_api.GetTableInfo = GetTableInfo;

   m_api.PushNotification = PushNotification;
   m_api.UpdateNotification = UpdateNotification;

   m_api.DisableStaticPrerendering = DisableStaticPrerendering;
   m_api.GetActiveViewSetup = GetActiveViewSetup;
   m_api.SetActiveViewSetup = SetActiveViewSetup;

   m_api.SetActionState = SetActionState;
   m_api.SetNudgeState = SetNudgeState;
   m_api.SetPlungerState = SetPlungerState;

   m_api.GetGameTime = GetGameTime;

   m_api.CreateTexture = CreateTexture;
   m_api.UpdateTexture = UpdateTexture;
   m_api.GetTextureInfo = GetTextureInfo;
   m_api.DeleteTexture = DeleteTexture;

   m_vpxPlugin = MsgPI::MsgPluginManager::GetInstance().RegisterPlugin("vpx"s, "VPX"s, "Visual Pinball X"s, ""s, ""s, "https://github.com/vpinball/vpinball"s,  //
      [](const uint32_t, const MsgPluginAPI*) { /* Load: nothing to do */ }, //
      []() { /* Load: nothing to do */ });
   m_vpxPlugin->Load(&MsgPI::MsgPluginManager::GetInstance().GetMsgAPI());
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, m_getVPXAPIMsgId, &OnGetVPXPluginAPI, nullptr);

   // Logging API
   m_loggingApi.Log = PluginLog;
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, m_getLoggingAPIMsgId, &OnGetLoggingPluginAPI, nullptr);

   // Scriptable API
   m_scriptableApi.RegisterScriptClass = RegisterScriptClass;
   m_scriptableApi.RegisterScriptTypeAlias = RegisterScriptTypeAlias;
   m_scriptableApi.RegisterScriptArrayType = RegisterScriptArray;
   m_scriptableApi.SubmitTypeLibrary = SubmitTypeLibrary;
   m_scriptableApi.SetCOMObjectOverride = SetCOMObjectOverride;
   m_scriptableApi.OnError = OnScriptError;
   m_scriptableApi.GetClassDef = GetClassDef;
   msgApi.SubscribeMsg(m_vpxPlugin->m_endpointId, m_getScriptingAPIMsgId, &OnGetScriptablePluginAPI, nullptr);
}

VPXPluginAPIImpl::~VPXPluginAPIImpl()
{
   const auto& msgApi = MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();
   msgApi.UnsubscribeMsg(m_getVPXAPIMsgId, &OnGetVPXPluginAPI);
   msgApi.ReleaseMsgID(m_getVPXAPIMsgId);
   msgApi.UnsubscribeMsg(m_getLoggingAPIMsgId, &OnGetLoggingPluginAPI);
   msgApi.ReleaseMsgID(m_getLoggingAPIMsgId);
   msgApi.UnsubscribeMsg(m_getScriptingAPIMsgId, &OnGetScriptablePluginAPI);
   msgApi.ReleaseMsgID(m_getScriptingAPIMsgId);
   msgApi.ReleaseMsgID(m_onGameStartMsgId);
   msgApi.ReleaseMsgID(m_onGameEndMsgId);
   msgApi.ReleaseMsgID(m_onDisplayGetSrcMsgId);
   msgApi.ReleaseMsgID(m_onDisplaySrcChgMsgId);
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
