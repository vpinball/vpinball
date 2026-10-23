// license:GPLv3+

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPTrigger.h"
#include "PUPCustomPos.h"
#include "LibAv.h"

#pragma warning(push)
#pragma warning(disable : 4267)
#include "RSJparser/RSJparser.tcc"
#pragma warning(pop)

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace PUP {

MSGPI_FLOAT_VAL_SETTING(pupMainVolume, "MainVol", "Main Volume", "Overall volume", true, 0.f, 1.f, 0.01f, 1.f);

MSGPI_INT_VAL_SETTING(pupBGPadLeft, "BGPadLeft", "Backglass Left Pad", "Left Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadRight, "BGPadRight", "Backglass Right Pad", "Right Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadTop, "BGPadTop", "Backglass Top Pad", "Top Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadBottom, "BGPadBottom", "Backglass Bottom Pad", "Bottom Padding of backglass", true, 0, 4096, 0);

MSGPI_INT_VAL_SETTING(pupSVPadLeft, "SVPadLeft", "Score View Left Pad", "Left Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadRight, "SVPadRight", "Score View Right Pad", "Right Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadTop, "SVPadTop", "Score View Top Pad", "Top Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadBottom, "SVPadBottom", "Score View Bottom Pad", "Bottom Padding of Score View", true, 0, 4096, 0);

MSGPI_INT_VAL_SETTING(pupTopperPadLeft, "TopperPadLeft", "Topper Left Pad", "Left Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadRight, "TopperPadRight", "Topper Right Pad", "Right Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadTop, "TopperPadTop", "Topper Top Pad", "Top Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadBottom, "TopperPadBottom", "Topper Bottom Pad", "Bottom Padding of topper", true, 0, 4096, 0);

PUPManager::PUPManager(const MsgPluginAPI* msgApi, uint32_t endpointId, const PagedUIPluginAPI* pagedUIApi, const std::filesystem::path& rootPath)
   : m_szRootPath(rootPath)
   , m_endpointId(endpointId)
   , m_msgApi(msgApi)
   , m_pagedUIApi(pagedUIApi)
   , m_resURIResolver(*msgApi, endpointId, true, false, false)
   , m_getVpxApiId(m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API))
   , m_getAuxRendererId(m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_getAudioSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_GET_SRC_MSG))
   , m_onAudioSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_SRC_CHG_MSG))
   , m_audioSrcDef({ .id = { endpointId, 0 }, .overrideId = { 0, 0 }, .name = "PinUp Player", .desc = "PinUp Player audio stream", .target = CTLPI_AUDIO_TARGET_BACKGLASS })
{
   msgApi->RegisterSetting(endpointId, &pupMainVolume);
   m_mainVolume = pupMainVolume_Get();
   msgApi->RegisterSetting(endpointId, &pupBGPadLeft);
   msgApi->RegisterSetting(endpointId, &pupBGPadRight);
   msgApi->RegisterSetting(endpointId, &pupBGPadTop);
   msgApi->RegisterSetting(endpointId, &pupBGPadBottom);
   msgApi->RegisterSetting(endpointId, &pupSVPadLeft);
   msgApi->RegisterSetting(endpointId, &pupSVPadRight);
   msgApi->RegisterSetting(endpointId, &pupSVPadTop);
   msgApi->RegisterSetting(endpointId, &pupSVPadBottom);
   msgApi->RegisterSetting(endpointId, &pupTopperPadLeft);
   msgApi->RegisterSetting(endpointId, &pupTopperPadRight);
   msgApi->RegisterSetting(endpointId, &pupTopperPadTop);
   msgApi->RegisterSetting(endpointId, &pupTopperPadBottom);

   m_msgApi->SubscribeMsg(m_endpointId, m_getAudioSrcId, OnGetAudioSrc, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAudioSrcChangedId, nullptr);

   m_msgApi->SubscribeMsg(m_endpointId, m_getAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   m_msgApi->BroadcastMsg(m_endpointId, m_getVpxApiId, &m_vpxApi);
}

PUPManager::~PUPManager()
{
   Unload();

   m_msgApi->UnsubscribeMsg(m_getAudioSrcId, OnGetAudioSrc, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAudioSrcChangedId, nullptr);
   m_msgApi->ReleaseMsgID(m_getAudioSrcId);
   m_msgApi->ReleaseMsgID(m_onAudioSrcChangedId);

   m_msgApi->UnsubscribeMsg(m_getAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_getAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);

   m_msgApi->ReleaseMsgID(m_getVpxApiId);

   m_msgApi->FlushPendingCallbacks(m_endpointId);
}

void PUPManager::Start()
{
   LOGI("PUP Manager start"s);
   assert(!IsRunning());
   m_B2SPluginEventStream = std::make_unique<B2SPluginEventStream>(m_msgApi, m_endpointId, m_controller, [this](char c, int id, int value) { QueueDOFEvent(c, id, value); });
   m_B2SPluginEventStream->SetDMDHandler(
      [](const GetDisplaySrcMsg& sources)
      {
         DisplaySrcId selected {};
         unsigned int largest = 128;
         for (unsigned int i = 0; i < sources.count; i++)
         {
            if (sources.entries[i].width >= largest && sources.entries[i].GetIdentifyFrame)
            {
               selected = sources.entries[i];
               largest = sources.entries[i].width;
            }
         }
         return selected;
      },
      [this](const DisplaySrcId& src, const uint8_t* frame) { return ProcessDmdFrame(src, frame); });
   m_B2SPluginEventStream->SetDmdIdentificationHandler(
      [](bool bySerum)
      {
         if (bySerum)
            LOGI("DMD frame identification provided by Serum; local matching disabled"s);
         else
            LOGI("DMD frame identification handled locally"s);
      });
}

void PUPManager::Stop()
{
   LOGI("PUP Manager stop"s);
   assert(IsRunning());
   m_B2SPluginEventStream = nullptr;
}

std::filesystem::path PUPManager::FindGameDir(const std::string_view& gameNs, const std::string_view& gameId) const
{
   if (gameId.empty())
      return {};

   // First search for pupvideos along the table file
   if (m_vpxApi != nullptr)
   {
      VPXTableInfo tableInfo {};
      m_vpxApi->GetTableInfo(&tableInfo);
      const std::filesystem::path pupBase = PluginStrings::PathFromNative(tableInfo.path).parent_path() / "pupvideos"sv;
      if (!gameNs.empty())
         if (std::filesystem::path path = find_case_insensitive_directory_path(pupBase / PluginStrings::PathFromUTF8(gameNs) / PluginStrings::PathFromUTF8(gameId)); !path.empty())
            return path;
      if (std::filesystem::path path = find_case_insensitive_directory_path(pupBase / PluginStrings::PathFromUTF8(gameId)); !path.empty())
         return path;
   }

   // If we did not find the pup folder along the table, search for it in the global 'pupvideos' path if defined
   if (!m_szRootPath.empty())
   {
      if (!gameNs.empty())
         if (std::filesystem::path path = find_case_insensitive_directory_path(m_szRootPath / PluginStrings::PathFromUTF8(gameNs) / PluginStrings::PathFromUTF8(gameId)); !path.empty())
            return path;
      return find_case_insensitive_directory_path(m_szRootPath / PluginStrings::PathFromUTF8(gameId));
   }

   return {};
}

void PUPManager::ApplyGameDir(const std::filesystem::path& path, const std::string_view& gameId, const ControllerDef& controller)
{
   if (path.empty() || path == m_szPath)
      return;

   m_szPath = path;
   m_szRomName = gameId;
   m_controllerGameId = controller.gameId != nullptr ? controller.gameId : "";
   m_controller = { controller.endpointId, m_controllerGameId.c_str() };
   LOGI("PUP path: " + PluginStrings::PathToUTF8(m_szPath));

   // Load Fonts
   LoadFonts();
   m_sounds.Load(m_szPath);
}

void PUPManager::SetGameDir(const ControllerDef& controller)
{
   assert(!IsRunning());

   // Guard against empty rom names (can come from early B2S broadcasts before the B2S name is
   // committed - see Monster Bash's pupvideos/mb_106b folder never being located otherwise).
   // With an empty component, `pupvideos` / "" resolves to the pupvideos parent directory which
   // then satisfies the directory-exists check in find_case_insensitive_directory_path and
   // wrongly becomes m_szPath.
   const std::string_view gameId = PinballPlugin::Controller::CtrlGetGameKey(controller.gameId);
   if (gameId.empty())
      return;

   ApplyGameDir(FindGameDir(PinballPlugin::Controller::CtrlGetGameNamespace(controller.gameId), gameId), gameId, controller);
}

void PUPManager::SetGameDir(const string& szRomName)
{
   assert(!IsRunning());
   if (szRomName.empty())
      return;

   // The script only hands over a rom name: bind to the controller exposing it
   // when there is one, otherwise keep the legacy folder lookup bound to none.
   ControllerDef controller = SelectControllerForGame(szRomName);
   if (controller.endpointId == 0)
      controller = { 0, szRomName.c_str() };
   SetGameDir(controller);
}

namespace
{
   int CommitOverlayDoc(PUPOverlayDoc& doc);
}

void PUPManager::LoadConfig(const ControllerDef& controller)
{
   const std::string_view gameId = PinballPlugin::Controller::CtrlGetGameKey(controller.gameId);

   // Tables commonly call B2SInit multiple times, and some tables configure PuP
   // entirely from script (PuPlayer.Init / playlistadd) before B2SInit ever fires.
   // In both cases, if we already have state for this ROM, keep it - just make
   // sure the manager is running and the initial DOF event has been queued.
   if (!m_szPath.empty() && StrCompareNoCase(string(gameId), m_szRomName) && (m_controller.endpointId == controller.endpointId || m_controller.endpointId == 0))
   {
      LOGI("Same ROM, skipping re-init"s);
      const bool wasRunning = IsRunning();
      if (m_controller.endpointId != controller.endpointId)
      {
         if (wasRunning)
            Stop();
         m_controllerGameId = controller.gameId != nullptr ? controller.gameId : "";
         m_controller = { controller.endpointId, m_controllerGameId.c_str() };
      }
      if (!IsRunning())
         Start();
      if (!wasRunning)
         QueueDOFEvent('D', 0, 1);
      return;
   }

   Unload();

   SetGameDir(controller);

   // Set game dir will define the path to the pup files, or empty it if not found
   if (m_szPath.empty())
      return;

   // Load the authored display list and expose it through the in game UI
   m_overlayDoc.path = m_szPath / "displays.pup";
   // Bind the shared overlay document to the plugin data model (see DMDOverlay/DMDOverlay.h):
   // three windows, bounds in window pixels rendered through the default full-view flip (no
   // mapQuad hook), glass image referenced by a pack-relative path staged until commit copies
   // it into the pack's 'PUPDisplays' folder, JSON persistence by CommitOverlayDoc
   static const char* targetNames[] = { "Backglass", "Score view", "Topper" };
   m_overlayDoc.vpxApi = m_vpxApi;
   m_overlayDoc.log = [](const char* func, int line, unsigned int level, const std::string& message) { LPILog_CPP(func, line, level, message); };
   m_overlayDoc.targetCount = 3;
   m_overlayDoc.targetNames = targetNames;
   m_overlayDoc.hasGlass = [](const PUPDMDDisplay& display) { return !display.glassPending.empty() || !display.glassPath.empty(); };
   m_overlayDoc.glassBytes = [](PUPDMDDisplay& display) -> const vector<uint8_t>*
   {
      // The staged pick resolves to a file before the commit copies it into the pack; the
      // persisted path is relative to the pup pack root. The file content is cached in
      // glassCache, keyed by the wanted path: this is called every frame, so the file system
      // is only accessed (case insensitive lookup, read) when the path changes
      std::filesystem::path wanted;
      if (!display.glassPending.empty())
         wanted = PluginStrings::PathFromNative(display.glassPending.c_str());
      else if (display.doc != nullptr && !display.glassPath.empty())
         wanted = display.doc->path.parent_path() / PluginStrings::PathFromUTF8(display.glassPath);
      if (wanted.empty())
      {
         display.glassKey = 0;
         display.glassCache.clear();
         return nullptr;
      }
      wanted = wanted.lexically_normal();
      const string key = PluginStrings::PathToUTF8(wanted);
      size_t hash = 14695981039346656037ull;
      for (const char c : key)
         hash = (hash ^ static_cast<uint8_t>(c)) * 1099511628211ull;
      if (hash != display.glassKey)
      {
         display.glassKey = hash;
         display.glassCache.clear();
         const std::filesystem::path resolved = find_case_insensitive_file_path(wanted);
         if (resolved.empty() || !DMDOverlay::ReadFileBytes(resolved, display.glassCache))
            LOGE("Missing glass file: " + key);
      }
      return display.glassCache.empty() ? nullptr : &display.glassCache;
   };
   m_overlayDoc.setGlass = [](PUPDMDDisplay& display, const std::filesystem::path& picked)
   {
      // The picked file is staged: it lands in 'PUPDisplays' at the next successful commit, so
      // cancelling the edit leaves no orphan files. The staged path keeps the picker encoding
      display.glassPending = PluginStrings::PathToNative(picked);
      display.glassPath = "PUPDisplays/"s + PluginStrings::PathToUTF8(picked.filename());
      return true;
   };
   m_overlayDoc.clearGlass = [](PUPDMDDisplay& display)
   {
      display.glassPath.clear();
      display.glassPending.clear();
   };
   m_overlayDoc.commitEdit = &CommitOverlayDoc;
   LoadDisplays();
   RegisterOverlaySheet();

   // Load playlists
   LoadPlaylists();

   // Load Fonts
   LoadFonts();

   // Load screens and start them
   if (std::filesystem::path szScreensPath = find_case_insensitive_file_path(m_szPath / "screens.pup"sv); !szScreensPath.empty())
   {
      std::ifstream screensFile;
      screensFile.open(szScreensPath, std::ifstream::in);
      if (screensFile.is_open()) {
         string line;
         int i = 0;
         while (std::getline(screensFile, line)) {
            if (++i == 1)
               continue;
            std::unique_ptr<PUPScreen> pScreen = PUPScreen::CreateFromCSV(this, PluginStrings::TextFromUTF8OrCP1252(line), m_playlists);
            if (pScreen)
               AddScreen(std::move(pScreen));
         }
      }
      else {
         LOGE("Unable to load " + PluginStrings::PathToUTF8(szScreensPath));
      }
   }
   else {
      LOGI("No screens.pup file found"s);
   }

   // Triggers that only a DMD frame match can fire.
   //
   // Counted from the parsed conditions rather than the trigger string, which
   // is what QueueDOFEvent matches on too: a trigger may carry several
   // conditions, so its string is not always a single "D1234". Number 0 is
   // excluded, being PuP's own startup event -- queued below by this code
   // rather than matched against a frame.
   m_dmdTriggerCount = 0;
   for (const auto& [screenNum, pScreen] : m_screenMap)
      for (const auto& [szTrigger, triggers] : pScreen->GetTriggers())
         for (PUPTrigger* pTrigger : triggers)
            if (std::ranges::any_of(pTrigger->GetTriggers(), [](const PUPTrigger::PUPTriggerCondition& condition) { return condition.m_type == 'D' && condition.m_number != 0; }))
               ++m_dmdTriggerCount;

   Start();

   // Queue initial game event
   QueueDOFEvent('D', 0, 1);
}

void PUPManager::LoadConfig(const string& szRomName)
{
   if (szRomName.empty())
      return;
   ControllerDef controller = SelectControllerForGame(szRomName);
   if (controller.endpointId == 0)
      controller = { 0, szRomName.c_str() };
   LoadConfig(controller);
}

// Select the controller exposing the given game key, a pinmame:: one winning
// over other namespaces when several match (selection order is otherwise
// undefined). Returns an empty ControllerDef when none does.
ControllerDef PUPManager::SelectControllerForGame(const std::string_view& gameKey)
{
   const unsigned int getControllersId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
   const std::vector<ControllerDef> controllers = PinballPlugin::Controller::GetCtrlItems<ControllerDef>(m_msgApi, m_endpointId, getControllersId);
   m_msgApi->ReleaseMsgID(getControllersId);

   const ControllerDef* selected = nullptr;
   for (const ControllerDef& controller : controllers)
   {
      if (!StrCompareNoCase(string(PinballPlugin::Controller::CtrlGetGameKey(controller.gameId)), string(gameKey)))
         continue;
      if (PinballPlugin::Controller::CtrlGetGameNamespace(controller.gameId) == "pinmame"sv)
      {
         selected = &controller;
         break;
      }
      if (selected == nullptr)
         selected = &controller;
   }
   if (selected == nullptr)
      return {};

   m_controllerGameId = selected->gameId;
   return { selected->endpointId, m_controllerGameId.c_str() };
}

void PUPManager::Unload()
{
   m_runToken = std::make_shared<int>(0);
   m_ticking = false;

   // Run any still-queued trigger invokes (see QueueDOFEvent) while the triggers
   // and screens they point to are still alive.
   m_msgApi->FlushPendingCallbacks(m_endpointId);

   if (IsRunning())
      Stop();

   m_screenOrder.clear();
   m_screenMap.clear();

   if (m_overlaySheetRegistered && m_pagedUIApi != nullptr)
   {
      m_pagedUIApi->UnregisterSheet(m_endpointId, "displays");
      m_overlaySheetRegistered = false;
   }
   m_overlayDoc = PUPOverlayDoc {};
   m_dmdOverlays.clear();

   m_dmd = nullptr;
   m_dmdTriggerCount = 0;
   m_dmdTriggerDataLoaded = false;
   m_reportedMissingIdentification = false;

   UnloadFonts();
   m_sounds.Unload();

   for (auto& playlist : m_playlists)
      delete playlist;
   m_playlists.clear();

   m_szPath.clear();
   m_szRomName.clear();
   m_controller = {};
   m_controllerGameId.clear();
}

void PUPManager::UnloadFonts()
{
   m_fontMap.clear();
   m_fontFilenameMap.clear();
   m_fonts.clear();
}

void PUPManager::LoadFonts()
{
   UnloadFonts();
   std::filesystem::path szFontsPath = find_case_insensitive_directory_path(m_szPath / "FONTS"sv);
   if (!szFontsPath.empty())
   {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath))
      {
         if (entry.is_regular_file())
         {
            std::filesystem::path szFontPath = entry.path();
            const std::filesystem::path szFontExt = lowerCase(szFontPath.extension());
            if (szFontExt == ".ttf" || szFontExt == ".otf")
            {
               if (TTF_Font* pTTFFont = TTF_OpenFont(PluginStrings::PathToUTF8(szFontPath).c_str(), 8))
               {
                  AddFont(std::make_unique<PUPFont>(pTTFFont, szFontPath), PluginStrings::PathToUTF8(entry.path().filename()));
               }
               else
               {
                  LOGE("Failed to load font: " + PluginStrings::PathToUTF8(szFontPath) + ' ' + SDL_GetError());
               }
            }
         }
      }
   }
   else
   {
      LOGI("No FONTS folder found"s);
   }

   if (m_vpxApi)
   {
      VPXInfo vpxInfo {};
      m_vpxApi->GetVpxInfo(&vpxInfo);
      std::filesystem::path fallbackPath = PluginStrings::PathFromNative(vpxInfo.path) / "assets" / "LiberationSans-Regular.ttf";
      if (TTF_Font* pTTFFont = TTF_OpenFont(PluginStrings::PathToUTF8(fallbackPath).c_str(), 8))
         AddFont(std::make_unique<PUPFont>(pTTFFont, fallbackPath), "LiberationSans-Regular.ttf");
   }
}

