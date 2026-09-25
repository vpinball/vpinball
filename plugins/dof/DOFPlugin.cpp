// license:GPLv3+

#include "plugins/VPXPlugin.h"
#include "plugins/B2SPluginEventStream.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DOF/DOF.h"
#pragma warning(pop)

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <locale>
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Direct Output Framework plugin
//

namespace DOFPlugin {

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;

static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>> controllers;
static std::unique_ptr<DOF::DOF> pDOF;

LPI_USE_CPP();
#define LOGD DOFPlugin::LPI_LOGD_CPP
#define LOGI DOFPlugin::LPI_LOGI_CPP
#define LOGW DOFPlugin::LPI_LOGW_CPP
#define LOGE DOFPlugin::LPI_LOGE_CPP

LPI_IMPLEMENT_CPP // Implement shared log support

#ifdef _WIN32
   static void SetThreadName(const std::string& name)
   {
      const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
      if (size_needed <= 1)
         return;
      std::wstring wstr(size_needed - 1, L'\0');
      if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
         return;
      HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
   }
#else
   static void SetThreadName(const std::string& name)
   {
#ifdef __APPLE__
      pthread_setname_np(name.c_str());
#elif defined(__linux__) || defined(__ANDROID__)
      pthread_setname_np(pthread_self(), name.c_str());
#endif
   }
#endif

void LIBDOFCALLBACK OnDOFLog(DOF_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      string buffer(size + 1, '\0');
      vsnprintf(buffer.data(), size + 1, format, args);
      buffer.pop_back(); // remove null terminator
      switch(logLevel) {
         case DOF_LogLevel_INFO:
            LOGI(buffer);
            break;
         case DOF_LogLevel_DEBUG:
            LOGD(buffer);
            break;
         case DOF_LogLevel_ERROR:
            LOGE(buffer);
            break;
         default:
            break;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// DOF rom list
//
// libDOF resolves the rom name given to Init against the table configuration
// lines of the ledcontrol ini files it loads (first CSV column of the
// [Config DOF] / [Config outs] section, see LedControlConfigList). Its public
// API does not expose that list, so the plugin parses the same files to pick
// the rom name handed to Init: ns::rom resolves to "ns_rom" when that name is
// declared, "rom" otherwise. Kept self-contained so a future libDOF query API
// can replace it whole.

static std::string DofToUpper(const std::string_view& s)
{
   std::string r(s);
   std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
   return r;
}

static std::string_view TrimSv(const std::string_view& s)
{
   const auto isSpace = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
   const size_t begin = s.find_first_not_of(" \t\r\n");
   if (begin == std::string_view::npos)
      return {};
   const size_t end = s.find_last_not_of(" \t\r\n");
   return s.substr(begin, end - begin + 1);
}

// Collect the short rom names declared by the ledcontrol ini files libDOF
// would load for this table: directoutputconfig*.ini, ledcontrol*.ini
// otherwise, looked up in IniFilesPath, the table folder, the global config
// folder and the current directory, the first folder with any match winning
// (mirrors DOF.cpp / GlobalConfig::GetIniFilesDictionary).
static std::unordered_set<std::string> LoadDofRomList(const std::filesystem::path& tablePath)
{
   std::filesystem::path globalConfigPath = std::filesystem::path(DOF::Config::GetInstance()->GetBasePath()) / "directoutputconfig" / "GlobalConfig_B2SServer.xml";
   if (!std::filesystem::exists(globalConfigPath))
      globalConfigPath = std::filesystem::path("directoutputconfig") / "GlobalConfig_B2SServer.xml";

   // IniFilesPath is the only global config setting that changes file lookup
   std::string iniFilesPath;
   {
      std::ifstream xml(globalConfigPath);
      std::stringstream content;
      content << xml.rdbuf();
      const std::string str = content.str();
      constexpr std::string_view openTag = "<IniFilesPath>"sv;
      constexpr std::string_view closeTag = "</IniFilesPath>"sv;
      if (const size_t open = str.find(openTag); open != std::string::npos)
         if (const size_t close = str.find(closeTag, open + openTag.size()); close != std::string::npos)
            iniFilesPath = std::string(TrimSv(std::string_view(str).substr(open + openTag.size(), close - open - openTag.size())));
   }

   std::vector<std::filesystem::path> lookupPaths;
   if (!iniFilesPath.empty() && std::filesystem::is_directory(iniFilesPath))
      lookupPaths.push_back(iniFilesPath);
   if (tablePath.has_parent_path())
      lookupPaths.push_back(tablePath.parent_path());
   if (globalConfigPath.has_parent_path())
      lookupPaths.push_back(globalConfigPath.parent_path());
   lookupPaths.push_back(std::filesystem::current_path());

   std::vector<std::filesystem::path> iniFiles;
   for (const std::string_view prefix : { "directoutputconfig"sv, "ledcontrol"sv })
   {
      const std::string upperPrefix = DofToUpper(prefix);
      for (const std::filesystem::path& dir : lookupPaths)
      {
         std::error_code ec;
         if (!std::filesystem::is_directory(dir, ec))
            continue;
         for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir, ec))
         {
            if (!entry.is_regular_file(ec))
               continue;
            const std::string name = DofToUpper(entry.path().filename().string());
            if (!name.starts_with(upperPrefix) || !name.ends_with(".INI"))
               continue;
            // Same filter as libDOF: unnumbered file, or numeric ledwiz suffix
            if (name != upperPrefix + ".INI"
               && !std::ranges::all_of(name.substr(upperPrefix.size(), name.size() - upperPrefix.size() - 4), [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }))
               continue;
            iniFiles.push_back(entry.path());
         }
         if (!iniFiles.empty())
            break;
      }
      if (!iniFiles.empty())
         break;
   }

   std::unordered_set<std::string> romList;
   for (const std::filesystem::path& iniFile : iniFiles)
   {
      std::ifstream file(iniFile);
      bool inConfigSection = false;
      std::string line;
      while (std::getline(file, line))
      {
         const std::string_view trimmed = TrimSv(line);
         if (trimmed.empty())
            continue;
         if (trimmed.front() == '[')
         {
            inConfigSection = trimmed == "[Config DOF]"sv || trimmed == "[Config outs]"sv;
            continue;
         }
         if (inConfigSection)
            if (const std::string_view shortRom = TrimSv(trimmed.substr(0, trimmed.find(','))); !shortRom.empty())
               romList.insert(DofToUpper(shortRom));
      }
   }
   return romList;
}

// Whether a rom name is declared in the rom list. Exact case-insensitive
// match only: anything looser would let ns_rom bind a config declared for ns,
// which is not the game this name was built for.
static bool DofRomListContains(const std::unordered_set<std::string>& romList, const std::string_view& romName) { return romList.contains(DofToUpper(romName)); }

static std::string ResolveDofRomName(const std::string_view& gameNs, const std::string_view& gameKey, const std::unordered_set<std::string>& romList)
{
   if (!gameNs.empty())
   {
      const std::string nsRom = std::string(gameNs) + "_" + std::string(gameKey);
      if (DofRomListContains(romList, nsRom))
         return nsRom;
   }
   return std::string(gameKey);
}

class DOFEventConsumer
{
public:
   DOFEventConsumer(const string& tablePath, const string& gameId, const ControllerDef& controller)
      : m_tablePath(tablePath)
      , m_gameId(gameId)
      , b2sPluginEventStream(
           std::make_unique<B2SPluginEventStream>(msgApi, endpointId, controller, [this](char type, int index, int value) { PostEvent({ static_cast<uint8_t>(type), index, value }); }))
   {
      m_thread = std::thread(&DOFEventConsumer::Run, this);
   }

