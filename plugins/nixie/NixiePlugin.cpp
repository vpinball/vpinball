// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"

#include <functional>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>

#include "common.h"

#include "Nixie.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace Nixie
{

MSGPI_FOLDER_VAL_SETTING(layoutFolderProp, "LayoutFolder", "Layout Folder", "Folder where custom Nixie layouts are stored", true, "", 1024);

LPI_IMPLEMENT_CPP // Implement shared log support

   static const MsgPluginAPI* msgApi
   = nullptr;
static unsigned int getVpxApiId;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;
static std::unique_ptr<Nixie> nixie;

static int OnRender(VPXRenderContext2D* ctx, void*)
{
   if ((ctx->window != VPXWindowId::VPXWINDOW_Backglass) && (ctx->window != VPXWindowId::VPXWINDOW_ScoreView))
      return false;
   if (nixie == nullptr)
   {
      VPXTableInfo tableInfo {};
      vpxApi->GetTableInfo(&tableInfo);
      std::filesystem::path tablePath = PluginStrings::PathFromNative(tableInfo.path);

      nixie = std::make_unique<Nixie>(msgApi, endpointId, vpxApi);

      // First try  a file matching the table file with nix extension
      nixie->Load(tablePath.replace_extension(".nix"));

      // Then try  a file matching the table's parent folder name with nix extension
      if (!nixie->HasLayouts(ctx->window))
      {
         std::filesystem::path folderFile = tablePath.parent_path().filename();
         folderFile += ".nix"; // Appended, instead of replace_extension, otherwise "AFM 1.2" turns into "AFM 1.nix"
         nixie->Load(tablePath.parent_path() / folderFile);
      }

      // Allow the user to provide a custom folder (out of application path) with his default layouts ?
      if (!nixie->HasLayouts(ctx->window))
      {
         if (std::string customPath = layoutFolderProp_Get(); !customPath.empty())
            nixie->Load(PluginStrings::PathFromUTF8OrNative(customPath));
      }

      // Finally defaults to base layouts provided with the plugin
      if (!nixie->HasLayouts(ctx->window))
      {
         // Load default layouts provided with plugin
         std::filesystem::path path;
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
         VPXInfo vpxInfo {};
         vpxApi->GetVpxInfo(&vpxInfo);
         path = PluginStrings::PathFromNative(vpxInfo.path) / "plugins"sv / "nixie"sv;
#else
         path = GetPluginPath();
#endif
         path = path / "layouts"sv;
         nixie->Load(path);
      }
   }
   return nixie->Render(ctx) ? 1 : 0;
}

static void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static constexpr AncillaryRendererDef entry = { "Nixie", "Nixie", "Renderer for generic score displays", nullptr, OnRender };
   GetAncillaryRendererMsg* msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   if (msg->window == VPXWindowId::VPXWINDOW_ScoreView)
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = entry;
      msg->count++;
   }
}

static void OnGameStart(const unsigned int eventId, void* userData, void* eventData) { }

static void OnGameEnd(const unsigned int eventId, void* userData, void* eventData) { nixie = nullptr; }

}

using namespace Nixie;

MSGPI_EXPORT void MSGPIAPI NixiePluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);

   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);

   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), OnGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

   msgApi->SubscribeMsg(endpointId, onGetAuxRendererId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, nullptr);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);

   msgApi->RegisterSetting(endpointId, &layoutFolderProp);
}

MSGPI_EXPORT void MSGPIAPI NixiePluginUnload()
{
   nixie = nullptr;
   msgApi->UnsubscribeMsg(onGetAuxRendererId, OnGetRenderer, nullptr);
   msgApi->UnsubscribeMsg(onGameStartId, OnGameStart, nullptr);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd, nullptr);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId, nullptr);
   msgApi->ReleaseMsgID(onGetAuxRendererId);
   msgApi->ReleaseMsgID(onAuxRendererChgId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(getVpxApiId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