void PUPManager::LoadPlaylists()
{
   std::filesystem::path szPlaylistsPath = find_case_insensitive_file_path(GetPath() / "playlists.pup"sv);
   std::ifstream playlistsFile;
   playlistsFile.open(szPlaylistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      ankerl::unordered_dense::set<std::string> lowerPlaylistNames;
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;
         PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSV(this, PluginStrings::TextFromUTF8OrCP1252(line));
         if (pPlaylist) {
            string folderNameLower = lowerCase(PluginStrings::PathToUTF8(pPlaylist->GetFolder()));
            if (lowerPlaylistNames.find(folderNameLower) == lowerPlaylistNames.end()) {
               m_playlists.push_back(pPlaylist);
               lowerPlaylistNames.insert(folderNameLower);
            }
            else {
               LOGE("Duplicate playlist: playlist=" + pPlaylist->ToString());
               delete pPlaylist;
            }
         }
      }
   }
}

namespace
{
   // Copies the glass images staged by a file pick or a Nixie import into the pack's
   // 'PUPDisplays' folder, deduplicating names; updates glassPath to the stored
   // relative path and clears the staged source
   void CopyStagedGlassImages(PUPOverlayDoc& doc)
   {
      const std::filesystem::path glassDir = doc.path.parent_path() / "PUPDisplays";
      for (PUPDMDDisplay& display : doc.displays)
      {
         if (display.glassPending.empty())
            continue;
         const std::filesystem::path src = PluginStrings::PathFromNative(display.glassPending.c_str());
         std::error_code ec;
         std::filesystem::create_directories(glassDir, ec);
         std::filesystem::path dst = glassDir / src.filename();
         if (!std::filesystem::equivalent(src, dst, ec))
         {
            for (int n = 1; std::filesystem::exists(dst, ec); n++)
            {
               std::filesystem::path candidate = src.stem();
               candidate += "_" + std::to_string(n);
               candidate += src.extension();
               dst = glassDir / candidate;
            }
            if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
               LOGE("Failed to copy glass image " + PluginStrings::PathToUTF8(src) + " to " + PluginStrings::PathToUTF8(dst) + ": " + ec.message());
         }
         display.glassPath = "PUPDisplays/"s + PluginStrings::PathToUTF8(dst.filename());
         display.glassPending.clear();
      }
   }

