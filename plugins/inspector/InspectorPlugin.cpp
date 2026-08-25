// license:GPLv3+

#include "common.h"
#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "WebServer.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <format>
#include <vector>
#include <mutex>
#include <cstring>

#include <string>
using namespace std::string_literals;
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace PinballPlugin::Controller;

namespace Inspector
{

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int getVpxApiId;
static unsigned int onControllersChangedId;
static unsigned int onDisplaySrcChgId;
static unsigned int onSegSrcChgId;

static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<StateSrcId>> stateSources;

MSGPI_INT_VAL_SETTING(portSetting, "port", "Web Server Port", "Port used by the inspector web server", true, 1024, 65535, 2113);

static std::unique_ptr<WebServer> webServer;

static std::mutex displayStateMutex;
typedef struct
{
   CtlResId id;
   unsigned int width;
   unsigned int height;
   unsigned int frameFormat;
   DisplayFrame(MSGPIAPI* GetRenderFrame)(const CtlResId id);
} DisplayProvider;
static std::map<uint64_t, DisplayProvider> displayGetters;
static unsigned int treeId = 0;

void UpdateTreeCache()
{
   treeId++;
   if (!webServer)
      return;

   std::lock_guard lock(displayStateMutex);
   displayGetters.clear();

   json root = json::object();
   root["treeId"s] = treeId;
   root["tree"s] = json::array();

   if (!msgApi)
   {
      webServer->UpdateTreeJson(root.dump());
      return;
   }

   const unsigned int getControllersId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
   vector<ControllerDef> controllerDefs = GetCtrlItems<ControllerDef>(msgApi, endpointId, getControllersId);
   msgApi->ReleaseMsgID(getControllersId);

   json tree = json::array();

   if (controllerDefs.empty())
   {
      root["tree"s] = tree;
      webServer->UpdateTreeJson(root.dump());
      return;
   }

   std::map<uint32_t, json> controllers;
   auto getController = [&](uint32_t epId) -> json&
   {
      if (controllers.find(epId) == controllers.end())
      {
         MsgEndpointInfo info;
         msgApi->GetEndpointInfo(epId, &info);
         json cNode = json::object();
         cNode["id"s] = epId;
         cNode["name"s] = info.name ? info.name : (info.id ? info.id : "Unknown Controller");
         cNode["type"s] = "controller";
         cNode["children"s] = json::array();
         const auto ctrlDef = std::ranges::find_if(controllerDefs, [epId](const auto& ctrl) { return ctrl.endpointId == epId; });
         cNode["game"s] = ctrlDef == controllerDefs.end() ? "" : ctrlDef->gameId;
         controllers[epId] = cNode;
      }
      return controllers[epId];
   };
   for (const ControllerDef& controller : controllerDefs)
      getController(controller.endpointId);

   // States
   stateSources->With(
      [&getController](const std::vector<StateSrcId>& items)
      {
         for (const StateSrcId& stateDef : items)
         {
            auto& cNode = getController(stateDef.id.endpointId);
            json gNode = json::object();
            gNode["id"s] = stateDef.id.resId;
            gNode["name"s] = stateDef.name ? stateDef.name : "Unnamed state group";
            gNode["desc"s] = stateDef.desc ? stateDef.desc : "";
            gNode["type"s] = "stategroup";
            gNode["children"s] = json::array();
            for (unsigned int j = 0; j < stateDef.nStates; j++)
            {
               json item = json::object();
               item["type"s] = "state";
               // FIXME What should we expose as an id ? the mapping (stable, human friendly) or the index (unique) ?
               item["id"s] = std::format("{:04X}.{:04X}.{:04X}", stateDef.id.endpointId, stateDef.id.resId, stateDef.stateDefs[j].mappingId);
               item["mapping"s] = std::format("{:02d}", stateDef.stateDefs[j].mappingId);
               item["name"s] = stateDef.stateDefs[j].name ? stateDef.stateDefs[j].name : ("Device " + std::to_string(j));
               item["desc"s] = stateDef.stateDefs[j].desc ? stateDef.stateDefs[j].desc : "No description available";
               item["format"s] = stateDef.stateDefs[j].dataFormat;
               item["outputType"s] = stateDef.stateDefs[j].semanticType;
               gNode["children"s].push_back(item);
            }
            cNode["children"s].push_back(gNode);
         }
      });

   // Displays
   {
      unsigned int getDisplaysMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
      std::map<uint32_t, json> displayCats;
      for (const DisplaySrcId& displayDef : GetCtrlItems<DisplaySrcId>(msgApi, endpointId, getDisplaysMsgId))
      {
         uint32_t epId = displayDef.id.endpointId;
         if (displayCats.find(epId) == displayCats.end())
         {
            json catNode = json::object();
            catNode["name"s] = "Displays";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            displayCats[epId] = catNode;
         }
         json item = json::object();
         item["type"s] = "display";
         item["id"s] = std::to_string(displayDef.id.id);
         item["mapping"s] = std::format("{:02d}", displayDef.id.resId);
         item["name"s] = std::format("Display {} {}x{}", displayDef.id.resId, displayDef.width, displayDef.height);
         item["format"s] = displayDef.frameFormat;
         item["hardware"s] = displayDef.hardware;
         displayCats[epId]["children"s].push_back(item);
         displayGetters[displayDef.id.id]
            = { displayDef.id, displayDef.width, displayDef.height, displayDef.frameFormat, displayDef.GetRenderFrame };
      }
      for (auto& pair : displayCats)
      {
         auto& cNode = getController(pair.first);
         cNode["children"s].push_back(pair.second);
      }
      msgApi->ReleaseMsgID(getDisplaysMsgId);
   }

   // Segment Displays
   {
      unsigned int getSegsMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG);
      for (const SegSrcId& segDef : GetCtrlItems<SegSrcId>(msgApi, endpointId, getSegsMsgId))
      {
         auto& cNode = getController(segDef.id.endpointId);
         json catNode = json::object();
         catNode["name"s] = "Segment Displays";
         catNode["type"s] = "category";
         catNode["children"s] = json::array();
         json item = json::object();
         item["type"s] = "seg_display";
         item["name"s] = std::format("Seg Display {}", segDef.id.resId);
         item["mapping"s] = std::format("{:02d}", segDef.id.resId);
         catNode["children"s].push_back(item);
         cNode["children"s].push_back(catNode);
      }
      msgApi->ReleaseMsgID(getSegsMsgId);
   }

