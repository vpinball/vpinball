#include "core/stdafx.h"
#include "core/vpversion.h"
#include "WebServer.h"

#include <zip.h>

#include <ifaddrs.h>
#include <filesystem>

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

void WebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data)
{
   WebServer* webServer = (WebServer*)c->fn_data;

   if (ev == MG_EV_HTTP_MSG) {
      struct mg_http_message *hm = (struct mg_http_message *) ev_data;

      if (mg_match(hm->uri, mg_str("/status"), NULL))
         webServer->Status(c, hm);
      else if (mg_match(hm->uri, mg_str("/files"), NULL))
         webServer->Files(c, hm);
      else if (mg_match(hm->uri, mg_str("/download"), NULL))
         webServer->Download(c, hm);
      else if (mg_match(hm->uri, mg_str("/upload"), NULL))
         webServer->Upload(c, hm);
      else if (mg_match(hm->uri, mg_str("/delete"), NULL))
         webServer->Delete(c, hm);
      else if (mg_match(hm->uri, mg_str("/folder"), NULL))
         webServer->Folder(c, hm);
      else if (mg_match(hm->uri, mg_str("/extract"), NULL))
         webServer->Extract(c, hm);
      else if (mg_match(hm->uri, mg_str("/command"), NULL))
         webServer->Command(c, hm);
      else {
         struct mg_http_serve_opts opts = {};

         string uri(hm->uri.buf, hm->uri.len);
         if (!uri.empty() && uri.front() == '/') uri.erase(0, 1);

         std::filesystem::path base = std::filesystem::path(g_pvp->m_myPath) / "assets";
         std::filesystem::path asset = uri.empty() ? base / "vpx.html" : base / uri;

         if (!uri.empty() && std::filesystem::exists(asset))
            mg_http_serve_file(c, hm, asset.string().c_str(), &opts);
         else
            mg_http_serve_file(c, hm, (base / "vpx.html").string().c_str(), &opts);
      }
   }
}

WebServer::WebServer() 
{
   m_run = false;
   m_pThread = nullptr;
}

WebServer::~WebServer() 
{
   m_run = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }
}

bool WebServer::Unzip(const char* pSource)
{
   int error = 0;
   zip_t* zip_archive = zip_open(pSource, ZIP_RDONLY, &error);
   if (!zip_archive) {
      PLOGE.printf("Unable to unzip file: source=%s", pSource);
      return false;
   }

   bool success = true;
   zip_int64_t file_count = zip_get_num_entries(zip_archive, 0);

   for (zip_uint64_t i = 0; i < (zip_uint64_t)file_count; ++i) {
      zip_stat_t file_stat;
      if (zip_stat_index(zip_archive, i, ZIP_STAT_NAME, &file_stat) != 0) {
         success = false;
         continue;
      }

      string filename = file_stat.name;
      if (filename.rfind("__MACOSX", 0) == 0)
         continue;

      std::filesystem::path path = std::filesystem::path(pSource).parent_path() / filename;
      if (filename.back() == '/')
         std::filesystem::create_directories(path);
      else {
         std::filesystem::create_directories(path.parent_path());
         zip_file_t* zip_file = zip_fopen_index(zip_archive, i, 0);
         if (!zip_file) {
             PLOGE.printf("Unable to extract file: %s", path.string().c_str());
             success = false;
             continue;
         }
         std::ofstream ofs(path, std::ios::binary);
         char buf[4096];
         zip_int64_t len;
         while ((len = zip_fread(zip_file, buf, sizeof(buf))) > 0)
             ofs.write(buf, len);
         zip_fclose(zip_file);
      }
   }

   zip_close(zip_archive);
   return success;
}

void WebServer::Status(struct mg_connection *c, struct mg_http_message* hm)
{
   bool running = g_pplayer != nullptr;
   string currentTable = (running && !g_pvp->m_currentTablePath.empty())
      ? (std::filesystem::path(g_pvp->m_currentTablePath) / g_pvp->GetActiveTable()->m_filename).string()
      : "";
   char* currentTableJson = (running && !currentTable.empty())
      ? mg_mprintf("\"%s\"", currentTable.c_str())
      : mg_mprintf("null");
   char* json = mg_mprintf(
      "{\"version\":\"%s\",\"running\":%s,\"currentTable\":%s}",
      VP_VERSION_STRING_FULL_LITERAL,
      running ? "true" : "false",
      currentTableJson
   );
   free(currentTableJson);
   mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json);
   free(json);
}

void WebServer::Files(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (!mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   PLOGI.printf("Retrieving file list: q=%s", q);

   string path = g_pvp->m_myPrefPath + q;

   if (*q != '\0')
      path += PATH_SEPARATOR_CHAR;

   DIR* dir = opendir(path.c_str());

   if (!dir) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string json = "[ "s;

   int i = 0;

   struct dirent *entry;

   while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
         continue;

      if (i > 0)
         json += ", ";

      string file = path + entry->d_name;
      string ext;
      if (entry->d_type != DT_DIR) ext = extension_from_path(file);

      struct stat st;
      if (stat(file.c_str(), &st) == 0) {
         char datebuf[32];
         struct tm tm;
         gmtime_r(&st.st_mtime, &tm);
         strftime(datebuf, sizeof(datebuf), "%Y-%m-%dT%H:%M:%SZ", &tm);

         char* buf = mg_mprintf(
            "{ %m: %m, %m: %m, %m: %s, %m: %lld, %m: %m }",
            MG_ESC("name"), MG_ESC(entry->d_name),
            MG_ESC("ext"), MG_ESC(ext.c_str()),
            MG_ESC("isDir"), entry->d_type == DT_DIR ? "true" : "false",
            MG_ESC("size"), (long long)st.st_size,
            MG_ESC("date"), MG_ESC(datebuf)
         );

         json += buf;
         free(buf);
         i++;
      }
   }

   json += " ]";

   closedir(dir);

   mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json.c_str());
}

