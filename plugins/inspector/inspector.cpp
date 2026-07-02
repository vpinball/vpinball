// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "WebServer.h"

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
unsigned int onCtlGameStartId, onCtlGameEndId;
unsigned int onInputSrcChgId, onDeviceSrcChgId, onDisplaySrcChgId, onSegSrcChgId;

std::vector<std::string> runningGames;

MSGPI_INT_VAL_SETTING(portSetting, "port", "Web Server Port", "Port used by the inspector web server", true, 1024, 65535, 2113);

std::unique_ptr<WebServer> webServer;

std::mutex deviceStatesMutex;
typedef float(MSGPIAPI* GetFloatState)(const unsigned int);
std::map<uint32_t, std::pair<unsigned int, GetFloatState>> deviceGetters;
typedef int(MSGPIAPI* GetInputState)(const unsigned int);
std::map<uint32_t, std::pair<unsigned int, GetInputState>> inputGetters;

void UpdateTreeCache()
{
   if (!webServer)
      return;

   std::lock_guard lock(deviceStatesMutex);
   deviceGetters.clear();
   inputGetters.clear();

   json root = json::array();

   if (!msgApi)
   {
      webServer->UpdateTreeJson(root.dump());
      return;
   }

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

      // Inputs
      {
         unsigned int getInputsMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
         GetInputSrcMsg inMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getInputsMsgId, &inMsg);
         std::vector<InputSrcId> inputDefs(inMsg.count);
         inMsg = { static_cast<unsigned int>(inputDefs.size()), 0, inputDefs.data() };
         msgApi->BroadcastMsg(endpointId, getInputsMsgId, &inMsg);
         for (unsigned int i = 0; i < inMsg.count; i++)
         {
            auto& cNode = getController(inMsg.entries[i].id.endpointId);
            json catNode = json::object();
            catNode["name"s] = "Inputs";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            std::map<uint32_t, json> groups;
            for (unsigned int j = 0; j < inMsg.entries[i].nInputs; j++)
            {
               json item = json::object();
               item["name"s] = inMsg.entries[i].inputDefs[j].name ? inMsg.entries[i].inputDefs[j].name : ("Input " + std::to_string(j));
               item["mapping"s] = inMsg.entries[i].inputDefs[j].id.mappingId;
               item["type"s] = "input";
               auto& cGroup = getGroup(groups, inMsg.entries[i].inputDefs[j].id.groupId, std::format("Input Group 0x{:04x}", inMsg.entries[i].inputDefs[j].id.groupId));
               cGroup["children"s].push_back(item);
               inputGetters[inMsg.entries[i].inputDefs[j].id.mappingId] = { j, inMsg.entries[i].GetInputState };
            }
            for (auto& pair : groups)
               catNode["children"s].push_back(pair.second);
            cNode["children"s].push_back(catNode);
         }
         msgApi->ReleaseMsgID(getInputsMsgId);
      }

      // Devices
      {
         unsigned int getDevicesMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
         GetDevSrcMsg devMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getDevicesMsgId, &devMsg);
         std::vector<DevSrcId> deviceDefs(devMsg.count);
         devMsg = { static_cast<unsigned int>(deviceDefs.size()), 0, deviceDefs.data() };
         msgApi->BroadcastMsg(endpointId, getDevicesMsgId, &devMsg);
         for (unsigned int i = 0; i < devMsg.count; i++)
         {
            auto& cNode = getController(devMsg.entries[i].id.endpointId);
            json catNode = json::object();
            catNode["name"s] = "Devices";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            std::map<uint32_t, json> groups;
            for (unsigned int j = 0; j < devMsg.entries[i].nDevices; j++)
            {
               json item = json::object();
               item["type"s] = "device";
               item["name"s] = devMsg.entries[i].deviceDefs[j].name ? devMsg.entries[i].deviceDefs[j].name : ("Device " + std::to_string(j));
               item["mapping"s] = devMsg.entries[i].deviceDefs[j].id.mappingId;
               item["state"s] = devMsg.entries[i].GetFloatState(j);
               auto& cGroup = getGroup(groups, devMsg.entries[i].deviceDefs[j].id.groupId, std::format("Device Group 0x{:04x}", devMsg.entries[i].deviceDefs[j].id.groupId));
               cGroup["children"s].push_back(item);
               deviceGetters[devMsg.entries[i].deviceDefs[j].id.mappingId] = { j, devMsg.entries[i].GetFloatState };
            }
            for (auto& pair : groups)
               catNode["children"s].push_back(pair.second);
            cNode["children"s].push_back(catNode);
         }
         msgApi->ReleaseMsgID(getDevicesMsgId);
      }

      // Displays
      {
         unsigned int getDisplaysMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
         GetDisplaySrcMsg dispMsg = { 0, 0, nullptr };
         msgApi->BroadcastMsg(endpointId, getDisplaysMsgId, &dispMsg);
         std::vector<DisplaySrcId> displayDefs(dispMsg.count);
         dispMsg = { static_cast<unsigned int>(displayDefs.size()), 0, displayDefs.data() };
         msgApi->BroadcastMsg(endpointId, getDisplaysMsgId, &dispMsg);
         for (unsigned int i = 0; i < dispMsg.count; i++)
         {
            auto& cNode = getController(dispMsg.entries[i].id.endpointId);
            json catNode = json::object();
            catNode["name"s] = "Displays";
            catNode["type"s] = "category";
            catNode["children"s] = json::array();
            json item = json::object();
            item["name"s] = std::format("Display {} {}x{}", dispMsg.entries[i].id.resId, dispMsg.entries[i].width, dispMsg.entries[i].height);
            item["type"s] = "display";
            catNode["children"s].push_back(item);
            cNode["children"s].push_back(catNode);
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
         root.push_back(pair.second);
   }

   webServer->UpdateTreeJson(root.dump());
}