   for (auto& pair : controllers)
      tree.push_back(pair.second);

   root["tree"s] = tree;

   webServer->UpdateTreeJson(root.dump());
}

std::string GetStatesJson()
{
   json states = json::array();
   stateSources->With(
      [&states](const std::vector<StateSrcId>& items)
      {

      for (const StateSrcId& stateDef : items)
      {
         json dItem = json::object();
         for (unsigned int j = 0; j < stateDef.nStates; j++)
         {
            if (stateDef.stateDefs[j].GetState == nullptr)
               continue;
            dItem["id"s] = std::format("{:04X}.{:04X}.{:04X}", stateDef.id.endpointId, stateDef.id.resId, stateDef.stateDefs[j].mappingId);
            dItem["type"s] = stateDef.stateDefs[j].semanticType;
            switch (stateDef.stateDefs[j].dataFormat)
            {
            case CTLPI_STATE_FORMAT_FLOAT:
            {
               float state;
               stateDef.stateDefs[j].GetState(stateDef.id, j, &state);
               dItem["format"s] = "float";
               dItem["state"s] = state;
               states.push_back(dItem);
            }
            break;

            case CTLPI_STATE_FORMAT_UINT8:
            {
               uint8_t state;
               stateDef.stateDefs[j].GetState(stateDef.id, j, &state);
               dItem["format"s] = "uint8";
               dItem["state"s] = state;
               states.push_back(dItem);
            }
            break;

            case CTLPI_STATE_FORMAT_INT32:
            {
               int32_t state;
               stateDef.stateDefs[j].GetState(stateDef.id, j, &state);
               dItem["format"s] = "int32";
               dItem["state"s] = state;
               states.push_back(dItem);
            }
            break;

            case CTLPI_STATE_FORMAT_INT64:
            {
               int64_t state;
               stateDef.stateDefs[j].GetState(stateDef.id, j, &state);
               dItem["format"s] = "int64";
               dItem["state"s] = std::to_string(state);
               states.push_back(dItem);
            }
            break;

            case CTLPI_STATE_FORMAT_STRING:
            {
               char* state = nullptr;
               stateDef.stateDefs[j].GetState(stateDef.id, j, &state);
               dItem["format"s] = "string";
               dItem["state"s] = state != nullptr ? state : "";
               states.push_back(dItem);
            }
            break;
            }
         }
      }
   });

   json root = json::object();
   root["treeId"s] = treeId;
   root["states"s] = states;
   return root.dump();
}

