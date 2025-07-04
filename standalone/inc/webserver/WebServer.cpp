#include "core/stdafx.h"
#include "core/vpversion.h"
#include "WebServer.h"

#include "standalone/VPinballLib.h"

#include <zip.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ifaddrs.h>
#include <filesystem>
#include <map>
#include <algorithm>

namespace {
   constexpr const char* HEADER_JSON = "Content-Type: application/json\r\n";

   constexpr const char* RESPONSE_OK = "OK";
   constexpr const char* RESPONSE_BAD_REQUEST = "Bad request";
   constexpr const char* RESPONSE_NOT_FOUND = "File not found";
   constexpr const char* RESPONSE_METHOD_NOT_ALLOWED = "Method Not Allowed";
   constexpr const char* RESPONSE_CONFLICT = "Conflict";
   constexpr const char* RESPONSE_INTERNAL_SERVER_ERROR = "Server error";

   constexpr int STATUS_OK = 200;
   constexpr int STATUS_BAD_REQUEST = 400;
   constexpr int STATUS_NOT_FOUND = 404;
   constexpr int STATUS_METHOD_NOT_ALLOWED = 405;
   constexpr int STATUS_CONFLICT = 409;
   constexpr int STATUS_INTERNAL_SERVER_ERROR = 500;

   constexpr const char* TEMP_DIR_NAME = "temp";
}

std::mutex WebServer::s_logMutex;
std::vector<struct mg_connection*> WebServer::s_logConnections;
std::vector<struct mg_connection*> WebServer::s_statusConnections;
std::deque<std::string> WebServer::s_recentLogs;
WebServer* WebServer::s_instance = nullptr;
int64_t WebServer::s_lastUpdateTimestamp = 0;

void WebServer::EventHandler(struct mg_connection *c, int ev, void *ev_data)
{
   WebServer* webServer = (WebServer*)c->fn_data;

   if (ev == MG_EV_HTTP_MSG) {
      struct mg_http_message *hm = (struct mg_http_message *) ev_data;

      if (mg_match(hm->uri, mg_str("/info"), NULL))
         webServer->Info(c, hm);
      else if (mg_match(hm->uri, mg_str("/status"), NULL))
         webServer->Status(c, hm);
      else if (mg_match(hm->uri, mg_str("/assets/*"), NULL))
         webServer->Assets(c, hm);
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
      else if (mg_match(hm->uri, mg_str("/log-stream"), NULL))
         webServer->LogStream(c, hm);
      else if (mg_match(hm->uri, mg_str("/rename"), NULL))
         webServer->Rename(c, hm);
      else if (mg_match(hm->uri, mg_str("/table-list"), NULL))
         webServer->TableList(c, hm);
      else if (mg_match(hm->uri, mg_str("/table-import"), NULL))
         webServer->TableImport(c, hm);
      else if (mg_match(hm->uri, mg_str("/table-export"), NULL))
         webServer->TableExport(c, hm);
      else if (mg_match(hm->uri, mg_str("/table-rename"), NULL))
         webServer->TableRename(c, hm);
      else {
         struct mg_http_serve_opts opts = {};

         string uri(hm->uri.buf, hm->uri.len);
         if (!uri.empty() && uri.front() == '/') uri.erase(0, 1);

         std::filesystem::path webBase = std::filesystem::path(g_pvp->m_myPath) / "assets" / "web";
         std::filesystem::path asset = uri.empty() ? webBase / "vpx.html" : webBase / uri;

         if (!uri.empty() && std::filesystem::exists(asset))
            mg_http_serve_file(c, hm, asset.string().c_str(), &opts);
         else
            mg_http_serve_file(c, hm, (webBase / "vpx.html").string().c_str(), &opts);
      }
   }
   else if (ev == MG_EV_WS_OPEN) {
   }
   else if (ev == MG_EV_WS_MSG) {
   }
   else if (ev == MG_EV_CLOSE) {
      std::lock_guard<std::mutex> lock(s_logMutex);
      auto logIt = std::find(s_logConnections.begin(), s_logConnections.end(), c);
      if (logIt != s_logConnections.end())
         s_logConnections.erase(logIt);

      auto statusIt = std::find(s_statusConnections.begin(), s_statusConnections.end(), c);
      if (statusIt != s_statusConnections.end())
         s_statusConnections.erase(statusIt);
   }
}

