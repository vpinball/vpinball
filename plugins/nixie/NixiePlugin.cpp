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

static const MsgPluginAPI* msgApi = nullptr;
static unsigned int getVpxApiId;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;

// One renderer per ancillary display: each walks the layout source fallback chain
// and selects its layout independently, like an instance of the plugin per display
struct NixieWindow
{
   std::unique_ptr<Nixie> renderer;
   int layoutSourceIndex = 0; // Next layout source to try in the fallback chain for this window
};
static NixieWindow nixies[3]; // Indexed by WindowIndex

static int WindowIndex(const VPXWindowId window)
{
   switch (window)
   {
   case VPXWindowId::VPXWINDOW_Backglass: return 0;
   case VPXWindowId::VPXWINDOW_ScoreView: return 1;
   case VPXWindowId::VPXWINDOW_Topper: return 2;
   default: return -1;
   }
}

static std::filesystem::path GetTablePath()
{
   VPXTableInfo tableInfo {};
   vpxApi->GetTableInfo(&tableInfo);
   return PluginStrings::PathFromNative(tableInfo.path);
}

// Resolve layouts for the given window by walking the layout source fallback chain:
// a file matching the table name, then one matching the table's folder name, then
// the custom layout folder setting, then the layouts provided with the plugin. Each
// window walks the chain independently, stopping at the first source providing a
// layout section targeting it.
static void EnsureLayouts(const int windowIndex)
{
   static constexpr VPXWindowId windows[3] = { VPXWindowId::VPXWINDOW_Backglass, VPXWindowId::VPXWINDOW_ScoreView, VPXWindowId::VPXWINDOW_Topper };
   NixieWindow& nw = nixies[windowIndex];
   while (!nw.renderer->HasLayouts(windows[windowIndex]) && nw.layoutSourceIndex < 4)
   {
      switch (nw.layoutSourceIndex)
      {
      case 0:
      {
         // First try  a file matching the table file with nix extension
         std::filesystem::path tablePath = GetTablePath();
         nw.renderer->Load(tablePath.replace_extension(".nix"));
         break;
      }
      case 1:
      {
         // Then try  a file matching the table's parent folder name with nix extension
         const std::filesystem::path tablePath = GetTablePath();
         std::filesystem::path folderFile = tablePath.parent_path().filename();
         folderFile += ".nix"; // Appended, instead of replace_extension, otherwise "AFM 1.2" turns into "AFM 1.nix"
         nw.renderer->Load(tablePath.parent_path() / folderFile);
         break;
      }
      case 2:
      {
         // Allow the user to provide a custom folder (out of application path) with his default layouts ?
         if (std::string customPath = layoutFolderProp_Get(); !customPath.empty())
            nw.renderer->Load(PluginStrings::PathFromUTF8OrNative(customPath));
         break;
      }
      case 3:
      {
         // Finally defaults to base layouts provided with the plugin
         std::filesystem::path path;
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
         VPXInfo vpxInfo {};
         vpxApi->GetVpxInfo(&vpxInfo);
         path = PluginStrings::PathFromNative(vpxInfo.path) / "plugins"sv / "nixie"sv;
#else
         path = GetPluginPath();
#endif
         path = path / "layouts"sv;
         nw.renderer->Load(path);
         break;
      }
      }
      nw.layoutSourceIndex++;
   }
}

static int OnRender(VPXRenderContext2D* ctx, void*)
{
   const int windowIndex = WindowIndex(ctx->window);
   if (windowIndex < 0)
      return false;
   if (nixies[windowIndex].renderer == nullptr)
      nixies[windowIndex].renderer = std::make_unique<Nixie>(msgApi, endpointId, vpxApi);
   EnsureLayouts(windowIndex);
   return nixies[windowIndex].renderer->Render(ctx) ? 1 : 0;
}

static void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static constexpr AncillaryRendererDef entry = { "Nixie", "Nixie", "Renderer for generic score displays", nullptr, OnRender };
   GetAncillaryRendererMsg* msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   if (msg->window == VPXWindowId::VPXWINDOW_Backglass || msg->window == VPXWindowId::VPXWINDOW_ScoreView || msg->window == VPXWindowId::VPXWINDOW_Topper)
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = entry;
      msg->count++;
   }
}

static void OnGameStart(const unsigned int eventId, void* userData, void* eventData) { }

static void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   for (auto& nixie : nixies)
      nixie = {};
}
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
   for (auto& nixie : nixies)
      nixie = {};
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