namespace
{
   // Converts a raw DisplayFrame (as produced by DisplaySrcId::GetRenderFrame) into a top-down 24bpp RGB
   // buffer, written into 'dst' after 'offset' bytes left free for the caller's own header, so that the
   // frame does not have to be copied again to prepend it. 'dst' is resized accordingly and its capacity
   // is reused from call to call. Returns false for unsupported frame formats
   bool ConvertFrameToRgb24(unsigned int width, unsigned int height, unsigned int frameFormat, const void* frame, size_t offset, std::vector<uint8_t>& dst)
   {
      const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
      dst.resize(offset + count * 3);
      uint8_t* const __restrict rgb = dst.data() + offset;

      if (frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
      {
         std::memcpy(rgb, frame, count * 3);
      }
      else if (frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565)
      {
         const uint16_t* const __restrict src = static_cast<const uint16_t*>(frame);
         for (size_t i = 0; i < count; i++)
         {
            const uint16_t px = src[i];
            const uint32_t r5 = (px >> 11) & 0x1F;
            const uint32_t g6 = (px >> 5) & 0x3F;
            const uint32_t b5 = px & 0x1F;
            // Replicate the high bits into the low ones, so that a full channel maps to 255 instead of 248
            rgb[i * 3 + 0] = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
            rgb[i * 3 + 1] = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
            rgb[i * 3 + 2] = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
         }
      }
      else if (frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
      {
         const float* const __restrict src = static_cast<const float*>(frame);
         for (size_t i = 0; i < count; i++)
         {
            float v = src[i];
            if (v < 0.f)
               v = 0.f;
            if (v > 1.f)
               v = 1.f;
            const uint8_t l = static_cast<uint8_t>(v * 255.f + 0.5f);
            rgb[i * 3 + 0] = l;
            rgb[i * 3 + 1] = l;
            rgb[i * 3 + 2] = l;
         }
      }
      else
      {
         return false; // unsupported/unknown frame format
      }
      return true;
   }
}

bool IsDisplayKnown(uint64_t mapping)
{
   std::lock_guard lock(displayStateMutex);
   return displayGetters.find(mapping) != displayGetters.end();
}

// Looks up the display registered under `mapping` (the same CtlResId.id sent to the client as the display
// node's "mapping" field) and writes its current frame into `rgb`, as described by ConvertFrameToRgb24.
// `lastFrameId` is the id of the frame the caller already has, or nullptr if it has none.
// Returns false, leaving `rgb` untouched, if the mapping is unknown, the source has no frame yet, the frame
// is the one the caller already has, or the frame format isn't supported
bool GetDisplayFrameRGB(uint64_t mapping, const uint32_t* lastFrameId, size_t headerSize, std::vector<uint8_t>& rgb, uint32_t& width, uint32_t& height, uint32_t& frameId)
{
   DisplayProvider provider;
   {
      std::lock_guard lock(displayStateMutex);
      auto it = displayGetters.find(mapping);
      if (it == displayGetters.end())
         return false;
      provider = it->second;
   }

   if (!provider.GetRenderFrame || provider.width == 0 || provider.height == 0)
      return false;

   const DisplayFrame frame = provider.GetRenderFrame(provider.id);
   if (!frame.frame)
      return false;

   // Displays stay unchanged for long stretches, so drop out before converting anything
   if (lastFrameId && (frame.frameId == *lastFrameId))
      return false;

   width = provider.width;
   height = provider.height;
   frameId = frame.frameId;
   return ConvertFrameToRgb24(provider.width, provider.height, provider.frameFormat, frame.frame, headerSize, rgb);
}

void OnSrcChanged(const unsigned int eventId, void* userData, void* msgData) { UpdateTreeCache(); }

} // namespace Inspector

using namespace Inspector;

MSGPI_EXPORT void MSGPIAPI InspectorPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);

   msgApi->SubscribeMsg(endpointId, onControllersChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onDisplaySrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onSegSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->RegisterSetting(endpointId, &portSetting);

   stateSources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<StateSrcId>>(
      msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG, nullptr, nullptr, []() { UpdateTreeCache(); });

   std::filesystem::path path;
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
   VPXInfo vpxInfo;
   vpxApi->GetVpxInfo(&vpxInfo);
   path = std::filesystem::path(vpxInfo.path) / "plugins"sv / "inspector"sv;
#else
   path = GetPluginPath();
#endif
   path = path / "assets"sv;

   webServer = std::make_unique<WebServer>();
   webServer->Start(portSetting_Get(), path.string());

   OnSrcChanged(onControllersChangedId, nullptr, nullptr);
   UpdateTreeCache();
}

MSGPI_EXPORT void MSGPIAPI InspectorPluginUnload()
{
   if (webServer)
   {
      webServer->Stop();
      webServer.reset();
   }

   stateSources = nullptr;

   msgApi->UnsubscribeMsg(onControllersChangedId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onDisplaySrcChgId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onSegSrcChgId, OnSrcChanged, nullptr);

   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onControllersChangedId);
   msgApi->ReleaseMsgID(onDisplaySrcChgId);
   msgApi->ReleaseMsgID(onSegSrcChgId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
