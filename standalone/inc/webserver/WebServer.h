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
    static void LogAppender(const std::string& formattedLog);
    void Start();
    void Stop();
    bool IsRunning() { return m_run; }
    string GetUrl();
    void SetLastUpdate();

private:
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
    void TableList(struct mg_connection *c, struct mg_http_message* hm);
    void TableImport(struct mg_connection *c, struct mg_http_message* hm);
    void TableExport(struct mg_connection *c, struct mg_http_message* hm);
    void TableRename(struct mg_connection *c, struct mg_http_message* hm);

    void AddLogEntry(const std::string& formattedLog);
    void BroadcastLogEntry(const std::string& formattedLog);
    static void BroadcastStatus();

    string GetIPAddress();
    bool ValidatePathParameter(struct mg_connection *c, struct mg_http_message* hm, const char* paramName, string& outValue);
    std::filesystem::path BuildUserPath(const char* relativePath);
    std::filesystem::path GetTempDirectory();
    void CleanupTempDirectory();
    string LookupTableName(const string& tableId);
    bool Unzip(const char* pSource);

    struct mg_mgr m_mgr;
    bool m_run;
    std::thread* m_pThread;
    string m_url;
    static std::mutex s_logMutex;
    static std::vector<struct mg_connection*> s_logConnections;
    static std::vector<struct mg_connection*> s_statusConnections;
    static std::deque<std::string> s_recentLogs;
    static const size_t MAX_RECENT_LOGS = 1000;
    static WebServer* s_instance;
    static int64_t s_lastUpdateTimestamp;
};
