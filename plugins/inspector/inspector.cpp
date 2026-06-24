// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "WebServer.h"

#include <format>
#include <vector>
#include <string>
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

void UpdateTreeCache() {
   if (!webServer || !msgApi) return;

   json root = json::array();
   
   if (!runningGames.empty()) {
      std::map<uint32_t, json> controllers;

      auto getController = [&](uint32_t epId) -> json& {
         if (controllers.find(epId) == controllers.end()) {
            MsgEndpointInfo info;
            msgApi->GetEndpointInfo(epId, &info);
            json cNode = json::object();
            cNode["id"] = epId;
            cNode["name"] = info.name ? info.name : (info.id ? info.id : "Unknown Controller");
            cNode["type"] = "controller";
            cNode["children"] = json::array();
            controllers[epId] = cNode;
         }
         return controllers[epId];
      };

      auto getGroup = [](std::map<uint32_t, json>& groups, uint16_t groupId, const std::string& name) -> json&
      {
         if (groups.find(groupId) == groups.end())
         {
            json cNode = json::object();
            cNode["id"] = groupId;
            cNode["name"] = name;
            cNode["type"] = "group";
            cNode["children"] = json::array();
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
            catNode["name"] = "Inputs";
            catNode["type"] = "category";
            catNode["children"] = json::array();
            std::map<uint32_t, json> groups;
            for (unsigned int j = 0; j < inMsg.entries[i].nInputs; j++)
            {
               json item = json::object();
               item["name"] = inMsg.entries[i].inputDefs[j].name ? inMsg.entries[i].inputDefs[j].name : ("Input " + std::to_string(j));
               item["mapping"] = std::format("{:04x}", inMsg.entries[i].inputDefs[j].id.deviceId);
               item["type"] = "input";
               auto& cGroup = getGroup(groups, inMsg.entries[i].inputDefs[j].id.groupId, std::format("Input Group 0x{:04x}", inMsg.entries[i].inputDefs[j].id.groupId));
               cGroup["children"].push_back(item);
            }
            for (auto& pair : groups)
               catNode["children"].push_back(pair.second);
            cNode["children"].push_back(catNode);
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
            catNode["name"] = "Devices";
            catNode["type"] = "category";
            catNode["children"] = json::array();
            std::map<uint32_t, json> groups;
            for (unsigned int j = 0; j < devMsg.entries[i].nDevices; j++)
            {
               json item = json::object();
               item["name"] = devMsg.entries[i].deviceDefs[j].name ? devMsg.entries[i].deviceDefs[j].name : ("Device " + std::to_string(j));
               item["mapping"] = std::format("{:04x}", devMsg.entries[i].deviceDefs[j].id.deviceId);
               item["type"] = "device";
               auto& cGroup = getGroup(groups, devMsg.entries[i].deviceDefs[j].id.groupId, std::format("Device Group 0x{:04x}", devMsg.entries[i].deviceDefs[j].id.groupId));
               cGroup["children"].push_back(item);
            }
            for (auto& pair : groups)
               catNode["children"].push_back(pair.second);
            cNode["children"].push_back(catNode);
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
            catNode["name"] = "Displays";
            catNode["type"] = "category";
            catNode["children"] = json::array();
            json item = json::object();
            item["name"] = std::format("Display {} {}x{}", dispMsg.entries[i].id.resId, dispMsg.entries[i].width, dispMsg.entries[i].height);
            item["type"] = "display";
            catNode["children"].push_back(item);
            cNode["children"].push_back(catNode);
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
            catNode["name"] = "Segment Displays";
            catNode["type"] = "category";
            catNode["children"] = json::array();
            json item = json::object();
            item["name"] = std::format("Seg Display {}", segMsg.entries[i].id.resId);
            item["type"] = "seg_display";
            catNode["children"].push_back(item);
            cNode["children"].push_back(catNode);
         }
         msgApi->ReleaseMsgID(getSegsMsgId);
      }

      for (auto& pair : controllers)
         root.push_back(pair.second);
   }

   webServer->UpdateTreeJson(root.dump());
}

void onCtlGameStart(const unsigned int eventId, void* userData, void* msgData) {
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   std::string gameId = msg && msg->gameId ? msg->gameId : "Unknown Game";
   runningGames.push_back(gameId);
   UpdateTreeCache();
}

void onCtlGameEnd(const unsigned int eventId, void* userData, void* msgData) {
   if (!runningGames.empty()) {
      runningGames.pop_back();
   }
   UpdateTreeCache();
}

void onSrcChanged(const unsigned int eventId, void* userData, void* msgData) {
   UpdateTreeCache();
}

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
   if (webServer) {
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
