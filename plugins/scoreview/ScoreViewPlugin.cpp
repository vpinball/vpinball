// license:GPLv3+

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "core/ResURIResolver.h"

#include "plugins/LoggingPlugin.h"
LPI_IMPLEMENT // Implement shared login support

#include "ScoreView.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <locale>
#include <codecvt>
#define PATH_SEPARATOR_CHAR '\\'
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace ScoreView
{

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;
static std::unique_ptr<ScoreView> scoreview;

#ifndef _WIN32
string GetLibraryPath()
{
   Dl_info info{};
   if (dladdr((void*)&GetLibraryPath, &info) == 0 || !info.dli_fname)
      return "";
   char buf[PATH_MAX];
   if (!realpath(info.dli_fname, buf))
      return "";
   return string(buf);
}
#endif

string PathFromFilename(const string& szfilename)
{
   const int len = (int)szfilename.length();
   // find the last '\' in the filename
   int end;
   for (end = len; end >= 0; end--)
   {
      if (szfilename[end] == PATH_SEPARATOR_CHAR)
         break;
   }

   if (end == 0)
      end = len - 1;

   // copy from the start of the string to the end (or last '\')
   const char* szT = szfilename.c_str();
   int count = end + 1;

   string szpath;
   while (count--)
   {
      szpath.push_back(*szT++);
   }
   return szpath;
}

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
         // TODO this is implemented in a platform dependent manner as platforms like Android or iOS may need special handling (as plugins are statically linked, and we may embbed layouts in ressources or define a custom path scheme)
         string fullpath;
         #ifdef _WIN32
         HMODULE hm = nullptr;
         if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("PluginLoad"), &hm) == 0)
            return false;
         TCHAR path[MAX_PATH];
         if (GetModuleFileName(hm, path, MAX_PATH) == 0)
            return false;
         #ifdef _UNICODE
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
            fullpath.resize(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, path, -1, &fullpath[0], size_needed, NULL, NULL);
         #else
            fullpath = string(path);
         #endif
         #else
         fullpath = GetLibraryPath();
         #endif
         scoreview->Load(PathFromFilename(fullpath) + PATH_SEPARATOR_CHAR + "layouts" + PATH_SEPARATOR_CHAR);
      }
   }
   return scoreview->Render(ctx) ? 1 : 0;
}

void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   if (msg->window == VPXAnciliaryWindow::VPXWINDOW_ScoreView)
   {
      if (msg->count < msg->maxEntryCount)
      {
         msg->entries[msg->count].id = "ScoreView";
         msg->entries[msg->count].name = "ScoreView";
         msg->entries[msg->count].description = "Renderer for generic score displays";
         msg->entries[msg->count].Render = OnRender;
      }
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
