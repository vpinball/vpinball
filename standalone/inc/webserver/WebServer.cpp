#include "stdafx.h"
#include "WebServer.h"
#include <ifaddrs.h>

void WebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
   WebServer* webServer = (WebServer*)fn_data;

   if (ev == MG_EV_HTTP_MSG) {
      struct mg_http_message *hm = (struct mg_http_message *) ev_data;

      if (mg_http_match_uri(hm, "/files"))
         webServer->Files(c, hm);
      else if (mg_http_match_uri(hm, "/download"))
         webServer->Download(c, hm);
      else if (mg_http_match_uri(hm, "/upload"))
         webServer->Upload(c, hm);
      else if (mg_http_match_uri(hm, "/delete"))
         webServer->Delete(c, hm);
      else if (mg_http_match_uri(hm, "/folder"))
         webServer->Folder(c, hm);
      else if (mg_http_match_uri(hm, "/activate"))
         webServer->Activate(c, hm);
      else if (mg_http_match_uri(hm, "/command"))
         webServer->Command(c, hm);
      else {
         string path = g_pvp->m_szMyPath + "res" + PATH_SEPARATOR_CHAR + "vpx.html";

         struct mg_http_serve_opts opts = {};
         mg_http_serve_file(c, hm, path.c_str(), &opts);
      }
   }
}

WebServer::WebServer() 
{
   m_run = false;
   m_pThread = NULL;

   m_addr = LoadValueWithDefault(regKey[RegName::Standalone], "WebServerAddr"s, "0.0.0.0"s);
   m_port = LoadValueWithDefault(regKey[RegName::Standalone], "WebServerPort"s, 2112);
}

WebServer::~WebServer() 
{
   m_run = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }
}

void WebServer::Files(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   PLOGI.printf("Retrieving file list: q=%s", q);

   string path = g_pvp->m_szMyPrefPath + q;

   if (*q != '\0')
      path += PATH_SEPARATOR_CHAR;

   DIR* dir = opendir(path.c_str());

   if (!dir) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string json = "[ ";

   int i = 0;

   struct dirent *entry;

   while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
         continue;

      if (i > 0)
         json += ", ";

      string file = path + entry->d_name;
      string ext = "";

      if (entry->d_type != DT_DIR)
         ext = extension_from_path(file);

      struct stat st;
      if (stat(file.c_str(), &st) == 0) {
         char* buf = mg_mprintf("{ %m: %m, %m: %m, %m: %s, %m: %lld }",
            MG_ESC("name"), MG_ESC(entry->d_name),
            MG_ESC("ext"), MG_ESC(ext.c_str()),
            MG_ESC("isDir"), entry->d_type == DT_DIR ? "true" : "false",
            MG_ESC("size"), (long long)st.st_size);

         json += buf;
         free(buf);

         i++;
      }
   }

   json += " ]";

   closedir(dir);

   mg_http_reply(c, 200, "", "%s", json.c_str());
}

void WebServer::Download(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   PLOGI.printf("Downloading file: q=%s", q);

   if (*q == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_szMyPrefPath + q;

   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, path.c_str(), &opts);
}

void WebServer::Upload(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   PLOGI.printf("Uploading file: q=%s", q);

   if (*q == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_szMyPrefPath + q;

   if (!mg_http_upload(c, hm, &mg_fs_posix, path.c_str(), 1024 * 1024 * 500)) {
      if (!strncmp(q, "VPinballX.ini", sizeof(q)))
         InitRegistry(path);
   }
}

void WebServer::Delete(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   if (*q == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_szMyPrefPath + q;

   if (std::filesystem::is_regular_file(path)) {
      if (std::filesystem::remove(path.c_str()))
         mg_http_reply(c, 200, "", "OK");
      else
         mg_http_reply(c, 500, "", "Server error");
   }
   else if (std::filesystem::is_directory(path)) {
      if (std::filesystem::remove_all(path) != 0)
         mg_http_reply(c, 200, "", "OK");
      else
         mg_http_reply(c, 500, "", "Server error");
   }
   else
      mg_http_reply(c, 400, "", "Bad request");
}

void WebServer::Folder(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   if (*q == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_szMyPrefPath + q;

   if (std::filesystem::create_directory(path))
      mg_http_reply(c, 200, "", "OK");
   else
      mg_http_reply(c, 500, "", "Server error");
}

void WebServer::Activate(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   mg_remove_double_dots(q);

   PLOGI.printf("Activating table: q=%s", q);

   if (*q == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   SaveValue(regKey[RegName::Standalone], "LaunchTable"s, q);

   mg_http_reply(c, 200, "", "OK");
}

void WebServer::Command(struct mg_connection *c, struct mg_http_message* hm)
{
   char cmd[1024];
   mg_http_get_var(&hm->query, "cmd", cmd, sizeof(cmd));

   if (*cmd == '\0') {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   if (!strncmp(cmd, "fps", sizeof(cmd))) {
      g_pplayer->m_liveUI->ToggleFPS();

      mg_http_reply(c, 200, "", "OK");
   }
   else if (!strncmp(cmd, "shutdown", sizeof(cmd))) {
      g_pvp->GetActiveTable()->QuitPlayer(Player::CS_CLOSE_APP);

      mg_http_reply(c, 200, "", "OK");
   }
   else
      mg_http_reply(c, 400, "", "Bad request");
}

string WebServer::GetUrl()
{
   return m_run ? m_url : "";
}

void WebServer::Start()
{
   if (m_run) {
      PLOGE.printf("Web server already running");
      return;
   }

   const bool debug = LoadValueWithDefault(regKey[RegName::Standalone], "WebServerDebug"s, false);

   if (debug) {
      mg_log_set(MG_LL_DEBUG);
      PLOGI.printf("Web server debug enabled");
   }

   string bindUrl = "http://" + m_addr + ":" + std::to_string(m_port);

   PLOGI.printf("Starting web server at %s", bindUrl.c_str());

   mg_mgr_init(&m_mgr);

   if (mg_http_listen(&m_mgr, bindUrl.c_str(), &WebServer::EventHandler, this)) {
      m_run = true;

      PLOGI.printf("Web server started");

      string ip = GetIPAddress();

      if (!ip.empty()) {
         m_url = "http://" + ip + ":" + std::to_string(m_port);

         PLOGI.printf("To access the web server, in a browser go to: %s", m_url.c_str());
      }
      else
         m_url = "";

      m_pThread = new std::thread([this]() {
         while (m_run)
            mg_mgr_poll(&m_mgr, 1000);

         mg_mgr_free(&m_mgr);
         m_url = "";

         PLOGI.printf("Web server closed");
      });
   }
   else {
      PLOGE.printf("Unable to start web server");
   }
}

void WebServer::Stop()
{
   if (!m_run) {
      PLOGE.printf("Web server is not running");
      return;
   }

   m_run = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;

      m_pThread = NULL;
   }
}

bool WebServer::IsRunning()
{
   return m_run;
}

string WebServer::GetIPAddress()
{
   struct ifaddrs *ifaddr;
   struct ifaddrs *ifa;
   int family;
   int s;
   char host[NI_MAXHOST];
   string ip;

   if (getifaddrs(&ifaddr) == -1)
      return "";

   for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == NULL)
         continue;

      family = ifa->ifa_addr->sa_family;

      if (family == AF_INET) {
         if (strcmp(ifa->ifa_name, "wlan0") == 0 || strcmp(ifa->ifa_name, "eth0") == 0 || strcmp(ifa->ifa_name, "en0") == 0) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
               return "";
            ip = host;
            break;
         }
      }
   }

   freeifaddrs(ifaddr);

   return ip;
}