   // Rewrites the whole document file: a JSON 'Displays' array of { Target, Source, X, Y, W, H,
   // plus the optional render style/tint/glass fields }, matching the other .pup files of the pack
   // (Doc::commitEdit hook)
   int CommitOverlayDoc(PUPOverlayDoc& doc)
   {
      if (doc.path.empty())
         return 1;
      CopyStagedGlassImages(doc);
      std::ofstream out(doc.path, std::ios::binary | std::ios::trunc);
      if (!out)
      {
         LOGE("Failed to write displays file: " + PluginStrings::PathToUTF8(doc.path));
         return 1;
      }
      static const char* targetNames[] = { "Backglass", "ScoreView", "Topper" };
      nlohmann::json json;
      json["Displays"] = nlohmann::json::array();
      for (const PUPDMDDisplay& display : doc.displays)
      {
         const char* const target = display.target >= 0 && display.target < 3 ? targetNames[display.target] : targetNames[0];
         nlohmann::json entry = { { "Target", target }, { "Source", display.source }, { "X", display.x }, { "Y", display.y }, { "W", display.w }, { "H", display.h } };
         const PinballPlugin::OverlayDisplayOptions& o = display.options;
         if (o.style > 0 && o.style <= 8)
            entry["Style"] = PinballPlugin::OverlayDisplayStyleNames[o.style - 1];
         if (o.sizeW > 0 || o.sizeH > 0)
            entry["Dots"] = { o.sizeW, o.sizeH };
         if (o.tintR != 1.f || o.tintG != 1.f || o.tintB != 1.f)
            entry["Tint"] = { o.tintR, o.tintG, o.tintB };
         nlohmann::json glass;
         if (!display.glassPath.empty())
            glass["Image"] = display.glassPath;
         if (o.glassAreaX != 0 || o.glassAreaY != 0 || o.glassAreaW != 0 || o.glassAreaH != 0)
            glass["Area"] = { o.glassAreaX, o.glassAreaY, o.glassAreaW, o.glassAreaH };
         if (o.glassTintR != 1.f || o.glassTintG != 1.f || o.glassTintB != 1.f)
            glass["Tint"] = { o.glassTintR, o.glassTintG, o.glassTintB };
         if (o.glassRoughness != 0.f)
            glass["Roughness"] = o.glassRoughness;
         if (o.glassAmbientR != 1.f || o.glassAmbientG != 1.f || o.glassAmbientB != 1.f)
            glass["Ambient"] = { o.glassAmbientR, o.glassAmbientG, o.glassAmbientB };
         if (o.glassPadL != 0.f || o.glassPadT != 0.f || o.glassPadR != 0.f || o.glassPadB != 0.f)
            glass["Pad"] = { o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB };
         if (!glass.empty())
            entry["Glass"] = glass;
         json["Displays"].push_back(entry);
      }
      out << json.dump(4);
      out.close();
      if (out.fail())
      {
         LOGE("Failed to write displays file: " + PluginStrings::PathToUTF8(doc.path));
         return 1;
      }
      return 0;
   }
}