void WebServer::LogAppender(const std::string& formattedLog)
{
   if (!s_instance) {
      static std::queue<std::string> pendingLogs;
      static std::mutex pendingMutex;
      std::lock_guard<std::mutex> lock(pendingMutex);
      pendingLogs.push(formattedLog);

      while (pendingLogs.size() > 100)
         pendingLogs.pop();

      return;
   }

   static std::queue<std::string> pendingLogs;
   static std::mutex pendingMutex;
   static bool processingPending = false;

   s_instance->AddLogEntry(formattedLog);

   if (!processingPending) {
      std::lock_guard<std::mutex> lock(pendingMutex);
      processingPending = true;
      while (!pendingLogs.empty()) {
         s_instance->AddLogEntry(pendingLogs.front());
         pendingLogs.pop();
      }
      processingPending = false;
   }
}

void WebServer::SetLastUpdate()
{
   s_lastUpdateTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
   ).count();
   PLOGD.printf("Web interface last update timestamp set to: %lld", (long long)s_lastUpdateTimestamp);

   BroadcastStatus();
}

WebServer::WebServer() 
{
   m_run = false;
   m_pThread = nullptr;
   s_instance = this;
}

WebServer::~WebServer() 
{
   m_run = false;
   s_instance = nullptr;

   {
      std::lock_guard<std::mutex> lock(s_logMutex);
      s_logConnections.clear();
      s_statusConnections.clear();
      s_recentLogs.clear();
   }

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }
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

   SetLastUpdate();

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

      VPinballLib::WebServerData webServerData = { m_url.c_str() };
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, &webServerData);

      m_pThread = new std::thread([this]() {
         while (m_run)
            mg_mgr_poll(&m_mgr, 1000);

         mg_mgr_free(&m_mgr);
         m_url.clear();

         PLOGI.printf("Web server closed");

         VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, nullptr);
      });
   }
   else {
      PLOGE.printf("Unable to start web server");

      VPinballLib::VPinball::SendEvent(VPinballLib::Event::WebServer, nullptr);
   }

   CleanupTempDirectory();
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

string WebServer::GetUrl()
{
   return m_run ? m_url : string();
}

void WebServer::Info(struct mg_connection *c, struct mg_http_message* hm)
{
   char* json = mg_mprintf(
      "{\"version\":\"%s\"}",
      VP_VERSION_STRING_FULL_LITERAL
   );
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", json);
   free(json);
}

void WebServer::Status(struct mg_connection *c, struct mg_http_message* hm)
{
   mg_ws_upgrade(c, hm, NULL);

   {
      std::lock_guard<std::mutex> lock(s_logMutex);
      s_statusConnections.push_back(c);
   }

   BroadcastStatus();
}

void WebServer::Assets(struct mg_connection *c, struct mg_http_message* hm)
{
   string uri(hm->uri.buf, hm->uri.len);

   if (uri.length() > 8 && uri.substr(0, 8) == "/assets/") {
      string assetPath = uri.substr(8);

      if (!mg_path_is_sane(mg_str(assetPath.c_str()))) {
         mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
         return;
      }

      std::filesystem::path fullPath = std::filesystem::path(g_pvp->m_myPath) / "assets" / assetPath;

      if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath)) {
         struct mg_http_serve_opts opts = {};
         mg_http_serve_file(c, hm, fullPath.string().c_str(), &opts);
      }
      else
         mg_http_reply(c, STATUS_NOT_FOUND, "", RESPONSE_NOT_FOUND);
   } 
   else
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
}

void WebServer::Files(struct mg_connection *c, struct mg_http_message* hm)
{
   char buffer[1024];
   mg_http_get_var(&hm->query, "q", buffer, sizeof(buffer));

   string q = buffer;
   if (!q.empty() && !mg_path_is_sane(mg_str(buffer))) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   PLOGD.printf("Retrieving file list: q=%s", q.c_str());

   string path = BuildUserPath(q.c_str());
   if (!q.empty())
      path += PATH_SEPARATOR_CHAR;

   DIR* dir = opendir(path.c_str());
   if (!dir) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   std::ostringstream json;
   json << "[ ";

   bool first = true;
   struct dirent *entry;

   while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
         continue;

      if (!first)
         json << ", ";

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

         json << buf;
         free(buf);
         first = false;
      }
   }

   json << " ]";
   closedir(dir);

   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", json.str().c_str());
}

