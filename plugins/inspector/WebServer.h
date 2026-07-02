// license:GPLv3+

#pragma once

#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include <mongoose/mongoose.h>

#include <vector>
#include <string>
#include <mutex>

namespace Inspector {

class WebServer {
public:
   WebServer();
   ~WebServer();

   void Start(int port);
   void Stop();

   void UpdateTreeJson(const std::string& json);

   static void EventHandler(struct mg_connection *c, int ev, void *ev_data);

private:
   void Info(struct mg_connection *c, struct mg_http_message* hm);
   void ApiTree(struct mg_connection *c, struct mg_http_message* hm);
   void ApiInputStates(struct mg_connection *c, struct mg_http_message *hm);
   void ApiDeviceStates(struct mg_connection *c, struct mg_http_message *hm);
   void Root(struct mg_connection *c, struct mg_http_message* hm);

   std::mutex m_treeMutex;
   std::string m_treeJson;

   struct mg_mgr m_mgr;
   bool m_run;
   std::unique_ptr<std::thread> m_pThread;
};

} // namespace Inspector