// Loads the authored display list of the pack: a JSON file matching the other .pup files of
// the folder, holding a 'Displays' array of { Target, Source, X, Y, W, H } objects (Target:
// "Backglass", "ScoreView" or "Topper"; Source: controller resource URI or empty for the
// default DMD; X/Y/W/H: bounds in window pixels from the top left) with the optional render
// style ('Style' name, 'Dots' size, emitter 'Tint') and 'Glass' object (Image path relative
// to the pack root inside 'PUPDisplays', Area, Tint, Roughness, Ambient, Pad)
void PUPManager::LoadDisplays()
{
   m_overlayDoc.displays.clear();
   const std::filesystem::path szDisplaysPath = find_case_insensitive_file_path(m_overlayDoc.path);
   if (!szDisplaysPath.empty())
   {
      std::ifstream displaysFile;
      displaysFile.open(szDisplaysPath, std::ifstream::in);
      if (displaysFile.is_open())
      {
         try
         {
            const nlohmann::json json = nlohmann::json::parse(displaysFile);
            // Expected shape: { "Displays": [ ... ] }; a bare top level array is also accepted
            const nlohmann::json& displays = json.is_object() && json.contains("Displays") ? json["Displays"] : json;
            const auto parseFloats = [](const nlohmann::json& j, float* values, int count)
            {
               if (!j.is_array())
                  return false;
               for (int i = 0; i < count; i++)
               {
                  if (i >= static_cast<int>(j.size()) || !j[i].is_number())
                     return false;
                  values[i] = j[i].get<float>();
               }
               return true;
            };
            if (displays.is_array())
               for (const nlohmann::json& entry : displays)
               {
                  if (!entry.is_object())
                     continue;
                  PUPDMDDisplay& display = m_overlayDoc.displays.emplace_back();
                  const string target = entry.value("Target", "Backglass"s);
                  display.target = StrCompareNoCase(target, "ScoreView"s) ? 1 : StrCompareNoCase(target, "Topper"s) ? 2 : 0;
                  display.source = entry.value("Source", ""s);
                  display.x = entry.value("X", 0);
                  display.y = entry.value("Y", 0);
                  display.w = entry.value("W", 0);
                  display.h = entry.value("H", 0);
                  const string style = entry.value("Style", ""s);
                  display.options.style = PinballPlugin::OverlayDisplayStyleIndex(style.empty() ? nullptr : style.c_str());
                  float f[4];
                  if (parseFloats(entry.value("Dots", nlohmann::json {}), f, 2))
                  {
                     display.options.sizeW = static_cast<int>(f[0]);
                     display.options.sizeH = static_cast<int>(f[1]);
                  }
                  if (parseFloats(entry.value("Tint", nlohmann::json {}), f, 3))
                  {
                     display.options.tintR = f[0];
                     display.options.tintG = f[1];
                     display.options.tintB = f[2];
                  }
                  if (const nlohmann::json& glass = entry.value("Glass", nlohmann::json {}); glass.is_object())
                  {
                     display.glassPath = glass.value("Image", ""s);
                     if (parseFloats(glass.value("Area", nlohmann::json {}), f, 4))
                     {
                        display.options.glassAreaX = static_cast<int>(f[0]);
                        display.options.glassAreaY = static_cast<int>(f[1]);
                        display.options.glassAreaW = static_cast<int>(f[2]);
                        display.options.glassAreaH = static_cast<int>(f[3]);
                     }
                     if (parseFloats(glass.value("Tint", nlohmann::json {}), f, 3))
                     {
                        display.options.glassTintR = f[0];
                        display.options.glassTintG = f[1];
                        display.options.glassTintB = f[2];
                     }
                     if (const nlohmann::json& roughness = glass.value("Roughness", nlohmann::json {}); roughness.is_number())
                        display.options.glassRoughness = roughness.get<float>();
                     if (parseFloats(glass.value("Ambient", nlohmann::json {}), f, 3))
                     {
                        display.options.glassAmbientR = f[0];
                        display.options.glassAmbientG = f[1];
                        display.options.glassAmbientB = f[2];
                     }
                     if (parseFloats(glass.value("Pad", nlohmann::json {}), f, 4))
                     {
                        display.options.glassPadL = f[0];
                        display.options.glassPadT = f[1];
                        display.options.glassPadR = f[2];
                        display.options.glassPadB = f[3];
                     }
                  }
               }
         }
         catch (const std::exception& e)
         {
            LOGE("Failed to parse " + PluginStrings::PathToUTF8(szDisplaysPath) + ": " + e.what());
         }
      }
      else
      {
         LOGE("Unable to load " + PluginStrings::PathToUTF8(szDisplaysPath));
      }
   }
   for (PUPDMDDisplay& display : m_overlayDoc.displays)
      display.doc = &m_overlayDoc;
}

void PUPManager::RegisterOverlaySheet()
{
   if (m_pagedUIApi == nullptr || m_overlaySheetRegistered || m_overlayDoc.path.empty())
      return;
   m_overlayDoc.requestRefresh = [this]() { m_pagedUIApi->RequestSheetRefresh(m_endpointId, "displays"); };
   const string name = "Modify \""s + PluginStrings::PathToUTF8(m_overlayDoc.path.filename()) + "\"";
   PagedUISheetDef sheet {};
   sheet.version = 1;
   sheet.id = "displays";
   sheet.name = name.c_str();
   sheet.description = "Layout of the loaded PinUp Player displays";
   DMDOverlay::InitEditSheetDef(&sheet, &m_overlayDoc);
   m_pagedUIApi->RegisterSheet(m_endpointId, &sheet);
   m_overlaySheetRegistered = true;
}

// Renders the displays of the document targeted at this window, growing the overlay pool on
// demand (a display presence in the document means enabled). Each display keeps the overlay of its
// document index whatever its target, so that the overlay state (glass texture, uploaded frame) is
// not shared between displays of different windows
int PUPManager::RenderDisplays(VPXRenderContext2D* ctx, int target)
{
   if (m_vpxApi == nullptr)
      return 0;
   m_overlayDoc.viewW[target] = static_cast<int>(ctx->srcWidth);
   m_overlayDoc.viewH[target] = static_cast<int>(ctx->srcHeight);
   int n = 0;
   for (size_t i = 0; i < m_overlayDoc.displays.size(); i++)
   {
      PUPDMDDisplay& display = m_overlayDoc.displays[i];
      if (display.target != target)
         continue;
      while (i >= m_dmdOverlays.size())
         m_dmdOverlays.emplace_back(std::make_unique<DMDOverlay::Renderer>(m_vpxApi, m_resURIResolver));
      m_dmdOverlays[i]->Render(ctx, display, nullptr);
      n++;
   }
   return n;
}

void PUPManager::Tick()
{
   const double gameTime = m_vpxApi ? m_vpxApi->GetGameTime() : 0.0;
   for (const auto& [key, screen] : m_screenMap)
      screen->Update(gameTime);
   RunLater(0.05, [this]() { Tick(); });
}