void WebServer::Download(struct mg_connection *c, struct mg_http_message* hm)
{
   string q;
   if (!ValidatePathParameter(c, hm, "q", q))
      return;

   PLOGI.printf("Downloading file: q=%s", q.c_str());

   string path = g_pvp->m_myPrefPath + q;

   struct mg_http_serve_opts opts = {};
   mg_http_serve_file(c, hm, path.c_str(), &opts);
}

void WebServer::Upload(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));
   if (*q != '\0' && !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   string file;
   if (!ValidatePathParameter(c, hm, "file", file))
      return;

   char offsetStr[32];
   mg_http_get_var(&hm->query, "offset", offsetStr, sizeof(offsetStr));
   long offset = offsetStr[0] ? strtol(offsetStr, nullptr, 10) : 0;
   if (offset <= 0) {
      PLOGI.printf("Uploading file: file=%s", file.c_str());
   }

   char lengthStr[32];
   mg_http_get_var(&hm->query, "length", lengthStr, sizeof(lengthStr));
   long length = lengthStr[0] ? strtol(lengthStr, nullptr, 10) : 0;

   string path = BuildUserPath(q);

   if (mg_http_upload(c, hm, &mg_fs_posix, path.c_str(), 1024 * 1024 * 500) == length) {
      if (*q == '\0' && file == "VPinballX.ini") {
         g_pvp->m_settings.LoadFromFile(path, true);
         g_pvp->m_settings.Save();
      }
      SetLastUpdate();
   }
}

void WebServer::Delete(struct mg_connection *c, struct mg_http_message* hm)
{
   string q;
   if (!ValidatePathParameter(c, hm, "q", q))
      return;

   string path = BuildUserPath(q.c_str());

   if (std::filesystem::is_regular_file(path)) {
      if (std::filesystem::remove(path.c_str())) {
         SetLastUpdate();
         mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
      } 
      else
         mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", RESPONSE_INTERNAL_SERVER_ERROR);
   }
   else if (std::filesystem::is_directory(path)) {
      std::filesystem::path dirPath(path);
      string dirName = dirPath.filename().string();

      bool isTableFolder = false;

      string parentStr = dirPath.parent_path().string();
      string prefStr = g_pvp->m_myPrefPath;
      if (!prefStr.empty() && prefStr.back() == '/')
         prefStr.pop_back();

      if (parentStr == prefStr) {
         VPinballLib::TablesData tablesData = {};
         tablesData.tables = nullptr;
         tablesData.tableCount = 0;
         tablesData.success = false;

         VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableList, &tablesData);

         if (tablesData.success && tablesData.tables && tablesData.tableCount > 0) {
            for (int i = 0; i < tablesData.tableCount; i++) {
               if (tablesData.tables[i].tableId && string(tablesData.tables[i].tableId) == dirName) {
                  isTableFolder = true;
                  break;
               }
            }
         }

         if (tablesData.tables) {
            for (int i = 0; i < tablesData.tableCount; i++) {
               if (tablesData.tables[i].tableId)
                  free(tablesData.tables[i].tableId);
               if (tablesData.tables[i].name)
                  free(tablesData.tables[i].name);
            }
            free(tablesData.tables);
         }
      }

      if (isTableFolder) {
         VPinballLib::TableEventData eventData = {};
         eventData.tableId = dirName.c_str();
         eventData.newName = nullptr;
         eventData.path = nullptr;
         eventData.success = false;

         VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableDelete, &eventData);

         if (eventData.success) {
            SetLastUpdate();
            mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
         } 
         else
            mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", "Failed to delete table");
      }
      else {
         if (std::filesystem::remove_all(path) != 0) {
            SetLastUpdate();
            mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
         } 
         else
            mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", RESPONSE_INTERNAL_SERVER_ERROR);
      }
   }
   else
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
}

void WebServer::Rename(struct mg_connection *c, struct mg_http_message* hm) 
{
   string q;
   if (!ValidatePathParameter(c, hm, "q", q))
      return;

   string newName;
   if (!ValidatePathParameter(c, hm, "name", newName))
      return;

   string oldPath = BuildUserPath(q.c_str());
   std::filesystem::path oldFile(oldPath);

   if (!std::filesystem::exists(oldFile)) {
      mg_http_reply(c, STATUS_NOT_FOUND, "", RESPONSE_NOT_FOUND);
      return;
   }

   std::filesystem::path newFile = oldFile.parent_path() / newName;
   if (std::filesystem::exists(newFile)) {
      mg_http_reply(c, STATUS_CONFLICT, "", RESPONSE_CONFLICT);
      return;
   }

   std::error_code ec;
   std::filesystem::rename(oldFile, newFile, ec);
   if (ec)
      mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", RESPONSE_INTERNAL_SERVER_ERROR);
   else {
      SetLastUpdate();
      mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
   }
}