void WebServer::Download(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   PLOGI.printf("Downloading file: q=%s", q);

   string path = g_pvp->m_myPrefPath + q;

   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, path.c_str(), &opts);
}

void WebServer::Upload(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   if (*q != '\0' && !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   char file[1024];
   mg_http_get_var(&hm->query, "file", file, sizeof(file));
   if (*file == '\0' || !mg_path_is_sane(mg_str(file))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   char offsetStr[32];
   mg_http_get_var(&hm->query, "offset", offsetStr, sizeof(offsetStr));
   long offset = offsetStr[0] ? strtol(offsetStr, nullptr, 10) : 0;
   if (offset <= 0) {
      PLOGI.printf("Uploading file: file=%s", file);
   }

   char lengthStr[32];
   mg_http_get_var(&hm->query, "length", lengthStr, sizeof(lengthStr));
   long length = lengthStr[0] ? strtol(lengthStr, nullptr, 10) : 0;

   string path = g_pvp->m_myPrefPath + q;

   if (mg_http_upload(c, hm, &mg_fs_posix, path.c_str(), 1024 * 1024 * 500) == length) {
      if (*q == '\0' && !strcmp(file, "VPinballX.ini")) {
         g_pvp->m_settings.LoadFromFile(path, true);
         g_pvp->m_settings.Save();
      }
   }
}

void WebServer::Delete(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_myPrefPath + q;

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

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_myPrefPath + q;

   std::error_code ec;
   if (std::filesystem::create_directory(path, ec))
      mg_http_reply(c, 200, "", "OK");
   else
      mg_http_reply(c, 500, "", "Server error");
}

void WebServer::Extract(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, 400, "", "Bad request");
      return;
   }

   string path = g_pvp->m_myPrefPath + q;

   if (std::filesystem::is_regular_file(path)) {
      if (extension_from_path(path) == "zip") {
         if (Unzip(path.c_str())) {
            PLOGI.printf("File unzipped: q=%s", path.c_str());
            mg_http_reply(c, 200, "", "OK");
         }
         else
            mg_http_reply(c, 500, "", "Server error");
      }
      else
         mg_http_reply(c, 400, "", "Bad request");
   }
   else
      mg_http_reply(c, 400, "", "Bad request");
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
   return m_run ? m_url : string();
}

void WebServer::Start()
{
   if (m_run) {
      PLOGE.printf("Web server already running");
      return;
   }

   if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServerDebug"s, false)) {
      mg_log_set(MG_LL_DEBUG);
      PLOGI.printf("Web server debug enabled");
   }

   const string addr = g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServerAddr"s, "0.0.0.0"s);
   const int port = g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServerPort"s, 2112);

   string bindUrl = "http://" + addr + ':' + std::to_string(port);

   PLOGI.printf("Starting web server at %s", bindUrl.c_str());

   mg_mgr_init(&m_mgr);

   if (mg_http_listen(&m_mgr, bindUrl.c_str(), &WebServer::EventHandler, this)) {
      m_run = true;

      PLOGI.printf("Web server started");

      string ip = GetIPAddress();

      if (!ip.empty()) {
         m_url = "http://" + ip + ':' + std::to_string(port);

         PLOGI.printf("To access the web server, in a browser go to: %s", m_url.c_str());
      }
      else
         m_url.clear();

#ifdef __LIBVPINBALL__
      VPinballLib::WebServerData webServerData = { m_url.c_str() };
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, &webServerData);
#endif

      m_pThread = new std::thread([this]() {
         while (m_run)
            mg_mgr_poll(&m_mgr, 1000);

         mg_mgr_free(&m_mgr);
         m_url.clear();

         PLOGI.printf("Web server closed");

#ifdef __LIBVPINBALL__
         VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, nullptr);
#endif
      });
   }
   else {
      PLOGE.printf("Unable to start web server");

#ifdef __LIBVPINBALL__
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, nullptr);
#endif
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

      m_pThread = nullptr;
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

   if (getifaddrs(&ifaddr) == -1)
      return string();

   for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == nullptr)
         continue;

      int family = ifa->ifa_addr->sa_family;

      if (family == AF_INET) {
         if (!strcmp(ifa->ifa_name, "wlan0") || !strcmp(ifa->ifa_name, "eth0") || !strcmp(ifa->ifa_name, "en0") || !strcmp(ifa->ifa_name, "en1")) {
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            freeifaddrs(ifaddr);
            if (s != 0)
               return string();
            else
               return host;
         }
      }
   }

   freeifaddrs(ifaddr);

   return string();
}