bool PUPManager::AddScreen(std::shared_ptr<PUPScreen> pScreen)
{
   if (!m_ticking)
   {
      m_ticking = true;
      RunLater(0.05, [this]() { Tick(); });
   }

   std::unique_lock lock(m_eventMutex);

   if (std::shared_ptr<PUPScreen> existing = GetScreen(pScreen->GetScreenNum()); existing)
   {
      LOGI("Replacing previously defined PUP screen: existing={" + existing->ToString(false) + "} ne<={" + pScreen->ToString(false) + '}');
      if (existing->GetParent())
         existing->GetParent()->ReplaceChild(existing, pScreen);
      for (const auto& [key, screen] : m_screenMap)
      {
         if (screen->GetParent() == existing.get())
         {
            pScreen->AddChild(screen);
         }
      }
      std::erase(m_screenOrder, existing);
   }
   pScreen->SetMainVolume(m_mainVolume);
   m_screenMap[pScreen->GetScreenNum()] = pScreen;
   m_screenOrder.push_back(pScreen);

   if (const std::unique_ptr<PUPCustomPos>& pCustomPos = pScreen->GetCustomPos(); pCustomPos)
   {
      const auto it = m_screenMap.find(pCustomPos->GetSourceScreen());
      std::shared_ptr<PUPScreen> parent;
      if (it != m_screenMap.end()) {
         parent = it->second;
      }
      else {
         switch (pCustomPos->GetSourceScreen()) {
         case 0:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(0,"Topper","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 1:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(1,"DMD 4x1","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 2:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(2,"Backglass 16x9","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 3:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(3,"Playfield","",,0,Off,0,)"s, m_playlists)); break;
         case 4:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(4,"Music","",,0,MusicOnly,0,)"s, m_playlists)); break;
         case 5:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(5,"FullDMD 16x9","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 6:  parent = /*std::move*/(PUPScreen::CreateFromCSV(this, R"(6,"Backglass 4x3-5x4","",,0,ForceBack,0,)"s, m_playlists)); break;
         default: parent = /*std::move*/(PUPScreen::CreateFromCSV(this, std::to_string(pCustomPos->GetSourceScreen()) + R"(,"","",,0,ForceBack,0,)"s, m_playlists)); break;
         }
         if (parent)
         {
            lock.unlock();
            AddScreen(parent);
            lock.lock();
         }
      }
      if (parent && pScreen != parent)
         parent->AddChild(pScreen);
   }

   LOGI("Screen added: screen={" + pScreen->ToString() + '}');

   return true;
}

bool PUPManager::AddScreen(int screenNum)
{
   std::unique_ptr<PUPScreen> pScreen = PUPScreen::CreateDefault(this, screenNum, m_playlists);
   return pScreen ? AddScreen(std::move(pScreen)) : false;
}

void PUPManager::SendScreenToBack(const PUPScreen* screen)
{
   LOGD("Send screen to back " + std::to_string(screen->GetScreenNum()));
   auto it = std::ranges::find_if(m_screenOrder, [screen](std::shared_ptr<PUPScreen> s) { return s.get() == screen; });
   if (it != m_screenOrder.end())
   {
      auto item = std::move(*it);
      m_screenOrder.erase(it);
      m_screenOrder.insert(m_screenOrder.begin(), item);
   }
}

void PUPManager::SendScreenToFront(const PUPScreen* screen)
{
   LOGD("Send screen to front " + std::to_string(screen->GetScreenNum()));
   auto it = std::ranges::find_if(m_screenOrder, [screen](std::shared_ptr<PUPScreen> s) { return s.get() == screen; });
   if (it != m_screenOrder.end())
   {
      auto item = std::move(*it);
      m_screenOrder.erase(it);
      m_screenOrder.push_back(item);
   }
}

std::shared_ptr<PUPScreen> PUPManager::GetScreen(int screenNum, bool logMissing) const
{
   if (const auto it = m_screenMap.find(screenNum); it != m_screenMap.end())
      return it->second;
   if (logMissing)
   {
      LOGE("Screen not found: screenNum=" + std::to_string(screenNum));
   }
   return nullptr;
}

bool PUPManager::AddFont(std::unique_ptr<PUPFont> pFont, const string& szFilename)
{
   if (!pFont || !pFont->GetTTFFont())
      return false;

   TTF_Font* const pTTFFont = pFont->GetTTFFont();

   // SDL_ttf gives nullptr for fonts without family or style name: the family falls back to the file name without extension
   const char* const familyName = TTF_GetFontFamilyName(pTTFFont);
   const string szFamilyName = familyName ? string(familyName) : szFilename.substr(0, szFilename.find_last_of('.'));
   const string szNormalizedFamilyName = lowerCase(string_replace_all(szFamilyName, "  "s, ' '));
   m_fontMap[szNormalizedFamilyName] = pFont.get();

   const char* const styleName = TTF_GetFontStyleName(pTTFFont);
   const string szStyleName = styleName ? string(styleName) : string();
   if (!szStyleName.empty() && szStyleName != "Regular")
   {
      const string szFullName = szFamilyName + ' ' + szStyleName;
      const string szNormalizedFullName = lowerCase(string_replace_all(szFullName, "  "s, ' '));
      m_fontMap[szNormalizedFullName] = pFont.get();
   }

   const string szNormalizedFilename = lowerCase(szFilename.substr(0, szFilename.length() - 4));
   m_fontFilenameMap[szNormalizedFilename] = pFont.get();

   LOGI(std::format("Font added: familyName={}, styleName={}, filename={}, winScale={}, winAscentRatio={}", szFamilyName, szStyleName, szFilename, pFont->GetWinScale(), pFont->GetWinAscentRatio()));

   m_fonts.push_back(std::move(pFont));
   return true;
}

PUPFont* PUPManager::GetFont(const string& szFont)
{
   string szNormalizedFamilyName = lowerCase(string_replace_all(szFont, "  "s, ' '));

   auto it = m_fontMap.find(szNormalizedFamilyName);
   if (it != m_fontMap.end())
      return it->second;
   it = m_fontFilenameMap.find(lowerCase(szFont));
   if (it != m_fontFilenameMap.end())
      return it->second;

   return nullptr;
}

int PUPManager::ProcessDmdFrame(const DisplaySrcId& src, const uint8_t* frame)
{
   // Unsupported frame format ?
   if (src.identifyFormat != CTLPI_DISPLAY_ID_FORMAT_BITPLANE2 && src.identifyFormat != CTLPI_DISPLAY_ID_FORMAT_BITPLANE4)
      return -1;

   // Lazily loads DMD capture until we have a DMD and its format
   if (m_dmd == nullptr)
   {
      m_dmd = std::make_unique<PUPDMD::DMD>();
      m_dmd->SetLogCallback(
         [](const char* format, va_list args, const void* userData)
         {
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);
            LOGD(buffer);
         },
         this);
      m_dmdTriggerDataLoaded = m_dmd->Load(PluginStrings::PathToNative(m_szPath).c_str(), "", src.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE2 ? 2 : 4);
      memset(m_idFrame.data(), 0, m_idFrame.size());
   }

   // Being called at all means nobody else is identifying frames for this game:
   // the event stream skips this call entirely while Serum does, and Serum
   // claims the game before it starts reading its colorization, so the question
   // is settled before the first frame gets here rather than however long a
   // load takes. So if the pack has triggers that only a frame match can fire
   // and there is no local trigger data to match against, those triggers never
   // fire, and the pack plays with parts of it silently missing.
   //
   // Worth a message because the failure is otherwise invisible: the pack
   // loads, the screens appear, and only the media behind the missing triggers
   // is absent. A pack built against a Serum colorization is the usual way into
   // this, its author having had no way to declare the dependency -- the PuP
   // editor only knows "D12345" strings.
   if (!m_dmdTriggerDataLoaded && m_dmdTriggerCount > 0 && !m_reportedMissingIdentification)
   {
      m_reportedMissingIdentification = true;
      LOGE(std::format("This PuP pack has {} DMD trigger(s) but no trigger data of its own, and no Serum colorization is identifying frames for '{}'. "
                       "Those triggers cannot fire. Install the colorization the pack was built against, or a pupdmd trigger set for it.",
         m_dmdTriggerCount, m_szRomName));
   }

   if (src.width == 128 && src.height == 32)
      return static_cast<int>(m_dmd->MatchIndexed(frame, 128, 32));

   // Reproduce legacy behavior for backward compatibility (scaling, padding, coloring)
   // This is very hacky and should be replaced by identification against the raw identify frame.
   if (src.width == 128 && src.height == 16)
   {
      memcpy(m_idFrame.data() + 128 * 8, frame, 128 * 16);
      return static_cast<int>(m_dmd->MatchIndexed(m_idFrame.data(), 128, 32));
   }

   if (src.width <= 256 && src.height == 64) // should be 192x64 or 256x64
   {
      // Resize with a triangle filter to mimic what original implementation in Freezy's DmdExt (https://github.com/freezy/dmd-extensions)
      // does, that is to say:
      // - convert from luminance to RGB (with hue = 0, sat = 1)
      // - resize using Windows 8.1 API which in turn uses IWICBitmapScaler with Fant interpolation mode (hence the triangle filter)
      // - convert back from RGB to HSL and send luminance to PinUp
      //
      // Some references regarding Fant scaling:
      // - https://github.com/sarnold/urt/blob/master/tools/fant.c
      // - https://photosauce.net/blog/post/examining-iwicbitmapscaler-and-the-wicbitmapinterpolationmode-values
      //
      // The Baywatch Pup pack was used to validate this (the filter is still a guess since Windows code is not available)
      //
      //
      // This boils down to a 2x2 box filter in this case (256(or lower) x 64 -> 128 x 32), also no border checking needed then
      // (which is also the same thing that PinMAME used internally to drive PinDMDs and the like for 256x64)
      // (for the 192x64 case, only every 2nd pixel was filtered there in order to also output 128x32, while this routine here outputs a centered 96x32 image)
      const unsigned int ofsX = (128 - (src.width / 2)) / 2;
      for (unsigned int y = 0; y < 32; y++)
      {
         for (unsigned int x = 0; x < src.width / 2; x++)
         {
#if 0 // keep this code around, in case at some point a radius greater than 1 is needed
            float lum = 0., sum = 0.;
            constexpr int radius = 1;
            for (int dx = 1 - radius; dx <= radius; dx++)
            {
               const int px = x * 2 + dx;
               for (int dy = 1 - radius; dy <= radius; dy++)
               {
                  const int py = y * 2 + dy;
                  const float weight = radius * radius - fabsf((float)dx - 0.5f) * fabsf((float)dy - 0.5f);
                  if (/*px >= 0 &&*/ static_cast<unsigned int>(px) < src.width //
                     && /*py >= 0 &&*/ static_cast<unsigned int>(py) < src.height) // unsigned int tests include the >= 0 ones
                     lum += static_cast<float>(frame[py * src.width + px]) * weight;
                  sum += weight;
               }
            }
            m_idFrame[y * 128 + ofsX + x] = (uint8_t)(lum / sum + 0.5f);
#else
            const unsigned int px = x * 2;
            const unsigned int py = y * (2 * src.width);
            uint32_t lum = frame[py + px];
            lum += frame[py + px + 1];
            lum += frame[py + px + src.width];
            lum += frame[py + px + src.width + 1];
            m_idFrame[y * 128 + ofsX + x] = (uint8_t)((lum + 2) >> 2);
#endif
         }
      }
      return static_cast<int>(m_dmd->MatchIndexed(m_idFrame.data(), 128, 32));
   }

   return -1;
}

