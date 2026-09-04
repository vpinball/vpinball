// license:GPLv3+

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <string>
#include <memory>
#include <mongoose/mongoose.h>

#include <vector>
#include <string>
#include <mutex>
#include <map>

namespace Inspector {

enum class SetSwitchResult
{
   Success,
   NotFound,
   NotASwitch,
   NotWritable
};

class WebServer
{
public:
   WebServer();
   ~WebServer();

   void Start(int port, const std::string& assetPath);
   void Stop();

   void UpdateTreeJson(const std::string& json);

   static void EventHandler(struct mg_connection *c, int ev, void *ev_data);

private:
   void Info(struct mg_connection *c, struct mg_http_message* hm);
   void ApiTree(struct mg_connection *c, struct mg_http_message* hm);
   void ApiStates(struct mg_connection *c, struct mg_http_message *hm);
   void ApiState(struct mg_connection *c, struct mg_http_message *hm);
   void Asset(struct mg_connection *c, struct mg_http_message *hm, const char *name);
   void Root(struct mg_connection *c, struct mg_http_message *hm);
   void Displays(struct mg_connection *c, struct mg_http_message *hm);
   void DisplayWsUpgrade(struct mg_connection *c, struct mg_http_message *hm);
   void PushDisplayWsFrames();

   // Display streaming clients, only accessed from the server thread
   struct DisplayWsClient
   {
      uint64_t mapping = 0;
      uint32_t lastFrameId = 0;
      bool hasFrame = false;
      bool ready = false;
   };
   std::map<struct mg_connection*, DisplayWsClient> m_displayWsClients;
   std::vector<uint8_t> m_displayWsFrame; // Scratch buffer for the pushed message, its capacity is reused across frames

   std::mutex m_treeMutex;
   std::string m_treeJson;
   std::string m_assetPath;

   struct mg_mgr m_mgr;
   std::atomic<bool> m_run;
   std::unique_ptr<std::thread> m_pThread;
};

} // namespace Inspector
