#pragma once

#include <thread>
#include "mongoose.h"

class WebServer {
public:
    WebServer();
    ~WebServer();

    static void EventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

    void Start();
    void Stop();
    bool IsRunning();
    string GetUrl();

#ifdef __ANDROID__
    void Init(string addr, int port, bool debug, string prefPath, string myPath);
#endif

private:
    bool Unzip(const char* pSource);
    void Files(struct mg_connection *c, struct mg_http_message* hm);
    void Download(struct mg_connection *c, struct mg_http_message* hm);
    void Upload(struct mg_connection *c, struct mg_http_message* hm);
    void Delete(struct mg_connection *c, struct mg_http_message* hm);
    void Folder(struct mg_connection *c, struct mg_http_message* hm);
    void Extract(struct mg_connection *c, struct mg_http_message* hm);
    void Activate(struct mg_connection *c, struct mg_http_message* hm);
    void Command(struct mg_connection *c, struct mg_http_message* hm);
    string GetIPAddress();

    struct mg_mgr m_mgr;

    bool m_run;
    std::thread* m_pThread;
    string m_url;

    string m_addr;
    int m_port;
    bool m_debug;
    string m_szMyPrefPath;
};
