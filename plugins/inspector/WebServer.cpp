// license:GPLv3+

#include "WebServer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <nlohmann/json.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;


namespace Inspector
{

extern std::string GetStatesJson();
extern SetSwitchResult SetSwitchState(const std::string& stateId, std::optional<bool> targetValue = std::nullopt, bool* outNewState = nullptr);
extern bool IsDisplayKnown(uint64_t mapping);
extern bool GetDisplayFrameRGB(uint64_t mapping, const uint32_t* lastFrameId, size_t headerSize, std::vector<uint8_t>& rgb, uint32_t& width, uint32_t& height, uint32_t& frameId);

constexpr const char* HEADER_JSON = "Content-Type: application/json\r\n";
constexpr int STATUS_OK = 200;

WebServer::WebServer()
{
   m_run = false;
   m_pThread = nullptr;
   m_treeJson = "[]"sv;
}

WebServer::~WebServer() { Stop(); }

void WebServer::Start(int port, const std::string& assetPath)
{
   if (m_run)
   {
      printf("[Inspector] Web server already running\n");
      return;
   }

   std::string bindUrl = "http://0.0.0.0:" + std::to_string(port);

   printf("[Inspector] Starting web server at %s\n", bindUrl.c_str());

   mg_mgr_init(&m_mgr);

   if (mg_http_listen(&m_mgr, bindUrl.c_str(), &WebServer::EventHandler, this))
   {
      m_run = true;
      m_assetPath = assetPath;
      printf("[Inspector] Web server started\n");

      m_pThread = std::make_unique<std::thread>(
         [this]()
         {
            while (m_run)
            {
               mg_mgr_poll(&m_mgr, m_displayWsClients.empty() ? 100 : 10);
               PushDisplayWsFrames();
            }

            mg_mgr_free(&m_mgr);
            printf("[Inspector] Web server closed\n");
         });
   }
   else
   {
      printf("[Inspector] Unable to start web server\n");
      mg_mgr_free(&m_mgr);
   }
}

void WebServer::Stop()
{
   if (!m_run)
   {
      return;
   }

   m_run = false;

   if (m_pThread && m_pThread->joinable())
      m_pThread->join();

   m_pThread.reset();
}

void WebServer::UpdateTreeJson(const std::string& json)
{
   std::lock_guard<std::mutex> lock(m_treeMutex);
   m_treeJson = json;
}

void WebServer::EventHandler(struct mg_connection* c, int ev, void* ev_data)
{
   WebServer* webServer = (WebServer*)c->fn_data;

   if (ev == MG_EV_HTTP_MSG)
   {
      struct mg_http_message* hm = (struct mg_http_message*)ev_data;

      if (mg_match(hm->uri, mg_str("/info"), NULL))
         webServer->Info(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/tree"), NULL))
         webServer->ApiTree(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/states"), NULL))
         webServer->ApiStates(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/state/toggle"), NULL) || mg_match(hm->uri, mg_str("/api/state"), NULL))
         webServer->ApiState(c, hm);
      else if (mg_match(hm->uri, mg_str("/ws/display"), NULL))
         webServer->DisplayWsUpgrade(c, hm);
      else if (mg_match(hm->uri, mg_str("/display-stream.js"), NULL))
         webServer->Asset(c, hm, "/display-stream.js");
      else if (mg_match(hm->uri, mg_str("/displays"), NULL) || mg_match(hm->uri, mg_str("/displays.html"), NULL))
         webServer->Displays(c, hm);
      else if (mg_match(hm->uri, mg_str("/"), NULL))
         webServer->Root(c, hm);
      else
      {
         mg_http_reply(c, 404, "", "Not found\n");
      }
   }
   else if (ev == MG_EV_WS_OPEN)
   {
      if (auto it = webServer->m_displayWsClients.find(c); it != webServer->m_displayWsClients.end())
         it->second.ready = true;
   }
   else if (ev == MG_EV_WS_CTL)
   {
      // Stop pushing as soon as the client starts the close handshake
      if ((((struct mg_ws_message*)ev_data)->flags & 15) == WEBSOCKET_OP_CLOSE)
         webServer->m_displayWsClients.erase(c);
   }
   else if (ev == MG_EV_CLOSE)
   {
      webServer->m_displayWsClients.erase(c);
   }
}

void WebServer::DisplayWsUpgrade(struct mg_connection* c, struct mg_http_message* hm)
{
   char idBuf[32];
   const int idLen = mg_http_get_var(&hm->query, "id", idBuf, sizeof(idBuf) - 1);
   if (idLen <= 0)
   {
      mg_http_reply(c, 400, "", "Missing or invalid 'id' parameter\n");
      return;
   }
   idBuf[idLen] = '\0';

   DisplayWsClient client;
   client.mapping = std::strtoull(idBuf, nullptr, 10);
   m_displayWsClients[c] = client;
   mg_ws_upgrade(c, hm, NULL);
}

// Pushes a binary message per new display frame to each streaming client:
// 12 byte header (uint32 LE width, height, frameId) followed by top-down RGB24 data
void WebServer::PushDisplayWsFrames()
{
   for (auto& [c, client] : m_displayWsClients)
   {
      if (!client.ready)
         continue;
      if (!IsDisplayKnown(client.mapping)) // Display is gone (e.g. table ended), close instead of going silent
      {
         client.ready = false;
         mg_ws_send(c, "", 0, WEBSOCKET_OP_CLOSE);
         continue;
      }
      if (c->send.len > 4 * 1024 * 1024) // Slow client, skip frames instead of growing the send buffer
         continue;
      uint32_t width, height, frameId;
      // Converted straight into m_displayWsFrame behind the header, and only when this client lacks the frame
      if (!GetDisplayFrameRGB(client.mapping, client.hasFrame ? &client.lastFrameId : nullptr, 12, m_displayWsFrame, width, height, frameId))
         continue;
      client.hasFrame = true;
      client.lastFrameId = frameId;
      memcpy(m_displayWsFrame.data() + 0, &width, 4);
      memcpy(m_displayWsFrame.data() + 4, &height, 4);
      memcpy(m_displayWsFrame.data() + 8, &frameId, 4);
      mg_ws_send(c, m_displayWsFrame.data(), m_displayWsFrame.size(), WEBSOCKET_OP_BINARY);
   }
}

void WebServer::Info(struct mg_connection* c, struct mg_http_message* hm)
{
   const char* response = "{\"status\": \"ok\", \"plugin\": \"inspector\"}";
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response);
}

void WebServer::ApiTree(struct mg_connection* c, struct mg_http_message* hm)
{
   std::string response;
   {
      std::lock_guard<std::mutex> lock(m_treeMutex);
      response = m_treeJson;
   }
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::ApiStates(struct mg_connection* c, struct mg_http_message* hm)
{
   std::string response = GetStatesJson();
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::ApiState(struct mg_connection* c, struct mg_http_message* hm)
{
   char idBuf[64] = { 0 };
   int idLen = mg_http_get_var(&hm->query, "id", idBuf, sizeof(idBuf) - 1);
   if (idLen <= 0)
      idLen = mg_http_get_var(&hm->body, "id", idBuf, sizeof(idBuf) - 1);
   
   std::optional<bool> optValue;
   char valBuf[16] = { 0 };
   int valLen = mg_http_get_var(&hm->query, "value", valBuf, sizeof(valBuf) - 1);
   if (valLen <= 0)
      valLen = mg_http_get_var(&hm->body, "value", valBuf, sizeof(valBuf) - 1);
   if (valLen > 0)
   {
      valBuf[valLen] = '\0';
      if (strcmp(valBuf, "true") == 0 || strcmp(valBuf, "1") == 0)
         optValue = true;
      else if (strcmp(valBuf, "false") == 0 || strcmp(valBuf, "0") == 0)
         optValue = false;
   }

   if (idLen <= 0 && hm->body.len > 0)
   {
      try
      {
         const auto bodyJson = nlohmann::json::parse(std::string_view(hm->body.buf, hm->body.len));
         if (bodyJson.contains("id") && bodyJson["id"].is_string())
         {
            const std::string idStr = bodyJson["id"];
            if (idStr.length() < sizeof(idBuf))
            {
               memcpy(idBuf, idStr.c_str(), idStr.length());
               idBuf[idStr.length()] = '\0';
               idLen = static_cast<int>(idStr.length());
            }
         }
         if (bodyJson.contains("value") && bodyJson["value"].is_boolean())
         {
            optValue = bodyJson["value"].get<bool>();
         }
      }
      catch (...)
      {
      }
   }

   if (idLen <= 0)
   {
      mg_http_reply(c, 400, HEADER_JSON, "{\"status\": \"error\", \"message\": \"Missing 'id' parameter\"}\n");
      return;
   }

   idBuf[idLen] = '\0';
   bool newState = false;
   const SetSwitchResult result = SetSwitchState(idBuf, optValue, &newState);

   switch (result)
   {
   case SetSwitchResult::Success: mg_http_reply(c, STATUS_OK, HEADER_JSON, "{\"status\": \"ok\", \"id\": \"%s\", \"state\": %s}\n", idBuf, newState ? "true" : "false"); break;
   case SetSwitchResult::NotFound: mg_http_reply(c, 404, HEADER_JSON, "{\"status\": \"error\", \"message\": \"State not found\"}\n"); break;
   case SetSwitchResult::NotASwitch: mg_http_reply(c, 400, HEADER_JSON, "{\"status\": \"error\", \"message\": \"State is not a switch\"}\n"); break;
   case SetSwitchResult::NotWritable: mg_http_reply(c, 403, HEADER_JSON, "{\"status\": \"error\", \"message\": \"Switch is read-only\"}\n"); break;
   }
}

void WebServer::Asset(struct mg_connection* c, struct mg_http_message* hm, const char* name)
{
   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, (m_assetPath + name).c_str(), &opts);
}

void WebServer::Root(struct mg_connection* c, struct mg_http_message* hm)
{
   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, (m_assetPath + "/index.html").c_str(), &opts);
}

void WebServer::Displays(struct mg_connection* c, struct mg_http_message* hm)
{
   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, (m_assetPath + "/displays.html").c_str(), &opts);
}
} // namespace Inspector