void PUPManager::DuckAllExcept(int percent, int masterScreenNum, bool all)
{
   if (percent == -1)
   {
      Unduck();
      return;
   }
   // PinUP ignores a new duck request while another screen is still the duck source.
   for (const auto& [key, screen] : m_screenMap)
      if (screen->IsDuckMaster())
         return;
   for (const auto& [key, screen] : m_screenMap)
   {
      if (key == masterScreenNum && !all)
         screen->SetDuckMaster(true);
      else
         screen->Duck(percent);
   }
}

void PUPManager::Unduck()
{
   for (const auto& [key, screen] : m_screenMap)
   {
      if (screen->IsDuckMaster())
         screen->SetDuckMaster(false);
      else
         screen->Unduck();
   }
}

void PUPManager::SetDefaultVolumeAll(int volume)
{
   for (const auto& [key, screen] : m_screenMap)
      screen->SetDefaultVolume(volume);
}

void PUPManager::QueueDOFEvent(char c, int id, int value)
{
   //LOGD(std::format("DOF Event {}{:03} = {}", c, id, value));

   std::lock_guard lock(m_eventMutex);
   for (const auto& [key, screen] : m_screenMap)
   {
      for (auto& [cmd, triggers] : screen->GetTriggers())
      {
         bool matched = false;
         for (auto& trigger : triggers[0]->GetTriggers())
         {
            if (trigger.m_type == c && trigger.m_number == id)
            {
               trigger.m_value = value;
               matched = true;
            }
         }
         if (matched && triggers[0]->IsTriggered())
         {
            for (const auto& trigger : triggers)
            {
               // Dispatch trigger action on main thread
               m_msgApi->RunOnMainThread(m_endpointId, 0.001, [](void* userData) { static_cast<PUPTrigger*>(userData)->Invoke(); }, trigger);
            }
            // Deviation from PinUP Player, which never clears them: D values are events (B2SData is never reset), so a
            // "D9=1,W5=1" trigger must not refire on a later W5=1. Segment display states (also D) are only resent on change
            for (auto& trigger : triggers[0]->GetTriggers())
            {
               if (trigger.m_type == 'D')
                  trigger.m_value = 0;
            }
         }
      }
   }
}

