// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"

#include <functional>
#include <cstring>
#include <cstdlib>
#include <memory>

#include "common.h"

#include "plugins/ResURIResolver.h"

#include "ScoreView.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace ScoreView
{

LPI_IMPLEMENT // Implement shared log support

static const MsgPluginAPI* msgApi = nullptr;
static unsigned int getVpxApiId;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;
static std::unique_ptr<ScoreView> scoreView;

static int OnRender(VPXRenderContext2D* ctx, void*)
{
   if ((ctx->window != VPXWindowId::VPXWINDOW_Backglass) && (ctx->window != VPXWindowId::VPXWINDOW_ScoreView))
      return false;
   if (scoreView == nullptr)
   {
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);
      std::filesystem::path tablePath = tableInfo.path;

      scoreView = std::make_unique<ScoreView>(msgApi, endpointId, vpxApi);

      // First try  a file matching the table file with scv extension
      scoreView->Load(tablePath.replace_extension(".scv"));

      // Then try  a file matching the table's parent folder name with scv extension
      if (!scoreView->HasLayouts())
         scoreView->Load(tablePath.parent_path() / tablePath.parent_path().filename().replace_extension(".scv"));

      // Allow the user to provide a custom folder (out of application path) with his default layouts ?

      // Finally defaults to base layouts provided with the plugin
      if (!scoreView->HasLayouts())
      {
         // Load default layouts provided with plugin
         std::filesystem::path path;
         #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
         VPXInfo vpxInfo;
         vpxApi->GetVpxInfo(&vpxInfo);
         path = std::filesystem::path(vpxInfo.path) / "plugins" / "scoreview";
         #else
         path = GetPluginPath();
         #endif
         path = path / "layouts"s;
         scoreView->Load(path);
      }
   }
   return scoreView->Render(ctx) ? 1 : 0;
}

static void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static constexpr AncillaryRendererDef entry = { "ScoreView", "ScoreView", "Renderer for generic score displays", nullptr, OnRender };
   GetAncillaryRendererMsg* msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   if (msg->window == VPXWindowId::VPXWINDOW_ScoreView)
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = entry;
      msg->count++;
   }
}

static void OnGameStart(const unsigned int eventId, void* userData, void* eventData)
{
}

static void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   scoreView = nullptr;
}

}

using namespace ScoreView;

MSGPI_EXPORT void MSGPIAPI ScoreViewPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);

   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);

   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), OnGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

   msgApi->SubscribeMsg(endpointId, onGetAuxRendererId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, nullptr);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);
}

MSGPI_EXPORT void MSGPIAPI ScoreViewPluginUnload()
{
   msgApi->UnsubscribeMsg(onGetAuxRendererId, OnGetRenderer);
   msgApi->UnsubscribeMsg(onGameStartId, OnGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId, nullptr);
   msgApi->ReleaseMsgID(onGetAuxRendererId);
   msgApi->ReleaseMsgID(onAuxRendererChgId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(getVpxApiId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
