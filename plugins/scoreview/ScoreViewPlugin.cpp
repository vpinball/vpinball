// license:GPLv3+

#include "MsgPlugin.h"
#include "ControllerPlugin.h"

#include "common.h"

#include <functional>
#include <cstring>
#include <cstdlib>
#include <memory>

#include "core/ResURIResolver.h"

#include "ScoreView.h"


#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace ScoreView
{

LPI_IMPLEMENT // Implement shared login support

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;
static std::unique_ptr<ScoreView> scoreview;

int OnRender(VPXRenderContext2D* ctx, void*)
{
   if ((ctx->window != VPXAnciliaryWindow::VPXWINDOW_Backglass) && (ctx->window != VPXAnciliaryWindow::VPXWINDOW_ScoreView))
      return false;
   if (scoreview == nullptr)
   {
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);

      scoreview = std::make_unique<ScoreView>(msgApi, endpointId, vpxApi);
      scoreview->Load(PathFromFilename(tableInfo.path));
      if (!scoreview->HasLayouts())
      {
         // Load default layouts provided with plugin
         string path;
         #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
         VPXInfo vpxInfo;
         m_vpxApi->GetVpxInfo(&vpxInfo);
         path = string(vpxInfo.path) + PATH_SEPARATOR_CHAR + "plugins"s + PATH_SEPARATOR_CHAR + "flexdmd"s + PATH_SEPARATOR_CHAR;
         #else
         path = GetPluginPath();
         #endif
         path = path + "layouts"s + PATH_SEPARATOR_CHAR;
         scoreview->Load(path);
      }
   }
   return scoreview->Render(ctx) ? 1 : 0;
}

void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static AnciliaryRendererDef entry = { "ScoreView", "ScoreView", "Renderer for generic score displays", nullptr, OnRender };
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   if (msg->window == VPXAnciliaryWindow::VPXWINDOW_ScoreView)
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = entry;
      msg->count++;
   }
}

void OnGameStart(const unsigned int eventId, void* userData, void* eventData)
{
}

void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   scoreview = nullptr;
}

}

using namespace ScoreView;

MSGPI_EXPORT void MSGPIAPI ScoreViewPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

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
   vpxApi = nullptr;
   msgApi = nullptr;
}