void PUPManager::CustomFunction(int sn, const string& szMsg)
{
   RSJresource json(szMsg);
   if (!json["FN"s].exists())
      return;
   if (sn == 99)
   {
      for (int i = 0; i < 20; i++)
         if (GetScreen(i))
            CustomFunction(i, szMsg);
      return;
   }
   std::shared_ptr<PUPScreen> pScreen = GetScreen(sn);
   if (!pScreen)
   {
      LOGE(std::format("Screen not found: screenNum={}, szMsg={}", sn, szMsg));
      return;
   }
   const int fn = json["FN"s].as<int>();
   switch (fn) {
         case 1:
            // show screen { "mt":301, "SN": XX, "FN":1 }
            pScreen->SetHidden(false);
            break;
         case 18:
            // reset to ForceBack { "mt":301, "SN": XX, "FN":18 }
            pScreen->SetTopmost(false);
            pScreen->SetMode(PUPScreen::Mode::ForceBack);
            break;
         case 20:
         {
            // set screen alpha { "mt":301, "SN": XX, "FN":20, "AM":1, "AV":255 }
            const int am = json["AM"s].as<int>(0);
            pScreen->m_screenAlpha = am ? static_cast<float>(json["AV"s].as<int>(255)) / 255.f : 1.f;
            break;
         }
         case 27:
            // show label page { "mt":301, "SN": XX, "FN":27, "PN":1, "HS":3, "EX":"" }
         {
            const int seconds = json["HS"s].as<int>(0);
            pScreen->SetPage(json["PN"s].as<int>(0), seconds >= 501 ? seconds : seconds * 1000, json["EX"s].as_str());
            break;
         }
         case 28:
            // hide screen { "mt":301, "SN": XX, "FN":28 }
            pScreen->SetHidden(true);
            break;
         case 40:
            // set volume { "mt":301, "SN": XX, "FN":40, "VL":9 }
            pScreen->SetVolumeCurrent(json["VL"s].as<int>(0));
            break;
         case 54:
            // default duck percent for '-xx' file names { "mt":301, "SN": XX, "FN":54, "DV":50 }
            pScreen->SetDefaultDuck(json["DV"s].as<int>(-1));
            break;
         case 2:
            // See pDisableLoopRefresh — form state flags (FF/FO), controls Windows form redraw
            // { "mt":301, "SN": XX, "FN": 2, "FF":0/1, "FO":0/1 }
            NOT_IMPLEMENTED(std::format("Loop refresh flags not implemented: screen={{{}}}, FF={}, FO={}", pScreen->ToString(false), json["FF"s].as<int>(0), json["FO"s].as<int>(0)));
            break;
         case 3:
            // Show/hide the label overlay window (every label of the screen)
            // { "mt":301, "SN": XX, "FN":3, "OT": 0 } - OT 0/1
            pScreen->m_hudVisible = (json["OT"s].as<int>(0) != 0);
            break;
         case 4:
         {
            // set StayOnTop { "mt":301, "SN": XX, "FN":4, "FS":1/0 }
            // FS=1 pins the screen above its siblings (ForceOn mode + topmost flag);
            // FS=0 releases it to render under other screens (ForceBack).
            const bool topmost = (json["FS"s].exists() && json["FS"s].as<int>() == 1);
            LOGD(std::format("Stay on top requested: screen={{{}}}, fn={}, topmost={}, szMsg={}", pScreen->ToString(false), fn, topmost, szMsg));
            pScreen->SetTopmost(topmost);
            pScreen->SetMode(topmost ? PUPScreen::Mode::ForceOn : PUPScreen::Mode::ForceBack);
            break;
         }
         case 5:
            // Send this screen to the back of its siblings (PinUP: TControl.SendToBack).
            // { "mt":301, "SN": XX, "FN":5 }
            LOGD(std::format("Send screen to back requested: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            SendScreenToBack(pScreen.get());
            break;
         case 6:
            // Bring this screen to the front of its siblings (PinUP: TControl.BringToFront, no topmost change).
            // { "mt":301, "SN": XX, "FN":6 }
            LOGD(std::format("Bring screen to front requested: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            SendScreenToFront(pScreen.get());
            break;
         case 10:
            // set all displays all volume { "mt":301, "SN": XX, "FN":10, "VL":9}  VL=volume level
            SetDefaultVolumeAll(json["VL"s].as<int>(0));
            break;
         case 11:
            // set all volume { "mt":301, "SN": XX, "FN":11, "VL":9}  VL=volume level
            LOGD(std::format("Set all volume requested: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            pScreen->SetVolumeCurrent(json["VL"s].as<int>(0));
            break;
         case 12:
            // "{ ""mt"":301, ""SN"": 18, ""FN"":12 }" 'STOPSCREEN
            pScreen->SetMode(PUPScreen::Mode::Off);
            pScreen->Stop();
            break;
         case 15:
            // set screen custompos { 'mt':301, 'SN':15,'FN':15,'CP':'parent_screen,x,y,w,h'} CP = CustomPos String, coordinates relative in %
            LOGD(std::format("Set screen custompos requested: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            pScreen->SetCustomPos(json["CP"s].as_str());
            break;
         case 16:
            // start executable
            //   { ""mt"":301, ""SN"": 2, ""FN"":16, ""EX"": """&PuPMiniGameExe  &""", ""WT"": """&PuPMiniGameTitle&""", ""RS"":1 , ""TO"":15 , ""WZ"":0 , ""SH"": 1 , ""FT"":""Visual Pinball Player"" }
            //   { ""mt"":301, ""SN"": 2, ""FN"":16, ""EX"": ""Pupinit.bat"", ""WT"": """", ""RS"":1 , ""TO"":15 , ""WZ"":0 , ""SH"": 1 , ""FT"":""Visual Pinball Player"" }
            // EX: Windows executable / WT: Window Title / WZ: Window Z order / RS: ? / TO: ? / SH: ? / FT: ?
            NOT_IMPLEMENTED("Executing OS executable is not implemented. szMsg=" + szMsg);
            break;
         case 17:
            // set window z order { ""mt"":301, ""SN"": ""2"", ""FN"":17, ""WT"":""Visual Pinball Player"", ""WZ"": 1, ""WP"": 1 }
            // See https://learn.microsoft.com/fr-fr/windows/win32/api/winuser/nf-winuser-setwindowpos
            // WT: Window Title / WZ: hWndInsertAfter / WP: uFlags
            NOT_IMPLEMENTED("Set OS window z ordering is not implemented. szMsg=" + szMsg);
            break;
         case 22:
         {
            // set overlay alpha { "mt":301, "SN": 16, "FN":22, "AM":1, "AV":255 }
            // AV: Alpha Value (0-255), AM: Alpha mode enabled (1) or disabled (0)
            const int am = json["AM"s].as<int>(0);
            pScreen->m_overlayAlpha = am ? static_cast<float>(json["AV"s].as<int>(255)) / 255.f : 1.f;
            break;
         }
         case 26:
            // set screen volume multiplier { "mt":301, "SN": XX, "FN":26, "VL":100 }
            pScreen->SetVolumeMultiplier(json["VL"s].as<int>(0));
            break;
         case 30:
            // {'mt':301, 'SN': XX, 'FN':30, 'PM':1 } set (play ?) jukebox mode: jukebox mode will auto advance to next media in playlist and you can use next/prior sub to manuall advance
            NOT_IMPLEMENTED(std::format("Jukebox mode not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 31:
            // pup jukebox control - {'mt':301, 'SN': XX, 'FN':31, 'PM':1 } - PM 1 = next, PM 2 = previous
            NOT_IMPLEMENTED(std::format("Jukebox mode not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 32:
            // "{ ""mt"":301, ""SN"": 1, ""FN"":32, ""FQ"":3 }"   'set no antialias on font render if real
            NOT_IMPLEMENTED(std::format("Font quality is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 33:
            // "{ ""mt"":301, ""SN"": 1, ""FN"":33 }"             'set pupdmd for mirror and hide behind other pups
            // TODO expose the DMD screen to other pluginsthrough the generic controllar plugin API
            NOT_IMPLEMENTED(std::format("DMD mirroring is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 34:
            // "{ ""mt"":301, ""SN"": "& pDisp &", ""FN"": 34 }"             'hideoverlay text during next videoplay on DMD auto return
            // Labels are hidden and LabelSet/LabelShowPage ignored until the current media ends.
            pScreen->m_hideDuringPlay = true;
            pScreen->m_hudVisible = false;
            break;
         case 41:
            // 'set safeloop mode on current playing media.  Good for background videos that refresh often?  { "mt":301, "SN": XX, "FN":41 }
            NOT_IMPLEMENTED(std::format("Safe loop mode not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 42:
         {
            // See PUPDMD AudioDuckPuP — temporarily duck all screens except master
            // "{ ""mt"":301, ""SN"": "& MasterPuPID& ", ""FN"": 42, ""DV"": "&VolLevel&" , ""ALL"":1 }"
            const int duckPercent = json["DV"s].as<int>(0);
            DuckAllExcept(duckPercent == 0 ? 1 : duckPercent, sn, json["ALL"s].as<int>(0) == 1);
            break;
         }
         case 45:
            // slow pc mode { "mt":301, "SN":XX, "FN":45, "SP":1 } - SP 0/1 = slow pc mode bool
            NOT_IMPLEMENTED(std::format("Slow PC mode is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 46:
            // See pDMDAlwaysPAD — when enabled, treat leading/trailing whitespace in label captions
            // as shadow-clearance padding rather than rendering it as advance-width glyphs.
            // { "mt":301, "SN": XX, "FN":46, "PA":1 } - PA 0/1 = pad text bool
            pScreen->m_padTextAlways = (json["PA"s].as<int>(0) != 0);
            break;
         case 50:
            // pSetAspectRatio(PuPID, arWidth, arHeight) "{ ""mt"":301, ""SN"": "&PuPID& ", ""FN"": 50, ""WIDTH"": "&arWidth&", ""HEIGHT"": "&arHeight&" }"   
            NOT_IMPLEMENTED(std::format("Set aspect ratio is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 51:
            // set media play position in ms { "mt":301, "SN": XX, "FN":51, "SP": 3431} - SP position in ms
            NOT_IMPLEMENTED(std::format("Set precise media position is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 52:
            // pDMDSetTextQuality(AALevel)  '0 to 4 aa.  4 is sloooooower.  default 1,  perhaps use 2-3 if small desktop view.  only affect text quality.  can set per label too with 'qual' settings.
            // "{ ""mt"":301, ""SN"": 5, ""FN"":52, ""SC"": "& AALevel &" }"    'slow pc mode
            NOT_IMPLEMENTED(std::format("Font antialiasing level is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 53:
            // Experimental frame rescale, FORCE higher frame size to autosize and rescale nicer,  like AA and auto-fit.
            // "{ ""mt"":301, ""SN"": "&PuPID& ", ""FN"": 53, ""XW"": "&fWidth&", ""YH"": "&fHeight&", ""FR"":1 }"
            NOT_IMPLEMENTED(std::format("Experimental frame rescale is not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         case 55:
            // Set media transition fade step, read from "FS" field.
            // { "mt":301, "SN": XX, "FN":55, "FS":<0..255> }
            //
            // Exposed in the Matrix tables via two helpers:
            //   Sub pTransitionFadeOff(PupID)  ' disable transition fade
            //       PuPlayer.SendMSG "{...""FN"":55,""FS"":255}"
            //   Sub pTransitionFadeON(PupID)   ' re-enable transition fade
            //       PuPlayer.SendMSG "{...""FN"":55,""FS"":10}"
            if (json["FS"s].exists())
               pScreen->SetFadeStep(json["FS"s].as<int>());
            break;
         case 57:
            // "Overlay Hide" (OH) toggle for a screen that has text/label overlays.
            // PinUp Player 2.0+ only; older players ignore it.
            // { "mt":301, "SN": XX, "FN":57, "OH":0/1 } - OH 1 = hide overlay, 0 = show
            // We don't act on it yet, so sending it produces no visible change on screen.
            // TODO implement overlay hide/show toggling the screen's overlay (HUD) visibility
            LOGW(std::format("Overlay hide (OH) toggle not implemented, ignoring: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
         default:
            NOT_IMPLEMENTED(std::format("Unknown function not implemented: screen={{{}}}, fn={}, szMsg={}", pScreen->ToString(false), fn, szMsg));
            break;
   }
}

void PUPManager::SendMsg(const string& szMsg)
{
   RSJresource json(szMsg);
   if (json["mt"s].exists()) {
      int mt = json["mt"s].as<int>();
      switch(mt) {
         case 301:
            // SN 99 applies the function to every screen
            if (json["SN"s].exists())
               CustomFunction(json["SN"s].as<int>(), szMsg);
            else
               NOT_IMPLEMENTED(std::format("Not implemented: mt={}, szMsg={}", mt, szMsg));
            break;
         case 402:
            // PuPSound { "mt":402, "SN":XX, "ns":"name.wav", "sX":0, "sY":0, "sZ":0, "vol":100, "sp":"" }
            PlaySound(json["ns"s].as_str(), json["sX"s].as<int>(0), json["sY"s].as<int>(0), json["sZ"s].as<int>(0), json["vol"s].as<int>(100), json["sp"s].as_str());
            break;
         case 400:
         case 401:
            // SoundAdd / SoundPlay: ignored by PinUP Player
            break;
         default:
            NOT_IMPLEMENTED(std::format("Not implemented: mt={}, szMsg={}", mt, szMsg));
            break;
      }
   }
   else {
      LOGE("No message type found: szMsg=" + szMsg);
   }

   return;
}

void PUPManager::RunLater(double seconds, std::function<void()> fn)
{
   struct Pending
   {
      std::weak_ptr<int> token;
      std::function<void()> fn;
   };
   auto* pending = new Pending { m_runToken, std::move(fn) };
   m_msgApi->RunOnMainThread(m_endpointId, seconds, [](void* userData)
   {
      std::unique_ptr<Pending> p(static_cast<Pending*>(userData));
      if (!p->token.expired())
         p->fn();
   }, pending);
}

int PUPManager::Render(VPXRenderContext2D* const renderCtx, void* context)
{
   auto me = static_cast<PUPManager*>(context);

   if (float volume = pupMainVolume_Get(); volume != me->m_mainVolume)
   {
      me->m_mainVolume = volume;
      for (const auto& [key, screen] : me->m_screenMap)
         screen->SetMainVolume(volume);
   }
   // A usable root must exist, must be a canvas rather than a positioned element (a childless
   // screen positioned on another screen; a screen whose custom position references itself has
   // no parent and stays a canvas), and its screen tree must contain something that can render.
   // Ancillary window renderers are exclusive (the first one that renders claims the window), so
   // claiming a window for a dead tree would blank it and starve lower priority renderers (e.g.
   // the B2S backglass behind an Off backglass screen 2). Skipping dead candidates also routes
   // content to the fallback screen (e.g. videos on DMD screen 1 when screen 5 is Off).
   auto isUsableRoot = [me](const std::shared_ptr<PUPScreen>& root) -> bool
   {
      if (root == nullptr || (root->GetParent() != nullptr && !root->HasChildren()))
         return false;
      for (const auto& screen : me->m_screenOrder)
      {
         if (screen->GetMode() == PUPScreen::Mode::Off || screen->GetMode() == PUPScreen::Mode::MusicOnly || screen->IsHidden())
            continue;
         const PUPScreen* parent = screen.get();
         while (parent && parent != root.get())
            parent = parent->GetParent();
         if (parent)
            return true;
      }
      return false;
   };

   int padLeft = 0;
   int padRight = 0;
   int padTop = 0;
   int padBottom = 0;
   int displayTarget = -1; // Target index of the authored displays: 0 = backglass, 1 = score view, 2 = topper
   std::shared_ptr<PUPScreen> rootScreen = nullptr;
   switch (renderCtx->window)
   {
   case VPXWindowId::VPXWINDOW_Topper:
      rootScreen = me->GetScreen(0);
      displayTarget = 2;
      padLeft = pupTopperPadLeft_Get();
      padRight = pupTopperPadRight_Get();
      padTop = pupTopperPadTop_Get();
      padBottom = pupTopperPadBottom_Get();
      break;
   case VPXWindowId::VPXWINDOW_Backglass:
      rootScreen = me->GetScreen(2);
      if (!isUsableRoot(rootScreen))
         rootScreen = me->GetScreen(6);
      displayTarget = 0;
      padLeft = pupBGPadLeft_Get();
      padRight = pupBGPadRight_Get();
      padTop = pupBGPadTop_Get();
      padBottom = pupBGPadBottom_Get();
      break;
   case VPXWindowId::VPXWINDOW_ScoreView:
      rootScreen = me->GetScreen(5);
      if (!isUsableRoot(rootScreen))
         rootScreen = me->GetScreen(1);
      displayTarget = 1;
      padLeft = pupSVPadLeft_Get();
      padRight = pupSVPadRight_Get();
      padTop = pupSVPadTop_Get();
      padBottom = pupSVPadBottom_Get();
      break;
   default: break;
   }

   renderCtx->srcWidth = renderCtx->outWidth;
   renderCtx->srcHeight = renderCtx->outHeight;

   // Authored displays still render (and claim the window) even without a usable screen tree
   if (!isUsableRoot(rootScreen))
      return displayTarget >= 0 && me->RenderDisplays(renderCtx, displayTarget) > 0;

   if (!LibAV::LibAV::GetInstance().isLoaded)
      return false;
   rootScreen->SetBounds(padLeft, padTop, static_cast<int>(renderCtx->srcWidth) - padLeft - padRight, static_cast<int>(renderCtx->srcHeight) - padTop - padBottom);

   // Render all children of rootScreen according to the following render order:
   // - Back screens (ForceBack or SetAsBackground)
   //   0. underlay
   //   1. video
   //   2. overlay
   // - Front (others)
   //   0. underlay
   //   1. video
   //   2. overlay
   // - active label page (not sure if back/front apply to label pages)
   vector<std::shared_ptr<PUPScreen>> screens;
   for (const auto& screen : me->m_screenOrder)
   {
      const PUPScreen* parent = screen.get();
      while (parent && parent != rootScreen.get())
         parent = parent->GetParent();
      if (parent)
         screens.push_back(screen);
   }
   // Render order - two tiers (non-topmost, topmost) mirroring Win32 HWND_TOPMOST behavior.
   // In the back (non-topmost) tier the popup video is drawn before the non-popup overlay so that
   // a full-window frame/overlay (e.g. a decorative backglass border) stays above the background
   // videos. In the front (topmost) tier popups are drawn last so callouts stay on top.
   auto renderScreens = [&renderCtx, &screens](bool popup, bool topmost, int startPass, int endPass)
   {
      for (int pass = startPass; pass <= endPass; pass++)
         std::ranges::for_each(screens,
            [&renderCtx, pass, popup, topmost](const auto& screen)
            {
               if (screen->IsPop() == popup && screen->IsTopmost() == topmost && screen->GetMode() != PUPScreen::Mode::Off)
                  screen->Render(renderCtx, pass);
            });
   };

   renderScreens(false, false, 0, 1);
   renderScreens(true, false, 0, 3);
   renderScreens(false, false, 2, 3);
   renderScreens(false, true, 0, 1);
   renderScreens(false, true, 2, 3);
   renderScreens(true, true, 0, 3);

   // Authored displays render on top of the screen tree
   if (displayTarget >= 0)
      me->RenderDisplays(renderCtx, displayTarget);

   // Set Game time after rendering to avoid updating while rendering if the decode thread are waiting for it
   if (me->m_vpxApi)
   {
      double gameTime = me->m_vpxApi->GetGameTime();
      for (const auto& [key, screen] : me->m_screenMap)
         screen->SetGameTime(gameTime);
   }

   return true;
}

void PUPManager::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   auto me = static_cast<PUPManager*>(context);
   auto msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   static constexpr AncillaryRendererDef entry = { "PUP", "PinUp Player", "Renderer for PinUp Player backglass", nullptr, Render };
   if (msg->window == VPXWindowId::VPXWINDOW_Backglass || msg->window == VPXWindowId::VPXWINDOW_ScoreView || msg->window == VPXWindowId::VPXWINDOW_Topper)
   {
      if (msg->count < msg->maxEntryCount) 
      {
         msg->entries[msg->count] = entry;
         msg->entries[msg->count].context = me;
      }
      msg->count++;
   }
}

void PUPManager::OnGetAudioSrc(const unsigned int eventId, void* context, void* msgData)
{
   auto me = static_cast<PUPManager*>(context);
   auto msg = static_cast<GetAudioSrcMsg*>(msgData);
   if (msg->count < msg->maxEntryCount) 
      msg->entries[msg->count] = me->m_audioSrcDef;
   msg->count++;
}

const string& PlayActionToString(PlayAction value)
{
   static const string actionStrings[] = { "PlayAction::Normal"s, "PlayAction::Loop"s, "PlayAction::SplashReset"s, "PlayAction::SplashReturn"s, "PlayAction::StopPlayer"s,
      "PlayAction::StopFile"s, "PlayAction::SetBG"s, "PlayAction::PlaySSF"s, "PlayAction::SkipSamePriority"s, "PlayAction::CustomFunction"s, "Unknown"s,
      "PlayAction::QueueSameP"s, "PlayAction::QueueAlways"s };
   static const string error = "Unknown"s;
   if ((int)value < 0 || (size_t)value >= std::size(actionStrings))
      return error;
   return actionStrings[(int)value];
}

}