   ~DOFEventConsumer()
   {
      b2sPluginEventStream = nullptr;
      {
         std::lock_guard lock(m_mutex);
         m_stopRequested = true;
      }
      m_cv.notify_one();

      if (m_thread.joinable())
         m_thread.join();
   }

   // Non-copyable, non-movable (owns a thread + sync primitives)
   DOFEventConsumer(const DOFEventConsumer&) = delete;
   DOFEventConsumer& operator=(const DOFEventConsumer&) = delete;

   struct B2SPluginEvent
   {
      uint8_t type;
      int32_t index;
      int32_t value;
   };

   void PostEvent(const B2SPluginEvent& ev)
   {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_queue.push(ev);
      }
      m_cv.notify_one();
   }

private:
   void Run()
   {
      SetThreadName("DOF.EventQueue"s);
      pDOF->Init(m_tablePath.c_str(), m_gameId.c_str());
      while (!m_stopRequested)
      {
         std::unique_lock lock(m_mutex);

         m_cv.wait(lock, [this] { return !m_queue.empty() || m_stopRequested; });

         if (m_stopRequested)
            break;

         while (!m_queue.empty())
         {
            B2SPluginEvent ev = m_queue.front();
            m_queue.pop();
            lock.unlock();
            pDOF->DataReceive(ev.type, ev.index, ev.value);
            lock.lock();
         }
      }
      pDOF->Finish();
   }

