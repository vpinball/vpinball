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
#include <optional>

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

static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>> controllerSources;
static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<SegSrcId>> segSources;
static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<DisplaySrcId>> displaySources;
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
   void* callContext;
   DisplayFrame(MSGPIAPI* GetRenderFrame)(void*);
} DisplayProvider;
static std::map<uint64_t, DisplayProvider> displayGetters;
static unsigned int treeId = 0;

void UpdateTreeCache()
{
   treeId++;
   if (!webServer)
      return;

   json root = json::object();
   root["treeId"s] = treeId;
   root["tree"s] = json::array();

   if (!msgApi)
   {
      webServer->UpdateTreeJson(root.dump());
      return;
   }

   // Controllers
   std::map<uint32_t, std::unique_ptr<json>> controllers;
   controllerSources->With(
      [&controllers](const std::vector<ControllerDef>& items)
      {
         for (const ControllerDef& controller : items)
         {
            MsgEndpointInfo info;
            msgApi->GetEndpointInfo(controller.endpointId, &info);
            json cNode = json::object();
            cNode["id"s] = controller.endpointId;
            cNode["name"s] = info.name ? info.name : (info.id ? info.id : "Unknown Controller");
            cNode["type"s] = "controller";
            cNode["children"s] = json::array();
            cNode["game"s] = controller.gameId ? controller.gameId : "";
            controllers[controller.endpointId] = std::make_unique<json>(cNode);
         }
      });

   auto getController = [&](uint32_t epId) -> json*
   {
      if (auto it = controllers.find(epId); it == controllers.end())
      {
         MsgEndpointInfo info;
         msgApi->GetEndpointInfo(epId, &info);
         json cNode = json::object();
         cNode["id"s] = epId;
         cNode["name"s] = info.name ? info.name : (info.id ? info.id : "Unknown Controller");
         cNode["type"s] = "controller";
         cNode["children"s] = json::array();
         controllers[epId] = std::make_unique<json>(cNode);
      }
      return controllers[epId].get();
   };

   std::lock_guard lock(displayStateMutex);
   displayGetters.clear();

   json tree = json::array();

   // States
   stateSources->With(
      [&getController](const std::vector<StateSrcId>& items)
      {
         for (const StateSrcId& stateDef : items)
         {
            auto* cNode = getController(stateDef.id.endpointId);
            if (!cNode)
               continue;

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
               item["writable"s] = stateDef.stateDefs[j].SetState != nullptr;
               gNode["children"s].push_back(item);
            }
            (*cNode)["children"s].push_back(gNode);
         }
      });

   // Displays
   displaySources->With(
      [&getController](const std::vector<DisplaySrcId>& items)
      {
         std::map<uint32_t, json> displayCats;
         for (const DisplaySrcId& displayDef : items)
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
            displayGetters[displayDef.id.id] = { displayDef.id, displayDef.width, displayDef.height, displayDef.frameFormat, displayDef.callContext, displayDef.GetRenderFrame };
         }
         for (auto& pair : displayCats)
         {
            auto* cNode = getController(pair.first);
            if (!cNode)
               continue;
            (*cNode)["children"s].push_back(pair.second);
         }
      });

   // Segment Displays
   segSources->With(
      [&getController](const std::vector<SegSrcId>& items)
      {
         for (const SegSrcId& segDef : items)
         {
            auto* cNode = getController(segDef.id.endpointId);
            if (!cNode)
               continue;
            json catNode = json::object();
            catNode["name"s] = "Segment Displays";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            json item = json::object();
            item["type"s] = "seg_display";
            item["name"s] = std::format("Seg Display {}", segDef.id.resId);
            item["mapping"s] = std::format("{:02d}", segDef.id.resId);
            catNode["children"s].push_back(item);
            (*cNode)["children"s].push_back(catNode);
         }
      });

   for (auto& pair : controllers)
      tree.push_back(*pair.second);

   root["tree"s] = tree;

   webServer->UpdateTreeJson(root.dump());
}

std::string GetStatesJson()
{
   json root = json::object();
   root["treeId"s] = treeId;
   root["states"s] = stateSources->With(
      [](const std::vector<StateSrcId>& items)
      {
         json states = json::array();
         for (const StateSrcId& stateDef : items)
         {
            json dItem = json::object();
            for (unsigned int j = 0; j < stateDef.nStates; j++)
            {
               const StateDef& def = stateDef.stateDefs[j];
               if (def.GetState == nullptr)
                  continue;
               dItem["id"s] = std::format("{:04X}.{:04X}.{:04X}", stateDef.id.endpointId, stateDef.id.resId, def.mappingId);
               dItem["type"s] = def.semanticType;
               switch (def.dataFormat)
               {
               case CTLPI_STATE_FORMAT_FLOAT:
               {
                  float state;
                  def.GetState(def.callContext, &state);
                  dItem["format"s] = "float";
                  dItem["state"s] = state;
                  states.push_back(dItem);
               }
               break;

               case CTLPI_STATE_FORMAT_UINT8:
               {
                  uint8_t state;
                  def.GetState(def.callContext, &state);
                  dItem["format"s] = "uint8";
                  dItem["state"s] = state;
                  states.push_back(dItem);
               }
               break;

               case CTLPI_STATE_FORMAT_INT32:
               {
                  int32_t state;
                  def.GetState(def.callContext, &state);
                  dItem["format"s] = "int32";
                  dItem["state"s] = state;
                  states.push_back(dItem);
               }
               break;

               case CTLPI_STATE_FORMAT_INT64:
               {
                  int64_t state;
                  def.GetState(def.callContext, &state);
                  dItem["format"s] = "int64";
                  dItem["state"s] = std::to_string(state);
                  states.push_back(dItem);
               }
               break;

               case CTLPI_STATE_FORMAT_STRING:
               {
                  char* state = nullptr;
                  def.GetState(def.callContext, &state);
                  dItem["format"s] = "string";
                  dItem["state"s] = state != nullptr ? state : "";
                  states.push_back(dItem);
               }
               break;
               }
            }
         }
         return states;
      });
   return root.dump();
}