void WebServer::Folder(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   string path = g_pvp->m_myPrefPath + q;

   std::error_code ec;
   if (std::filesystem::create_directory(path, ec)) {
      SetLastUpdate();
      mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
   }
   else
      mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", RESPONSE_INTERNAL_SERVER_ERROR);
}

void WebServer::Extract(struct mg_connection *c, struct mg_http_message* hm)
{
   char q[1024];
   mg_http_get_var(&hm->query, "q", q, sizeof(q));

   if (*q == '\0' || !mg_path_is_sane(mg_str(q))) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   string path = g_pvp->m_myPrefPath + q;

   if (std::filesystem::is_regular_file(path)) {
      if (extension_from_path(path) == "zip") {
         if (Unzip(path.c_str())) {
            PLOGI.printf("File unzipped: q=%s", path.c_str());
            SetLastUpdate();
            mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
         }
         else
            mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", RESPONSE_INTERNAL_SERVER_ERROR);
      }
      else
         mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
   }
   else
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
}

void WebServer::Command(struct mg_connection *c, struct mg_http_message* hm)
{
   char cmd[1024];
   mg_http_get_var(&hm->query, "cmd", cmd, sizeof(cmd));

   if (*cmd == '\0') {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return;
   }

   if (!strncmp(cmd, "fps", sizeof(cmd))) {
      g_pplayer->m_liveUI->ToggleFPS();
      mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
   }
   else if (!strncmp(cmd, "shutdown", sizeof(cmd))) {
      g_pvp->GetActiveTable()->QuitPlayer(Player::CS_CLOSE_APP);
      mg_http_reply(c, STATUS_OK, "", RESPONSE_OK);
   }
   else if (!strncmp(cmd, "cls", sizeof(cmd))) {
      {
         std::lock_guard<std::mutex> lock(s_logMutex);
         s_recentLogs.clear();
      }
      char* json = mg_mprintf("{%m:%m,%m:%m}", 
         MG_ESC("status"), MG_ESC("success"),
         MG_ESC("message"), MG_ESC("Logs cleared"));
      mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", json);
      free(json);
   }
   else
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
}

void WebServer::LogStream(struct mg_connection *c, struct mg_http_message* hm)
{
   mg_printf(c, "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/event-stream\r\n"
              "Cache-Control: no-cache\r\n"
              "Connection: keep-alive\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "\r\n");

   c->is_resp = 0;

   {
      std::lock_guard<std::mutex> lock(s_logMutex);  
      s_logConnections.push_back(c);

      for (const auto& logLine : s_recentLogs) {
         std::string data = "data: " + logLine + "\n\n";
         mg_send(c, data.c_str(), data.length());
      }
   }
}

void WebServer::TableList(struct mg_connection *c, struct mg_http_message* hm)
{
   VPinballLib::TablesData tablesData = {};
   tablesData.tables = nullptr;
   tablesData.tableCount = 0;
   tablesData.success = false;

   VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableList, &tablesData);

   if (tablesData.success && tablesData.tables && tablesData.tableCount > 0) {
      std::ostringstream json;
      json << "[";
      for (int i = 0; i < tablesData.tableCount; i++) {
         if (i > 0)
            json << ",";

         const char* tableId = tablesData.tables[i].tableId ? tablesData.tables[i].tableId : "";
         const char* tableName = tablesData.tables[i].name ? tablesData.tables[i].name : "";

         char* tableEntry = mg_mprintf("{%m:%m,%m:%m}", 
            MG_ESC("table"), MG_ESC(tableId),
            MG_ESC("name"), MG_ESC(tableName));
         json << tableEntry;
         free(tableEntry);
      }
      json << "]";

      mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", json.str().c_str());
   }
   else {
      mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", "[]");
   }

   if (tablesData.tables) {
      for (int i = 0; i < tablesData.tableCount; i++) {
         if (tablesData.tables[i].tableId)
            free(tablesData.tables[i].tableId);
         if (tablesData.tables[i].name)
            free(tablesData.tables[i].name);
      }
      free(tablesData.tables);
   }
}

