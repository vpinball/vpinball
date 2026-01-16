// license:GPLv3+

#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <queue>
#include <deque>
#include <filesystem>
#include <mongoose/mongoose.h>

class WebServer {
public:
   WebServer();
   ~WebServer();

   static void EventHandler(struct mg_connection *c, int ev, void *ev_data);
   static void LogAppender(const string& formattedLog);
   static void BroadcastStatus();
   void Update();
   bool IsRunning() { return m_run; }
   string GetUrl();

private:
   void Start();
   void Stop();
   void SetLastUpdate();
   void Info(struct mg_connection *c, struct mg_http_message* hm);
   void Status(struct mg_connection *c, struct mg_http_message* hm);
   void Assets(struct mg_connection *c, struct mg_http_message* hm);
   void Files(struct mg_connection *c, struct mg_http_message* hm);
   void Download(struct mg_connection *c, struct mg_http_message* hm);
   void Upload(struct mg_connection *c, struct mg_http_message* hm);
   void Delete(struct mg_connection *c, struct mg_http_message* hm);
   void Rename(struct mg_connection *c, struct mg_http_message* hm);
   void Folder(struct mg_connection *c, struct mg_http_message* hm);
   void Extract(struct mg_connection *c, struct mg_http_message* hm);
   void Command(struct mg_connection *c, struct mg_http_message* hm);
   void LogStream(struct mg_connection *c, struct mg_http_message* hm);

   void AddLogEntry(const string& formattedLog);
   void BroadcastLogEntry(const string& formattedLog);

   string GetIPAddress();
   bool ValidatePathParameter(struct mg_connection *c, struct mg_http_message* hm, const char* paramName, string& outValue);
   std::filesystem::path BuildTablePath(const char* relativePath);
   bool Unzip(const char* pSource);

   struct mg_mgr m_mgr;
   bool m_run;
   std::unique_ptr<std::thread> m_pThread;
   string m_url;
   static std::mutex s_logMutex;
   static vector<struct mg_connection*> s_logConnections;
   static vector<struct mg_connection*> s_statusConnections;
   static std::deque<string> s_recentLogs;
   static const size_t MAX_RECENT_LOGS = 1000;
   static WebServer* s_instance;
   static int64_t s_lastUpdateTimestamp;
};