std::string GetDeviceStatesJson()
{
   std::lock_guard lock(deviceStatesMutex);

   json root = json::array();
   for (const auto& pair : deviceGetters) 
   {
      json dItem = json::object();
      dItem["id"s] = pair.first;
      dItem["state"s] = pair.second.second(pair.second.first);
      root.push_back(dItem);
   }
   return root.dump();
}

std::string GetInputStatesJson()
{
   std::lock_guard lock(deviceStatesMutex);

   json root = json::array();
   for (const auto& pair : inputGetters)
   {
      json dItem = json::object();
      dItem["id"s] = pair.first;
      dItem["state"s] = pair.second.second(pair.second.first) != 0;
      root.push_back(dItem);
   }
   return root.dump();
}

void onCtlGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   std::string gameId = msg && msg->gameId ? msg->gameId : "Unknown Game";
   runningGames.push_back(gameId);
   UpdateTreeCache();
}

void onCtlGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   if (!runningGames.empty())
   {
      runningGames.pop_back();
   }
   UpdateTreeCache();
}

void onSrcChanged(const unsigned int eventId, void* userData, void* msgData) { UpdateTreeCache(); }

} // namespace Inspector

using namespace Inspector;

MSGPI_EXPORT void MSGPIAPI InspectorPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);

   msgApi->SubscribeMsg(endpointId, onCtlGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), onCtlGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onCtlGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), onCtlGameEnd, nullptr);

   msgApi->SubscribeMsg(endpointId, onInputSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG), onSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onDeviceSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG), onSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onDisplaySrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), onSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onSegSrcChgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG), onSrcChanged, nullptr);
   msgApi->RegisterSetting(endpointId, &portSetting);

   webServer = std::make_unique<WebServer>();
   webServer->Start(portSetting_Get());
   UpdateTreeCache();
}

MSGPI_EXPORT void MSGPIAPI InspectorPluginUnload()
{
   if (webServer)
   {
      webServer->Stop();
      webServer.reset();
   }

   msgApi->UnsubscribeMsg(onCtlGameStartId, onCtlGameStart, nullptr);
   msgApi->UnsubscribeMsg(onCtlGameEndId, onCtlGameEnd, nullptr);
   msgApi->UnsubscribeMsg(onInputSrcChgId, onSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onDeviceSrcChgId, onSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onDisplaySrcChgId, onSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onSegSrcChgId, onSrcChanged, nullptr);

   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onCtlGameStartId);
   msgApi->ReleaseMsgID(onCtlGameEndId);
   msgApi->ReleaseMsgID(onInputSrcChgId);
   msgApi->ReleaseMsgID(onDeviceSrcChgId);
   msgApi->ReleaseMsgID(onDisplaySrcChgId);
   msgApi->ReleaseMsgID(onSegSrcChgId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