void WebServer::TableImport(struct mg_connection *c, struct mg_http_message* hm)
{
   string file;
   if (!ValidatePathParameter(c, hm, "file", file))
      return;

   char lengthStr[32];
   mg_http_get_var(&hm->query, "length", lengthStr, sizeof(lengthStr));
   long length = lengthStr[0] ? strtol(lengthStr, nullptr, 10) : 0;

   std::filesystem::path tempDir = GetTempDirectory();

   if (mg_http_upload(c, hm, &mg_fs_posix, tempDir.c_str(), 1024 * 1024 * 500) == length) {
      std::filesystem::path fullPath = tempDir / file;

      if (std::filesystem::exists(fullPath)) {
         VPinballLib::TableEventData eventData = {};
         eventData.tableId = nullptr;
         eventData.newName = nullptr;
         eventData.path = fullPath.c_str();
         eventData.success = false;

         VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableImport, &eventData);

         SetLastUpdate();
      }
   }
}

void WebServer::TableExport(struct mg_connection *c, struct mg_http_message* hm)
{
   string table;
   if (!ValidatePathParameter(c, hm, "table", table))
      return;

   string tablePath = BuildUserPath(table.c_str());

   if (!std::filesystem::exists(tablePath) || !std::filesystem::is_directory(tablePath)) {
      mg_http_reply(c, STATUS_NOT_FOUND, "", RESPONSE_NOT_FOUND);
      return;
   }

   string actualTableName = LookupTableName(table);

   std::filesystem::path tempDir = GetTempDirectory();

   string tableName = actualTableName;
   std::replace(tableName.begin(), tableName.end(), ' ', '_');

   string exportFileName = tableName + ".vpxz";
   std::filesystem::path exportPath = tempDir / exportFileName;

   if (std::filesystem::exists(exportPath)) {
      std::error_code ec;
      std::filesystem::remove(exportPath, ec);
      if (ec) {
         PLOGE.printf("Failed to remove existing export file: %s", ec.message().c_str());
         mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", "Failed to remove existing export file");
         return;
      }
   }

   if (VPinballLib::VPinball::GetInstance().Compress(tablePath, exportPath.string()) != VPinballLib::VPinballStatus::Success) {
      mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", "Failed to create export file");
      return;
   }

   struct mg_http_serve_opts opts = {};
   string downloadFileName = tableName + ".vpxz";
   string headers = "Content-Disposition: attachment; filename=\"" + downloadFileName + "\"\r\n";
   opts.extra_headers = headers.c_str();

   mg_http_serve_file(c, hm, exportPath.string().c_str(), &opts);

   PLOGI.printf("Exporting %s...", actualTableName.c_str());
}

void WebServer::TableRename(struct mg_connection *c, struct mg_http_message* hm)
{
   string table;
   if (!ValidatePathParameter(c, hm, "table", table))
      return;

   string newName;
   if (!ValidatePathParameter(c, hm, "name", newName))
      return;

   VPinballLib::TableEventData eventData = {};
   eventData.tableId = table.c_str();
   eventData.newName = newName.c_str();
   eventData.path = nullptr;
   eventData.success = false;

   VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableRename, &eventData);

   if (eventData.success) {
      char* json = mg_mprintf("{%m:%s}", MG_ESC("success"), "true");
      mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", json);
      free(json);
      SetLastUpdate();
      PLOGI.printf("Table renamed successfully: %s -> %s", table.c_str(), newName.c_str());
   } 
   else {
      mg_http_reply(c, STATUS_INTERNAL_SERVER_ERROR, "", "%s", "Failed to rename table");
      PLOGI.printf("Table rename failed: %s -> %s", table.c_str(), newName.c_str());
   }
}

void WebServer::AddLogEntry(const std::string& formattedLog)
{
   std::lock_guard<std::mutex> lock(s_logMutex);

   if (!s_recentLogs.empty() && s_recentLogs.back() == formattedLog)
      return;

   s_recentLogs.push_back(formattedLog);

   while (s_recentLogs.size() > MAX_RECENT_LOGS)
      s_recentLogs.pop_front();

   BroadcastLogEntry(formattedLog);
}

void WebServer::BroadcastLogEntry(const std::string& formattedLog)
{
   if (s_logConnections.empty())
      return;

   std::string data = "data: " + formattedLog + "\n\n";

   auto it = s_logConnections.begin();
   while (it != s_logConnections.end()) {
      struct mg_connection* conn = *it;
      if (conn && mg_send(conn, data.c_str(), data.length()) == 0)
         it = s_logConnections.erase(it);
      else
         ++it;
   }
}

