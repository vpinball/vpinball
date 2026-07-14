// license:GPLv3+

#include "WebServer.h"
#include <cstdio>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;


namespace Inspector
{

extern std::string GetStatesJson();

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
               mg_mgr_poll(&m_mgr, 100);

            mg_mgr_free(&m_mgr);
            printf("[Inspector] Web server closed\n");
         });
   }
   else
   {
      printf("[Inspector] Unable to start web server\n");
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
      else if (mg_match(hm->uri, mg_str("/"), NULL))
         webServer->Root(c, hm);
      else
      {
         mg_http_reply(c, 404, "", "Not found\n");
      }
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

void WebServer::Root(struct mg_connection* c, struct mg_http_message* hm)
{
   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, (m_assetPath + "/index.html").c_str(), &opts);
}
} // namespace Inspector
