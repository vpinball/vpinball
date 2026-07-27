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

#include <string>
using namespace std::string_literals;
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Inspector
{

const MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;

uint32_t endpointId;
unsigned int getVpxApiId;
unsigned int onControllerGameStartId, onControllerGameEndId;
unsigned int onStateSrcChgId, onDisplaySrcChgId, onSegSrcChgId;

std::vector<std::string> runningGames;

MSGPI_INT_VAL_SETTING(portSetting, "port", "Web Server Port", "Port used by the inspector web server", true, 1024, 65535, 2113);

std::unique_ptr<WebServer> webServer;

std::mutex deviceStatesMutex;
typedef struct
{
   unsigned int index;
   int typeMask;
   int(MSGPIAPI* GetState)(unsigned int index, int type, void* pResult);
} StateProvider;
std::map<uint64_t, StateProvider> stateGetters;
unsigned int treeId = 0;

void UpdateTreeCache()
{
   treeId++;
   if (!webServer)
      return;

   std::lock_guard lock(deviceStatesMutex);
   stateGetters.clear();

   json root = json::object();
   root["treeId"] = treeId;

   if (!msgApi)
   {
      webServer->UpdateTreeJson(root.dump());
      return;
   }

   json tree = json::array();
   if (!runningGames.empty())
   {
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
         GetStateSrcMsg devMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getStateMsgId, &devMsg);
         std::vector<StateSrcId> stateDefs(devMsg.count);
         devMsg = { static_cast<unsigned int>(stateDefs.size()), 0, stateDefs.data() };
         msgApi->BroadcastMsg(endpointId, getStateMsgId, &devMsg);
         for (unsigned int i = 0; i < devMsg.count; i++)
         {
            auto& cNode = getController(devMsg.entries[i].id.endpointId);
            json catNode = json::object();
            catNode["name"s] = "Game States";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            std::map<uint32_t, json> groups;
            for (unsigned int j = 0; j < devMsg.entries[i].nStates; j++)
            {
               json item = json::object();
               item["type"s] = "state";
               item["name"s] = devMsg.entries[i].stateDefs[j].name ? devMsg.entries[i].stateDefs[j].name : ("Device " + std::to_string(j));
               item["desc"s] = devMsg.entries[i].stateDefs[j].desc ? devMsg.entries[i].stateDefs[j].desc : "No description available";
               item["mapping"s] = std::to_string(devMsg.entries[i].stateDefs[j].id.mappingId);
               item["writable"s] = devMsg.entries[i].stateDefs[j].writable;
               item["dataType"s] = devMsg.entries[i].stateDefs[j].typeMask;
               int groupIndex = -1;
               for (unsigned int k = 0; k < devMsg.entries[i].nGroups; k++)
               {
                  if (devMsg.entries[i].groupDefs[k].id == devMsg.entries[i].stateDefs[j].id.groupId)
                  {
                     groupIndex = k;
                     break;
                  }
               }
               if (groupIndex >= 0)
               {
                  auto& cGroup = getGroup(groups, devMsg.entries[i].stateDefs[j].id.groupId, devMsg.entries[i].groupDefs[groupIndex].name);
                  cGroup["children"s].push_back(item);
                  stateGetters[devMsg.entries[i].stateDefs[j].id.mappingId] = { j, devMsg.entries[i].stateDefs[j].typeMask, devMsg.entries[i].GetState };
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
         GetDisplaySrcMsg dispMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getDisplaysMsgId, &dispMsg);
         std::vector<DisplaySrcId> displayDefs(dispMsg.count);
         dispMsg = { static_cast<unsigned int>(displayDefs.size()), 0, displayDefs.data() };
         msgApi->BroadcastMsg(endpointId, getDisplaysMsgId, &dispMsg);
         std::map<uint32_t, json> displayCats;
         for (unsigned int i = 0; i < dispMsg.count; i++)
         {
            uint32_t epId = dispMsg.entries[i].id.endpointId;
            if (displayCats.find(epId) == displayCats.end())
            {
               json catNode = json::object();
               catNode["name"s] = "Displays";
               catNode["type"s] = "category";
               catNode["children"s] = json::array();
               displayCats[epId] = catNode;
            }
            json item = json::object();
            item["name"s] = std::format("Display {} {}x{}", dispMsg.entries[i].id.resId, dispMsg.entries[i].width, dispMsg.entries[i].height);
            item["type"s] = "display";
            item["mapping"s] = std::to_string(dispMsg.entries[i].id.id);
            displayCats[epId]["children"s].push_back(item);
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
         GetSegSrcMsg segMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getSegsMsgId, &segMsg);
         std::vector<SegSrcId> displayDefs(segMsg.count);
         segMsg = { static_cast<unsigned int>(displayDefs.size()), 0, displayDefs.data() };
         msgApi->BroadcastMsg(endpointId, getSegsMsgId, &segMsg);
         for (unsigned int i = 0; i < segMsg.count; i++)
         {
            auto& cNode = getController(segMsg.entries[i].id.endpointId);
            json catNode = json::object();
            catNode["name"s] = "Segment Displays";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            json item = json::object();
            item["name"s] = std::format("Seg Display {}", segMsg.entries[i].id.resId);
            item["type"s] = "seg_display";
            catNode["children"s].push_back(item);
            cNode["children"s].push_back(catNode);
         }
         msgApi->ReleaseMsgID(getSegsMsgId);
      }

      for (auto& pair : controllers)
         tree.push_back(pair.second);
   }
   root["tree"] = tree;

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
   root["treeId"] = treeId;
   root["states"] = states;
   return root.dump();
}

void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   const CtlOnGameStateChgMsg* msg = static_cast<const CtlOnGameStateChgMsg*>(msgData);
   assert(msg && msg->gameId);
   runningGames.push_back(msg->gameId);
   UpdateTreeCache();
}

void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   const CtlOnGameStateChgMsg* msg = static_cast<const CtlOnGameStateChgMsg*>(msgData);
   if (const auto it = std::find(runningGames.begin(), runningGames.end(), msg->gameId); it != runningGames.end())
   {
      runningGames.erase(it);
      UpdateTreeCache();
   }
}

void OnSrcChanged(const unsigned int eventId, void* userData, void* msgData) { UpdateTreeCache(); }

} // namespace Inspector

using namespace Inspector;

MSGPI_EXPORT void MSGPIAPI InspectorPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);

   msgApi->SubscribeMsg(endpointId, onControllerGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnControllerGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onControllerGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), OnControllerGameEnd, nullptr);

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
   UpdateTreeCache();
}

MSGPI_EXPORT void MSGPIAPI InspectorPluginUnload()
{
   if (webServer)
   {
      webServer->Stop();
      webServer.reset();
   }

   msgApi->UnsubscribeMsg(onControllerGameStartId, OnControllerGameStart, nullptr);
   msgApi->UnsubscribeMsg(onControllerGameEndId, OnControllerGameEnd, nullptr);
   msgApi->UnsubscribeMsg(onStateSrcChgId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onDisplaySrcChgId, OnSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onSegSrcChgId, OnSrcChanged, nullptr);

   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onControllerGameStartId);
   msgApi->ReleaseMsgID(onControllerGameEndId);
   msgApi->ReleaseMsgID(onStateSrcChgId);
   msgApi->ReleaseMsgID(onDisplaySrcChgId);
   msgApi->ReleaseMsgID(onSegSrcChgId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