void WebServer::BroadcastStatus()
{
   if (s_statusConnections.empty()) return;

   bool running = g_pplayer != nullptr;
   string currentTable = "";
   if (running && !g_pvp->m_currentTablePath.empty())
      currentTable = g_pvp->GetActiveTable()->m_filename;

   char* json = nullptr;
   if (!currentTable.empty()) {
      json = mg_mprintf(
         "{%m:%s,%m:%m,%m:%lld}",
         MG_ESC("running"), running ? "true" : "false",
         MG_ESC("currentTable"), MG_ESC(currentTable.c_str()),
         MG_ESC("lastUpdate"), (long long)s_lastUpdateTimestamp
      );
   } else {
      json = mg_mprintf(
         "{%m:%s,%m:%s,%m:%lld}",
         MG_ESC("running"), running ? "true" : "false",
         MG_ESC("currentTable"), "null",
         MG_ESC("lastUpdate"), (long long)s_lastUpdateTimestamp
      );
   }

   std::lock_guard<std::mutex> lock(s_logMutex);
   auto it = s_statusConnections.begin();
   while (it != s_statusConnections.end()) {
      struct mg_connection* conn = *it;
      if (conn && mg_ws_send(conn, json, strlen(json), WEBSOCKET_OP_TEXT) == 0)
         it = s_statusConnections.erase(it);
      else
         ++it;
   }

   free(json);
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

bool WebServer::ValidatePathParameter(struct mg_connection *c, struct mg_http_message* hm, const char* paramName, string& outValue)
{
   char buffer[1024];
   mg_http_get_var(&hm->query, paramName, buffer, sizeof(buffer));

   if (*buffer == '\0' || !mg_path_is_sane(mg_str(buffer))) {
      mg_http_reply(c, STATUS_BAD_REQUEST, "", "%s", RESPONSE_BAD_REQUEST);
      return false;
   }

   outValue = buffer;
   return true;
}

std::filesystem::path WebServer::BuildUserPath(const char* relativePath)
{
   return std::filesystem::path(g_pvp->m_myPrefPath) / relativePath;
}

std::filesystem::path WebServer::GetTempDirectory()
{
   std::filesystem::path tempDir = std::filesystem::path(g_pvp->m_myPrefPath) / TEMP_DIR_NAME;

   std::error_code ec;
   std::filesystem::create_directories(tempDir, ec);
   if (ec) {
      PLOGE.printf("Failed to create temp directory: %s", ec.message().c_str());
   }

   return tempDir;
}

void WebServer::CleanupTempDirectory()
{
   std::filesystem::path tempDir = std::filesystem::path(g_pvp->m_myPrefPath) / TEMP_DIR_NAME;

   if (!std::filesystem::exists(tempDir))
      return;

   try {
      auto now = std::filesystem::file_time_type::clock::now();
      auto cutoff = now - std::chrono::hours(1);

      for (const auto& entry : std::filesystem::directory_iterator(tempDir)) {
         if (entry.is_regular_file()) {
            auto lastWrite = entry.last_write_time();
            if (lastWrite < cutoff) {
               std::error_code ec;
               std::filesystem::remove(entry.path(), ec);
               if (!ec) {
                  PLOGI.printf("Cleaned up old temp file: %s", entry.path().filename().string().c_str());
               }
            }
         }
      }
   }
   catch (const std::exception& e) {
      PLOGE.printf("Error during temp directory cleanup: %s", e.what());
   }
}

string WebServer::LookupTableName(const string& tableId)
{
   VPinballLib::TablesData tablesData = {};
   tablesData.tables = nullptr;
   tablesData.tableCount = 0;
   tablesData.success = false;

   VPinballLib::VPinball::SendEvent(VPinballLib::Event::TableList, &tablesData);

   string actualTableName = tableId;
   if (tablesData.success && tablesData.tables && tablesData.tableCount > 0) {
      for (int i = 0; i < tablesData.tableCount; i++) {
         if (tablesData.tables[i].tableId && string(tablesData.tables[i].tableId) == tableId) {
            if (tablesData.tables[i].name)
               actualTableName = string(tablesData.tables[i].name);
            break;
         }
      }

      for (int i = 0; i < tablesData.tableCount; i++) {
         if (tablesData.tables[i].tableId)
            free(tablesData.tables[i].tableId);
         if (tablesData.tables[i].name)
            free(tablesData.tables[i].name);
      }
      if (tablesData.tables)
         free(tablesData.tables);
   }

   return actualTableName;
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