   const string m_tablePath;
   const string m_gameId;

   std::queue<B2SPluginEvent> m_queue;
   std::mutex m_mutex;
   std::condition_variable m_cv;
   bool m_stopRequested = false;
   std::thread m_thread;

   std::unique_ptr<B2SPluginEventStream> b2sPluginEventStream;
};

static std::unique_ptr<DOFEventConsumer> dofThread;

static void SetupDOF()
{
   const ControllerDef controller = controllers->With([](const std::vector<ControllerDef>& items) { return items.empty() ? ControllerDef { } : items.front(); });
   if (controller.gameId == nullptr || controller.endpointId == 0)
      return;
   const std::string_view gameNs = PinballPlugin::Controller::CtrlGetGameNamespace(controller.gameId);
   const std::string_view gameKey = PinballPlugin::Controller::CtrlGetGameKey(controller.gameId);
   if (gameKey.empty())
      return;

   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   const string path = tableInfo.path;
   const string romName = ResolveDofRomName(gameNs, gameKey, LoadDofRomList(path));

   LOGI("New game started: gameId="s + controller.gameId + ", romName=" + romName);
   dofThread = std::make_unique<DOFEventConsumer>(path, romName, controller);
}

}

using namespace DOFPlugin;

MSGPI_EXPORT void MSGPIAPI DOFPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   VPXInfo vpxInfo;
   vpxApi->GetVpxInfo(&vpxInfo);

   DOF::Config* pConfig = DOF::Config::GetInstance();
   pConfig->SetLogCallback(OnDOFLog);
   pConfig->SetBasePath(vpxInfo.prefPath);

   pDOF = std::make_unique<DOF::DOF>();

   controllers = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
      [](std::vector<ControllerDef>& items)
      {
         // Select the controller for which a DOF table config is declared
         // (ns_rom or rom in the rom list), a pinmame:: one winning over other
         // namespaces on ties (selection order is otherwise undefined). When no
         // controller resolves to a declared config, keep the legacy behavior
         // of binding the first pinmame one, then any controller: DOF can still
         // find table-filename mappings the rom list does not describe.
         const std::unordered_set<std::string> romList = LoadDofRomList(
            []
            {
               VPXTableInfo tableInfo;
               vpxApi->GetTableInfo(&tableInfo);
               return std::filesystem::path(tableInfo.path);
            }());
         const ControllerDef* selected = nullptr;
         int bestScore = -1;
         for (const ControllerDef& controller : items)
         {
            const std::string_view gameNs = PinballPlugin::Controller::CtrlGetGameNamespace(controller.gameId);
            const std::string_view gameKey = PinballPlugin::Controller::CtrlGetGameKey(controller.gameId);
            if (gameKey.empty())
               continue;
            const bool pinmame = gameNs == "pinmame"sv;
            const bool match = (!gameNs.empty() && DofRomListContains(romList, std::string(gameNs) + "_" + std::string(gameKey))) || DofRomListContains(romList, gameKey);
            const int score = (match ? 2 : 0) + (pinmame ? 1 : 0);
            if (score > bestScore)
            {
               bestScore = score;
               selected = &controller;
               if (score == 3)
                  break;
            }
         }
         items.clear();
         if (selected != nullptr)
            items.push_back(*selected);
      },
      []() { dofThread = nullptr; }, []() { SetupDOF(); });
   controllers->Subscribe();
}

MSGPI_EXPORT void MSGPIAPI DOFPluginUnload()
{
   controllers->Unsubscribe();
   controllers = nullptr;
   pDOF = nullptr;

   msgApi = nullptr;
   vpxApi = nullptr;
}
