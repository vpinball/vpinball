// license:GPLv3+

#include "common.h"
#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "pinmame/PinMAMEPlugin.h"
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
static unsigned int onStateSrcChgId;
static unsigned int onDisplaySrcChgId;
static unsigned int onSegSrcChgId;

MSGPI_INT_VAL_SETTING(portSetting, "port", "Web Server Port", "Port used by the inspector web server", true, 1024, 65535, 2113);

static std::unique_ptr<WebServer> webServer;

static std::mutex deviceStatesMutex;
typedef struct
{
   unsigned int index;
   int typeMask;
   int(MSGPIAPI* GetState)(unsigned int index, int type, void* pResult);
} StateProvider;
static std::map<uint64_t, StateProvider> stateGetters;
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

   std::lock_guard lock(deviceStatesMutex);
   stateGetters.clear();
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

   // States
   {
      auto getGroup = [](std::map<uint32_t, json>& groups, uint16_t groupId, const std::string& name) -> json&
      {
         if (groups.find(groupId) == groups.end())
         {
            json cNode = json::object();
            cNode["id"s] = groupId;
            cNode["name"s] = name;
            cNode["type"s] = "group";
            cNode["children"s] = json::array();
            groups[groupId] = cNode;
         }
         return groups[groupId];
      };

      unsigned int getStateMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_GET_SRC_MSG);
      for (const StateSrcId& stateDef : GetCtrlItems<StateSrcId>(msgApi, endpointId, getStateMsgId))
      {
         auto& cNode = getController(stateDef.id.endpointId);
         json catNode = json::object();
         catNode["name"s] = "Game States";
         catNode["type"s] = "category";
         catNode["children"s] = json::array();
         std::map<uint32_t, json> groups;
         for (unsigned int j = 0; j < stateDef.nStates; j++)
         {
            json item = json::object();
            item["type"s] = "state";
            item["name"s] = stateDef.stateDefs[j].name ? stateDef.stateDefs[j].name : ("Device " + std::to_string(j));
            item["desc"s] = stateDef.stateDefs[j].desc ? stateDef.stateDefs[j].desc : "No description available";
            item["mapping"s] = std::to_string(stateDef.stateDefs[j].id.mappingId);
            item["writable"s] = stateDef.stateDefs[j].writable;
            item["dataType"s] = stateDef.stateDefs[j].typeMask;
            int groupIndex = -1;
            for (unsigned int k = 0; k < stateDef.nGroups; k++)
            {
               if (stateDef.groupDefs[k].id == stateDef.stateDefs[j].id.groupId)
               {
                  groupIndex = k;
                  break;
               }
            }
            if (groupIndex >= 0)
            {
               auto& cGroup = getGroup(groups, stateDef.stateDefs[j].id.groupId, stateDef.groupDefs[groupIndex].name);
               cGroup["children"s].push_back(item);
               stateGetters[stateDef.stateDefs[j].id.mappingId] = { j, stateDef.stateDefs[j].typeMask, stateDef.GetState };
            }
         }
         for (auto& pair : groups)
            catNode["children"s].push_back(pair.second);
         cNode["children"s].push_back(catNode);
      }
      msgApi->ReleaseMsgID(getStateMsgId);
   }

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
         item["name"s] = std::format("Display {} {}x{}", displayDef.id.resId, displayDef.width, displayDef.height);
         item["type"s] = "display";
         item["mapping"s] = std::to_string(displayDef.id.id);
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
         item["name"s] = std::format("Seg Display {}", segDef.id.resId);
         item["type"s] = "seg_display";
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
   std::lock_guard lock(deviceStatesMutex);

   json states = json::array();
   for (const auto& pair : stateGetters)
   {
      json dItem = json::object();
      dItem["id"s] = std::to_string(pair.first);
      if ((pair.second.typeMask & CTLPI_STATE_TYPE_FLOAT) != 0)
      {
         if (float state; pair.second.GetState(pair.second.index, CTLPI_STATE_TYPE_FLOAT, &state) == 0)
         {
            dItem["type"s] = "float";
            dItem["state"s] = state;
            states.push_back(dItem);
         }
      }
      else if ((pair.second.typeMask & CTLPI_STATE_TYPE_UINT8) != 0)
      {
         if (uint8_t state; pair.second.GetState(pair.second.index, CTLPI_STATE_TYPE_UINT8, &state) == 0)
         {
            dItem["type"s] = "uint8";
            dItem["state"s] = state;
            states.push_back(dItem);
         }
      }
      else if ((pair.second.typeMask & CTLPI_STATE_TYPE_INT32) != 0)
      {
         if (int32_t state; pair.second.GetState(pair.second.index, CTLPI_STATE_TYPE_INT32, &state) == 0)
         {
            dItem["type"s] = "int32";
            dItem["state"s] = state;
            states.push_back(dItem);
         }
      }
      else if ((pair.second.typeMask & CTLPI_STATE_TYPE_INT64) != 0)
      {
         if (int64_t state; pair.second.GetState(pair.second.index, CTLPI_STATE_TYPE_INT64, &state) == 0)
         {
            dItem["type"s] = "int64";
            dItem["state"s] = std::to_string(state);
            states.push_back(dItem);
         }
      }
      else if ((pair.second.typeMask & CTLPI_STATE_TYPE_STRING) != 0)
      {
         if (char* state; pair.second.GetState(pair.second.index, CTLPI_STATE_TYPE_STRING, &state) == 0)
         {
            dItem["type"s] = "int64";
            dItem["state"s] = std::string(state);
            states.push_back(dItem);
         }
      }
   }

   json root = json::object();
   root["treeId"s] = treeId;
   root["states"s] = states;
   return root.dump();
}

