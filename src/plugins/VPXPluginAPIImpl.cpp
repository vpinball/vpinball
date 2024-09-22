// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPlugin.h"
#include "VPXPluginAPIImpl.h"

namespace VPXPluginAPIImpl
{

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
// Global API

VPXPluginAPI g_vpxAPI {
   GetTableInfo,

   GetOption,
   PushNotification,
   UpdateNotification,

   DisableStaticPrerendering,
   GetActiveViewSetup,
   SetActiveViewSetup,
};

void OnGetPluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginAPI** pResult = static_cast<VPXPluginAPI**>(msgData);
   *pResult = &VPXPluginAPIImpl::g_vpxAPI;
}

void RegisterVPXPluginAPI()
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
   // VPX API
   msgApi.SubscribeMsg(msgApi.GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &OnGetPluginAPI, nullptr);
}

}