SetSwitchResult SetSwitchState(const std::string& stateId, std::optional<bool> targetValue, bool* outNewState)
{
   if (!stateSources)
      return SetSwitchResult::NotFound;

   uint32_t epId = 0, resId = 0, mapId = 0;
#ifndef _WIN32
#define sscanf_s sscanf
#endif
   const bool hasParsed = (sscanf_s(stateId.c_str(), "%x.%x.%x", &epId, &resId, &mapId) == 3);

   SetSwitchResult result = SetSwitchResult::NotFound;

   stateSources->With(
      [&](const std::vector<StateSrcId>& items)
      {
         for (const StateSrcId& stateDef : items)
         {
            if (hasParsed && (stateDef.id.endpointId != epId || stateDef.id.resId != resId))
               continue;

            for (unsigned int j = 0; j < stateDef.nStates; j++)
            {
               const StateDef& def = stateDef.stateDefs[j];
               const bool match = hasParsed ? (def.mappingId == mapId) : (std::format("{:04X}.{:04X}.{:04X}", stateDef.id.endpointId, stateDef.id.resId, def.mappingId) == stateId);

               if (match)
               {
                  if (def.semanticType != CTLPI_STATE_TYPE_SWITCH)
                  {
                     result = SetSwitchResult::NotASwitch;
                     return;
                  }

                  if (def.SetState == nullptr)
                  {
                     result = SetSwitchResult::NotWritable;
                     return;
                  }

                  bool current = false;
                  if (def.GetState != nullptr)
                  {
                     switch (def.dataFormat)
                     {
                     case CTLPI_STATE_FORMAT_UINT8:
                     {
                        uint8_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_INT8:
                     {
                        int8_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_UINT16:
                     {
                        uint16_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_INT16:
                     {
                        int16_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_UINT32:
                     {
                        uint32_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_INT32:
                     {
                        int32_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_UINT64:
                     {
                        uint64_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_INT64:
                     {
                        int64_t v = 0;
                        def.GetState(def.callContext, &v);
                        current = (v != 0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_FLOAT:
                     {
                        float v = 0.0f;
                        def.GetState(def.callContext, &v);
                        current = (v > 0.0f);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_DOUBLE:
                     {
                        double v = 0.0;
                        def.GetState(def.callContext, &v);
                        current = (v > 0.0);
                        break;
                     }
                     case CTLPI_STATE_FORMAT_STRING:
                     {
                        char* v = nullptr;
                        def.GetState(def.callContext, &v);
                        current = (v != nullptr && *v != '\0' && strcmp(v, "0") != 0);
                        break;
                     }
                     default: break;
                     }
                  }

                  const bool target = targetValue.has_value() ? *targetValue : !current;

                  switch (def.dataFormat)
                  {
                  case CTLPI_STATE_FORMAT_UINT8:
                  {
                     uint8_t bv = target ? 0xFF : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_INT8:
                  {
                     int8_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_UINT16:
                  {
                     uint16_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_INT16:
                  {
                     int16_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_UINT32:
                  {
                     uint32_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_INT32:
                  {
                     int32_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_UINT64:
                  {
                     uint64_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_INT64:
                  {
                     int64_t bv = target ? 1 : 0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_FLOAT:
                  {
                     float bv = target ? 1.0f : 0.0f;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_DOUBLE:
                  {
                     double bv = target ? 1.0 : 0.0;
                     def.SetState(def.callContext, &bv);
                     break;
                  }
                  case CTLPI_STATE_FORMAT_STRING:
                  {
                     const char* bv = target ? "1" : "0";
                     def.SetState(def.callContext, bv);
                     break;
                  }
                  default: break;
                  }

                  if (outNewState)
                     *outNewState = target;

                  result = SetSwitchResult::Success;
                  return;
               }
            }
         }
      });

   return result;
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

   const DisplayFrame frame = provider.GetRenderFrame(provider.callContext);
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

   msgApi->RegisterSetting(endpointId, &portSetting);

   controllerSources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG, nullptr, nullptr, []() { UpdateTreeCache(); });

   displaySources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<DisplaySrcId>>(
      msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, nullptr, nullptr, []() { UpdateTreeCache(); });

   segSources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<SegSrcId>>(
      msgApi, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG, nullptr, nullptr, []() { UpdateTreeCache(); });

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

   controllerSources->Subscribe();
   displaySources->Subscribe();
   segSources->Subscribe();
   stateSources->Subscribe();
   UpdateTreeCache();
}

MSGPI_EXPORT void MSGPIAPI InspectorPluginUnload()
{
   if (webServer)
   {
      webServer->Stop();
      webServer.reset();
   }

   controllerSources->Unsubscribe();
   displaySources->Unsubscribe();
   segSources->Unsubscribe();
   stateSources->Unsubscribe();

   controllerSources = nullptr;
   displaySources = nullptr;
   segSources = nullptr;
   stateSources = nullptr;

   msgApi->ReleaseMsgID(getVpxApiId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