namespace
{
   // Converts a raw DisplayFrame (as produced by DisplaySrcId::GetRenderFrame)
   // into a top-down 24bpp RGB buffer. Returns an empty vector for unsupported
   // frame formats.
   std::vector<uint8_t> ConvertFrameToRgb24(unsigned int width, unsigned int height, unsigned int frameFormat, const void* frame)
   {
      const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
      std::vector<uint8_t> rgb(count * 3);

      if (frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
      {
         std::memcpy(rgb.data(), frame, count * 3);
      }
      else if (frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565)
      {
         const uint16_t* const __restrict src = static_cast<const uint16_t*>(frame);
         for (size_t i = 0; i < count; i++)
         {
            const uint16_t px = src[i];
            const uint8_t r5 = static_cast<uint8_t>((px >> 11) & 0x1F);
            const uint8_t g6 = static_cast<uint8_t>((px >> 5) & 0x3F);
            const uint8_t b5 = static_cast<uint8_t>(px & 0x1F);
            rgb[i * 3 + 0] = static_cast<uint8_t>((r5 * 255 + 15) / 31);
            rgb[i * 3 + 1] = static_cast<uint8_t>((g6 * 255 + 31) / 63);
            rgb[i * 3 + 2] = static_cast<uint8_t>((b5 * 255 + 15) / 31);
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
         return { }; // unsupported/unknown frame format
      }
      return rgb;
   }
}

bool IsDisplayKnown(uint64_t mapping)
{
   std::lock_guard lock(deviceStatesMutex);
   return displayGetters.find(mapping) != displayGetters.end();
}

// Looks up the display registered under `mapping` (the same CtlResId.id sent to the client as
// the display node's "mapping" field) and returns its current frame as a top-down RGB24 buffer.
// Returns an empty vector if the mapping is unknown, the source has no frame yet, or the frame
// format isn't supported.
std::vector<uint8_t> GetDisplayFrameRGB(uint64_t mapping, uint32_t& width, uint32_t& height, uint32_t& frameId)
{
   DisplayProvider provider;
   {
      std::lock_guard lock(deviceStatesMutex);
      auto it = displayGetters.find(mapping);
      if (it == displayGetters.end())
         return { };
      provider = it->second;
   }

   if (!provider.GetRenderFrame || provider.width == 0 || provider.height == 0)
      return { };

   const DisplayFrame frame = provider.GetRenderFrame(provider.id);
   if (!frame.frame)
      return { };

   width = provider.width;
   height = provider.height;
   frameId = frame.frameId;
   return ConvertFrameToRgb24(provider.width, provider.height, provider.frameFormat, frame.frame);
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
   msgApi->SubscribeMsg(endpointId, onStateSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_ON_SRC_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onDisplaySrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onSegSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG), OnSrcChanged, nullptr);
   msgApi->RegisterSetting(endpointId, &portSetting);

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

   msgApi->UnsubscribeMsg(onControllersChangedId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onStateSrcChgId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onDisplaySrcChgId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onSegSrcChgId, OnSrcChanged, nullptr);

   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onControllersChangedId);
   msgApi->ReleaseMsgID(onStateSrcChgId);
   msgApi->ReleaseMsgID(onDisplaySrcChgId);
   msgApi->ReleaseMsgID(onSegSrcChgId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
