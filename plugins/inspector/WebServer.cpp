// license:GPLv3+

#include "WebServer.h"
#include <cstdio>
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace Inspector {

constexpr const char* HEADER_JSON = "Content-Type: application/json\r\n";
constexpr int STATUS_OK = 200;

WebServer::WebServer()
{
   m_run = false;
   m_pThread = nullptr;
   m_treeJson = "[]"sv;
}

WebServer::~WebServer()
{
   Stop();
}

void WebServer::Start(int port)
{
   if (m_run) {
      printf("[Inspector] Web server already running\n");
      return;
   }

   std::string bindUrl = "http://0.0.0.0:" + std::to_string(port);

   printf("[Inspector] Starting web server at %s\n", bindUrl.c_str());

   mg_mgr_init(&m_mgr);

   if (mg_http_listen(&m_mgr, bindUrl.c_str(), &WebServer::EventHandler, this)) {
      m_run = true;
      printf("[Inspector] Web server started\n");

      m_pThread = std::make_unique<std::thread>([this]() {
         while (m_run)
            mg_mgr_poll(&m_mgr, 100);

         mg_mgr_free(&m_mgr);
         printf("[Inspector] Web server closed\n");
      });
   }
   else {
      printf("[Inspector] Unable to start web server\n");
   }
}

void WebServer::Stop()
{
   if (!m_run) {
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

void WebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data)
{
   WebServer* webServer = (WebServer*)c->fn_data;

   if (ev == MG_EV_HTTP_MSG) {
      struct mg_http_message *hm = (struct mg_http_message *) ev_data;

      if (mg_match(hm->uri, mg_str("/info"), NULL))
         webServer->Info(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/tree"), NULL))
         webServer->ApiTree(c, hm);
      else if (mg_match(hm->uri, mg_str("/"), NULL))
         webServer->Root(c, hm);
      else {
         mg_http_reply(c, 404, "", "Not found\n");
      }
   }
}

void WebServer::Info(struct mg_connection *c, struct mg_http_message* hm)
{
   const char* response = "{\"status\": \"ok\", \"plugin\": \"inspector\"}";
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response);
}

void WebServer::ApiTree(struct mg_connection *c, struct mg_http_message* hm)
{
   std::string response;
   {
      std::lock_guard<std::mutex> lock(m_treeMutex);
      response = m_treeJson;
   }
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::Root(struct mg_connection *c, struct mg_http_message* hm)
{
   const char* html = R"(<!DOCTYPE html>
<html>
<head>
    <title>Inspector TreeView</title>
    <style>
        body { font-family: 'Inter', sans-serif; background: #121212; color: #ffffff; padding: 20px; }
        ul { list-style-type: none; }
        .tree-root { padding-left: 0; }
        details > summary { cursor: pointer; padding: 4px; border-radius: 4px; transition: background 0.2s; }
        details > summary:hover { background: #222222; }
        .node-game { color: #4CAF50; font-weight: bold; }
        .node-controller { color: #2196F3; }
        .node-category { color: #FF9800; }
        .node-item { color: #E0E0E0; font-size: 0.9em; margin-left: 20px; padding: 2px 0; }
        h1 { font-weight: 300; border-bottom: 1px solid #333; padding-bottom: 10px; }
    </style>
</head>
<body>
    <h1>VPX Inspector</h1>
    <div id="tree">Loading...</div>

    <script>
        function buildNode(node) {
            if (!node || Object.keys(node).length === 0) return '<em>No active controllers</em>';
            if (Array.isArray(node)) {
                if (node.length === 0) return '<em>No active controllers</em>';
                let html = '';
                for (let child of node) {
                    html += `<li>${buildNode(child)}</li>`;
                }
                return html;
            }
            if (node.type === 'input' || node.type === 'device') {
                return `<div class="node-item">${node.mapping}: ${node.name}</div>`;
            }
            else if (node.type === 'display' || node.type === 'seg_display') {
                return `<div class="node-item">${node.name}</div>`;
            }
            
            let html = `<details open><summary class="node-${node.type}">${node.name}</summary><ul>`;
            if (node.children) {
                for (let child of node.children) {
                    html += `<li>${buildNode(child)}</li>`;
                }
            }
            html += `</ul></details>`;
            return html;
        }

        fetch('/api/tree')
            .then(res => res.json())
            .then(data => {
                document.getElementById('tree').innerHTML = `<ul class="tree-root">${buildNode(data)}</ul>`;
            })
            .catch(err => {
                document.getElementById('tree').innerHTML = `Error loading tree: ${err}`;
            });
    </script>
</body>
</html>)";

   mg_http_reply(c, STATUS_OK, "Content-Type: text/html\r\n", "%s", html);
}

} // namespace Inspector
