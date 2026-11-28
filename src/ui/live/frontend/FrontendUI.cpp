// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendUI.h"

#include "core/player.h"
#include "core/Settings.h"
#include "core/SettingsService.h"
#include "core/extern.h"
#include "core/FileLocator.h"
#include "core/VPApp.h"
#include "imgui/imgui_internal.h"
#include "parts/pintable.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "renderer/VRDevice.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/ingameui/InGameUIWidgets.h"
#include "ui/live/ingameui/LoadingPage.h"
#include "ui/live/LiveUI.h"
#include "ui/VPXFileFeedback.h"

#include <fstream>


namespace VPX::Frontend
{

static uint32_t FrontendTimeMs() { return static_cast<uint32_t>(SDL_GetTicks()); }


FrontendUI::FrontendUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
{
}

FrontendUI::~FrontendUI()
{
   // An in-flight asynchronous table load must not keep a stale owner: once flagged abandoned the
   // worker releases the loaded table itself, and a completed result is released right away
   if (m_tableLoad)
      if (PinTable *const table = (m_tableLoad->abandoned.store(true), m_tableLoad->table.exchange(nullptr)))
         table->Release();
   if (m_catalogScan)
      m_catalogScan->abandoned.store(true, std::memory_order_release);
   if (m_auxSubscribed)
   {
      m_msgApi->UnsubscribeMsg(m_auxRendererMsgId, OnGetAuxRenderer, this);
      m_msgApi->BroadcastMsg(m_endpointId, m_auxRendererChgMsgId, nullptr); // Let the next session recollect renderers without us
      m_msgApi->ReleaseMsgID(m_auxRendererMsgId);
      m_msgApi->ReleaseMsgID(m_auxRendererChgMsgId);
   }
   if (m_vpxApi != nullptr)
   {
      for (const auto &[path, texture] : m_auxTextures)
         m_vpxApi->DeleteTexture(texture);
      for (const auto &[path, video] : m_auxVideos)
         m_vpxApi->DeleteTexture(video.texture);
   }
}

void FrontendUI::SetupAuxRenderer()
{
   if (m_auxSubscribed)
      return;
   m_msgApi = &m_player->m_pluginManager.GetMsgAPI();
   m_vpxApi = &m_player->m_pluginAPI.getAPI();
   m_endpointId = m_player->m_pluginAPI.GetVPXEndPointId();
   m_auxRendererMsgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER);
   m_auxRendererChgMsgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG);
   m_msgApi->SubscribeMsg(m_endpointId, m_auxRendererMsgId, OnGetAuxRenderer, this);
   // The initial renderer collection may have run before we subscribed: ask for a recollect
   m_msgApi->BroadcastMsg(m_endpointId, m_auxRendererChgMsgId, nullptr);
   m_auxSubscribed = true;
}

void FrontendUI::OnGetAuxRenderer(const unsigned int msgId, void *userData, void *msgData)
{
   auto *msg = static_cast<GetAncillaryRendererMsg *>(msgData);
   if (msg->window != VPXWindowId::VPXWINDOW_Backglass && msg->window != VPXWindowId::VPXWINDOW_ScoreView && msg->window != VPXWindowId::VPXWINDOW_Topper)
      return;
   AncillaryRendererDef entry { "VPX.Frontend", "Frontend", "Mini frontend table media", userData, &FrontendUI::RenderAncillary };
   if (msg->count < msg->maxEntryCount)
      msg->entries[msg->count] = entry;
   msg->count++;
}

int MSGPIAPI FrontendUI::RenderAncillary(VPXRenderContext2D *ctx, void *context)
{
   auto *me = static_cast<FrontendUI *>(context);
   const TableCatalog::Entry *const selected = me->GetSelected();
   if (selected == nullptr)
      return 0;
   const TableCatalog::Entry &entry = *selected;
   VPXTexture texture = nullptr;
   switch (ctx->window)
   {
   case VPXWindowId::VPXWINDOW_Backglass:
      if (entry.HasMedia(TableCatalog::MediaType::Backglass))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::Backglass));
      break;
   case VPXWindowId::VPXWINDOW_ScoreView:
      if (entry.HasMedia(TableCatalog::MediaType::FullDMD))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::FullDMD));
      else if (entry.HasMedia(TableCatalog::MediaType::DMD))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::DMD));
      break;
   case VPXWindowId::VPXWINDOW_Topper:
      if (entry.HasMedia(TableCatalog::MediaType::Topper))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::Topper));
      break;
   default: break;
   }
   if (texture == nullptr)
      return 0;
   VPXTextureInfo *const info = me->m_vpxApi->GetTextureInfo(texture);
   if (info == nullptr)
      return 0;
   ctx->DrawImage(ctx, texture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(info->width), static_cast<float>(info->height), 0.f, 0.f, 0.f, 0.f, 0.f, ctx->srcWidth, ctx->srcHeight);
   return 1;
}

VPXTexture FrontendUI::GetAuxMedia(const std::filesystem::path &path) { return m_media != nullptr && m_media->IsVideoBacked(path) ? GetAuxVideoTexture(path) : GetAuxTexture(path); }

// Video medias on ancillary windows: the decode session publishes its latest frame as a bitmap
// which is copied into the window's texture; the sequence check skips unchanged uploads
VPXTexture FrontendUI::GetAuxVideoTexture(const std::filesystem::path &path)
{
   if (m_media == nullptr)
      return nullptr;
   uint64_t seq = 0;
   const std::shared_ptr<const BaseTexture> frame = m_media->GetVideoFrame(path, &seq);
   if (frame == nullptr)
      return nullptr;
   std::lock_guard lock(m_auxTexMutex);
   AuxVideo &slot = m_auxVideos[path];
   if (slot.seq != seq)
   {
      m_vpxApi->UpdateTexture(&slot.texture, static_cast<int>(frame->width()), static_cast<int>(frame->height()), VPXTextureFormat::VPXTEXFMT_sRGBA8, frame->datac());
      slot.seq = seq;
   }
   return slot.texture;
}

VPXTexture FrontendUI::GetAuxTexture(const std::filesystem::path &path)
{
   std::lock_guard lock(m_auxTexMutex);
   if (const auto it = m_auxTextures.find(path); it != m_auxTextures.end())
      return it->second;
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   if (!file.is_open())
      return nullptr;
   const std::streamsize size = file.tellg();
   vector<uint8_t> data(static_cast<size_t>(size));
   file.seekg(0);
   if (!file.read(reinterpret_cast<char *>(data.data()), size))
      return nullptr;
   VPXTexture texture = m_vpxApi->CreateTexture(data.data(), static_cast<int>(size));
   return m_auxTextures.emplace(path, texture).first->second;
}

// The table played or edited by the previous session: a fresh frontend session (each
// session has its own FrontendUI instance) puts the wheel back on it when it rescans
static std::filesystem::path s_resumeTablePath;

// Recursive folder watcher used to keep the catalog synchronized while the frontend runs. Windows
// uses the OS change notifications; other platforms fall back to a low-frequency scan trigger.
class CatalogFolderWatcher
{
public:
   ~CatalogFolderWatcher() { Stop(); }

   void Start(const vector<std::filesystem::path> &roots)
   {
      vector<string> key;
      for (const std::filesystem::path &root : roots)
         key.push_back(PathToUTF8(root));
      std::ranges::sort(key);
      if (m_key == key)
         return;
      Stop();
      m_key = std::move(key);
      m_lastEventMs.store(0, std::memory_order_release);
      if (roots.empty())
         return;

#ifdef _WIN32
      m_usePolling = false;
      for (const std::filesystem::path &root : roots)
      {
         std::error_code ec;
         if (!std::filesystem::is_directory(root, ec))
            continue;
         HANDLE handle
            = FindFirstChangeNotificationW(root.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE);
         if (handle != INVALID_HANDLE_VALUE)
            m_handles.push_back(handle);
      }
      if (m_handles.size() > MAXIMUM_WAIT_OBJECTS - 1)
      {
         PLOGW << "Too many frontend table roots to watch them all; using polling";
         for (HANDLE handle : m_handles)
            FindCloseChangeNotification(handle);
         m_handles.clear();
         m_usePolling = true;
      }
      if (m_handles.empty())
         m_usePolling = true;
#else
      m_usePolling = true;
#endif

      m_stop.store(false, std::memory_order_release);
      m_thread = std::thread(
         [this]()
         {
#ifdef _WIN32
            if (!m_usePolling)
            {
               while (!m_stop.load(std::memory_order_acquire))
               {
                  const DWORD result = WaitForMultipleObjects(static_cast<DWORD>(m_handles.size()), m_handles.data(), FALSE, 500);
                  if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + m_handles.size())
                  {
                     m_lastEventMs.store(FrontendTimeMs(), std::memory_order_release);
                     FindNextChangeNotification(m_handles[result - WAIT_OBJECT_0]);
                  }
                  else if (result == WAIT_FAILED)
                  {
                     PLOGW << "Frontend table folder watcher stopped after a Windows wait failure";
                     break;
                  }
               }
               for (HANDLE handle : m_handles)
                  FindCloseChangeNotification(handle);
               return;
            }
#endif
            // Portable fallback: periodically request a scan. Scanning is incremental and reuses
            // validated cached details, but folder enumeration can still be expensive on very large
            // libraries, so this low-frequency safety net must not scan continuously.
            while (!m_stop.load(std::memory_order_acquire))
            {
               for (int i = 0; i < 3000 && !m_stop.load(std::memory_order_acquire); ++i)
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
               if (!m_stop.load(std::memory_order_acquire))
                  m_lastEventMs.store(FrontendTimeMs(), std::memory_order_release);
            }
         });
   }

   uint32_t LastEventMs() const { return m_lastEventMs.load(std::memory_order_acquire); }

private:
   void Stop()
   {
      m_stop.store(true, std::memory_order_release);
      if (m_thread.joinable())
         m_thread.join();
#ifdef _WIN32
      // Handles are closed by the worker after its wait loop exits.
      m_handles.clear();
#endif
      m_usePolling = false;
   }

   vector<string> m_key;
   std::thread m_thread;
   std::atomic<bool> m_stop { true };
   std::atomic<uint32_t> m_lastEventMs { 0 };
   bool m_usePolling = false;
#ifdef _WIN32
   vector<HANDLE> m_handles;
#endif
};

static std::filesystem::path GetCatalogCachePath() { return g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Preferences) / "table-catalog.cache"s; }

struct CatalogScanAborted
{
};

void FrontendUI::Rescan()
{
   vector<std::filesystem::path> roots;
   const string &folders = g_settingsService.GetAppSettings().GetFrontend_TableFolders();
   size_t start = 0;
   while (start <= folders.size())
   {
      const size_t end = folders.find(';', start);
      const string folder = folders.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
         roots.emplace_back(folder);
      if (end == string::npos)
         break;
      start = end + 1;
   }

   UpdateCatalogWatcher(roots);

   if (s_resumeTablePath.empty())
   {
      const string &lastTable = g_settingsService.GetAppSettings().GetFrontend_LastTable();
      if (!lastTable.empty())
         s_resumeTablePath = PathFromUTF8(lastTable);
   }

   // The cache is only a startup snapshot: it is shown immediately, then reconciled by this scan.
   if (!m_catalogCacheLoadAttempted)
   {
      m_catalogCacheLoadAttempted = true;
      TableCatalog cached;
      if (cached.LoadCacheFile(GetCatalogCachePath(), roots))
         AdoptCatalog(std::move(cached), true);
   }

   if (m_catalogScan)
      m_catalogScan->abandoned.store(true, std::memory_order_release);
   m_catalogScanStartedAt = FrontendTimeMs();
   const std::shared_ptr<PendingCatalogScan> scan = std::make_shared<PendingCatalogScan>();
   scan->initial = m_catalog;
   m_catalogScan = scan;
   m_scanned = true;
   const std::filesystem::path cachePath = GetCatalogCachePath();
   const bool discardStaleCache = g_settingsService.GetAppSettings().GetFrontend_CacheDiscardStale();
   constexpr std::array<int, 8> cacheLimits { 0, 1, 2, 5, 10, 25, 50, -1 };
   const int textureCacheLimit = cacheLimits[std::clamp(g_settingsService.GetAppSettings().GetFrontend_TextureCacheLimit(), 0, static_cast<int>(cacheLimits.size()) - 1)];
   std::thread(
      [scan, roots = std::move(roots), cachePath, discardStaleCache, textureCacheLimit]() mutable
      {
         const auto pushUpdate = [scan](CatalogScanUpdate::Type type, vector<TableCatalog::Entry> entries)
         {
            if (entries.empty() && type == CatalogScanUpdate::Type::Upsert)
               return;
            std::lock_guard lock(scan->updateMutex);
            scan->updates.push_back({ type, std::move(entries) });
         };
         try
         {
            TableCatalog scanned;
            vector<std::filesystem::path> cacheFolders;
            vector<TableCatalog::Entry> batch;
            size_t published = 0;
            auto lastPublish = std::chrono::steady_clock::now();
            const auto flushBatch = [&](const bool force)
            {
               const auto now = std::chrono::steady_clock::now();
               if (!batch.empty() && (force || batch.size() >= 50 || now - lastPublish >= std::chrono::milliseconds(200)))
               {
                  pushUpdate(CatalogScanUpdate::Type::Upsert, batch);
                  batch.clear();
                  lastPublish = now;
               }
            };
            scanned.ScanPaths(
               roots,
               [&](const size_t count)
               {
                  if (scan->abandoned.load(std::memory_order_acquire))
                     throw CatalogScanAborted {};
                  if (count > published)
                  {
                     const auto &entries = scanned.GetEntries();
                     batch.insert(batch.end(), entries.begin() + static_cast<ptrdiff_t>(published), entries.begin() + static_cast<ptrdiff_t>(count));
                     published = count;
                  }
                  flushBatch(false);
               },
               &cacheFolders);
            flushBatch(true);

            // The complete path pass is the only point where entries missing from the filesystem
            // are removed. Upserts before it are additive, keeping the cached catalog usable.
            pushUpdate(CatalogScanUpdate::Type::Reconcile, scanned.GetEntries());

            TableCatalog working = scan->initial;
            working.ReconcileEntries(scanned.GetEntries());
            vector<TableCatalog::Entry> detailsBatch;
            lastPublish = std::chrono::steady_clock::now();
            const auto flushDetails = [&](const bool force)
            {
               const auto now = std::chrono::steady_clock::now();
               if (!detailsBatch.empty() && (force || detailsBatch.size() >= 25 || now - lastPublish >= std::chrono::milliseconds(200)))
               {
                  pushUpdate(CatalogScanUpdate::Type::Upsert, detailsBatch);
                  detailsBatch.clear();
                  lastPublish = now;
               }
            };
            for (size_t i = 0; i < working.GetEntries().size(); ++i)
            {
               if (scan->abandoned.load(std::memory_order_acquire))
                  throw CatalogScanAborted {};
               if (!working.GetEntries()[i].detailsLoaded)
               {
                  working.EnsureEntryDetails(i);
                  detailsBatch.push_back(working.GetEntries()[i]);
                  flushDetails(false);
               }
            }
            flushDetails(true);
            working.LoadAllDetails(); // Entries are already loaded: this only restores title ordering
            const TableCatalog::TextureCacheCleanupResult cleanup = TableCatalog::CleanupTextureCaches(roots, working.GetEntries(), discardStaleCache, textureCacheLimit, &cacheFolders);
            if (cleanup.staleRemoved != 0 || cleanup.overflowRemoved != 0 || cleanup.failedRemovals != 0)
	    {
               PLOGI << "Frontend texture cache cleanup removed " << cleanup.staleRemoved << " stale and " << cleanup.overflowRemoved << " overflowing table caches ("
                     << cleanup.failedRemovals << " failures)";
	    }
            // Do not rewrite an unchanged cache: besides being wasted IO for large libraries, a
            // broadly configured watch root could otherwise keep triggering scans recursively.
            std::error_code cacheEc;
            const bool cacheExists = std::filesystem::is_regular_file(cachePath, cacheEc);
            if ((working.GetEntries() != scan->initial.GetEntries() || !cacheExists) && !working.SaveCacheFile(cachePath, roots))
	    {
               PLOGW << "Failed to write the frontend table catalog cache";
	    }
         }
         catch (const CatalogScanAborted &)
         {
         }
         catch (const std::exception &e)
         {
            PLOGE << "Frontend table scan failed: " << e.what();
         }
         scan->detailsReady.store(true, std::memory_order_release);
      })
      .detach();
}

void FrontendUI::UpdateCatalogWatcher(const vector<std::filesystem::path> &roots)
{
   if (m_catalogWatcher == nullptr)
      m_catalogWatcher = std::make_unique<CatalogFolderWatcher>();
   m_catalogWatcher->Start(roots);
}

void FrontendUI::PollCatalogWatcher()
{
   if (m_catalogWatcher == nullptr)
      return;
   const uint32_t eventAt = m_catalogWatcher->LastEventMs();
   if (eventAt != 0 && eventAt != m_catalogWatchEventSeen)
   {
      m_catalogWatchEventSeen = eventAt;
      // Events already preceding the scan start are covered by it; events occurring while it
      // runs may affect a folder it has already enumerated and are debounced for another pass.
      const bool coveredByCurrentScan = m_catalogScanStartedAt != 0 && static_cast<int32_t>(m_catalogScanStartedAt - eventAt) >= 0;
      if (!coveredByCurrentScan)
         m_catalogWatchDirtySince = FrontendTimeMs();
   }
   if (m_catalogWatchDirtySince != 0 && FrontendTimeMs() - m_catalogWatchDirtySince > 1500 && m_catalogScan == nullptr)
   {
      m_catalogWatchDirtySince = 0;
      Rescan();
   }
}

void FrontendUI::PollCatalogScan()
{
   if (!m_catalogScan)
      return;
   // Catalog updates reorder and copy the table vector. Never apply them while the wheel is
   // being navigated: keeping input and animation fluid takes precedence over publishing scan
   // details, which are coalesced and applied as soon as the navigation pauses.
   const uint32_t now = FrontendTimeMs();
   if (m_wheelNavDir != 0 || m_wheelDragging || fabsf(m_wheelVel) > 0.f || (m_lastWheelNavigationAt != 0 && now - m_lastWheelNavigationAt < 250))
      return;
   const std::shared_ptr<PendingCatalogScan> scan = m_catalogScan;
   std::deque<CatalogScanUpdate> updates;
   bool complete = false;
   {
      std::lock_guard lock(scan->updateMutex);
      updates.swap(scan->updates);
      // Once detailsReady is set the worker does not publish anymore, but it may have published
      // after the previous frame drained the queue: only retire the scan with an empty queue.
      complete = scan->detailsReady.load(std::memory_order_acquire) && scan->updates.empty();
   }
   // Path batches published before the complete reconciliation are obsolete as soon as that
   // reconciliation is available: it carries the whole path scan and preserves still valid details.
   if (const auto reconcile = std::ranges::find_if(updates, [](const CatalogScanUpdate &update) { return update.type == CatalogScanUpdate::Type::Reconcile; }); reconcile != updates.end())
      updates.erase(updates.begin(), reconcile);

   // Coalesce adjacent additive batches: applying every small worker batch would reorder the
   // catalog repeatedly on the UI thread, while only the merged result affects the final state.
   std::deque<CatalogScanUpdate> coalesced;
   for (CatalogScanUpdate &update : updates)
   {
      if (update.type == CatalogScanUpdate::Type::Upsert && !coalesced.empty() && coalesced.back().type == CatalogScanUpdate::Type::Upsert)
         coalesced.back().entries.insert(coalesced.back().entries.end(), std::make_move_iterator(update.entries.begin()), std::make_move_iterator(update.entries.end()));
      else
         coalesced.push_back(std::move(update));
   }
   for (const CatalogScanUpdate &update : coalesced)
      ApplyCatalogScanUpdate(update);
   if (complete)
      m_catalogScan.reset();
}

void FrontendUI::AdoptCatalog(TableCatalog &&catalog, const bool restoreResumeSelection)
{
   std::filesystem::path wanted;
   if (restoreResumeSelection && !s_resumeTablePath.empty())
      wanted = s_resumeTablePath;
   else if (HasSelection())
      wanted = m_catalog.GetEntries()[m_selected].tablePath;
   m_catalog = std::move(catalog);

   const auto &entries = m_catalog.GetEntries();
   m_selected = 0;
   bool found = false;
   if (!wanted.empty())
      for (int i = 0; i < static_cast<int>(entries.size()); ++i)
         if (entries[i].tablePath == wanted)
         {
            m_selected = i;
            found = true;
            break;
         }
   m_selected = std::clamp(m_selected, 0, std::max(0, static_cast<int>(entries.size()) - 1));
   m_wheelPos = static_cast<float>(m_selected);
   // A startup cache is only a snapshot: when it does not contain the resume target yet, keep the
   // path so the incremental scan can still select the table if it appears in a later batch.
   if (restoreResumeSelection && found)
      s_resumeTablePath.clear();
}

void FrontendUI::ApplyCatalogScanUpdate(const CatalogScanUpdate &update)
{
   std::filesystem::path wanted;
   const bool restoringResumeSelection = !s_resumeTablePath.empty();
   if (restoringResumeSelection)
      wanted = s_resumeTablePath;
   else if (HasSelection())
      wanted = m_catalog.GetEntries()[m_selected].tablePath;
   if (update.type == CatalogScanUpdate::Type::Reconcile)
      m_catalog.ReconcileEntries(update.entries);
   else
      m_catalog.UpsertEntries(update.entries);

   const auto &entries = m_catalog.GetEntries();
   int selected = 0;
   bool found = false;
   if (!wanted.empty())
      for (int i = 0; i < static_cast<int>(entries.size()); ++i)
         if (entries[i].tablePath == wanted)
         {
            selected = i;
            found = true;
            break;
         }
   const int previousSelected = m_selected;
   m_selected = entries.empty() ? 0 : std::clamp(selected, 0, static_cast<int>(entries.size()) - 1);
   // Entries inserted before the selection shift its index. Shift the continuous wheel position
   // by the same amount so a scan update does not visually jump or fight an in-flight ease.
   if (restoringResumeSelection || !found)
      m_wheelPos = static_cast<float>(m_selected);
   else
      m_wheelPos += static_cast<float>(m_selected - previousSelected);
   // The resumed table may appear in any incremental batch. Keep looking until it is found, or
   // until the complete path reconciliation proves that it is not in the catalog anymore.
   if (!wanted.empty() && !found && update.type == CatalogScanUpdate::Type::Reconcile)
      RememberTablePath(entries.empty() ? std::filesystem::path() : entries[m_selected].tablePath, false);
   if (restoringResumeSelection && (found || update.type == CatalogScanUpdate::Type::Reconcile))
      s_resumeTablePath.clear();
}

void FrontendUI::LaunchSession(const Player::PlayMode playMode, const std::filesystem::path &tablePath)
{
   const TableCatalog::Entry *const entry = tablePath.empty() ? GetSelected() : FindEntry(tablePath);
   if (entry == nullptr || m_tableLoad)
      return;
   if (!TableCatalog::TableExists(*entry))
   {
      m_liveUI.PushNotification("The selected table no longer exists; rescanning the table folders"s, 3500);
      Rescan();
      return;
   }

   // Keep the VR state when playing from an XR frontend; edit sessions stay on the desktop as the
   // live editor is not available inside the headset
   const Player::VROverride vr = m_player->IsVR() && playMode == Player::PlayMode::Play ? Player::VROverride::ForceOn : Player::VROverride::Auto;
   BeginTableLoad(*entry, playMode, vr);
}

void FrontendUI::BeginTableLoad(const TableCatalog::Entry &entry, const Player::PlayMode playMode, const Player::VROverride vr)
{
   const auto load = std::make_shared<PendingTableLoad>();
   load->playMode = playMode;
   load->vr = vr;
   load->title = entry.title;
   const std::filesystem::path tablePath = entry.tablePath;
   m_tableLoad = load;
   RememberTablePath(tablePath, true); // The next frontend session puts the wheel back on it
   g_settingsService.GetAppSettings().Save();
   m_navigationSaveAt = 0;

   // The table file is parsed on a worker thread: the load may take seconds on a full
   // featured table, and the loading screen keeps giving the user feedback meanwhile
   std::thread(
      [load, tablePath]()
      {
         CComObject<PinTable> *table;
         CComObject<PinTable>::CreateInstance(&table);
         table->AddRef();
         VPXFileFeedback feedback;
         const HRESULT hr = table->LoadGameFromFilename(tablePath, feedback);
         PinTable *const loaded = FAILED(hr) && (hr != APPX_E_BLOCK_HASH_INVALID) && (hr != APPX_E_CORRUPT_CONTENT) ? nullptr : table;
         if (loaded == nullptr)
            table->Release();
         load->table.store(loaded, std::memory_order_release);
         load->done.store(true, std::memory_order_release);
         // The owner may have been destroyed while the file was being parsed: it never polls
         // the result anymore, the loaded table is released here instead of being adopted
         if (load->abandoned.load(std::memory_order_acquire))
            if (PinTable *const t = load->table.exchange(nullptr))
               t->Release();
      })
      .detach();

   // The loading screen gives feedback while the file parses, rendered by the frontend itself:
   // back navigation is suspended (see OnNavigateBack) until the load completes
   m_player->m_isLoading = true;
   if (m_liveUI.IsInGameUIOpened())
      m_liveUI.m_inGameUI.Close();
   m_loadingPage = std::make_unique<InGameUI::LoadingPage>("LOADING"s);
   m_loadingPage->Open(false);
}

bool FrontendUI::OnNavigateBack()
{
   // The loading screen is the navigation root while a table file is parsed: back is suspended
   if (m_tableLoad)
      return true;

   return false;
}

bool FrontendUI::NavTriggered(const bool pressed, uint32_t &heldSince, uint32_t &lastTrigger)
{
   const uint32_t now = FrontendTimeMs();
   if (!pressed)
   {
      heldSince = 0;
      return false;
   }
   if (heldSince == 0)
   {
      heldSince = now;
      lastTrigger = now;
      return true;
   }
   const uint32_t repeat = (now - heldSince) < 500 ? 500 : 150;
   if (now - lastTrigger >= repeat)
   {
      lastTrigger = now;
      return true;
   }
   return false;
}

void FrontendUI::ApplyWheelDrag(const float deltaX, const float pxPerCell)
{
   m_wheelDragging = true;
   m_wheelPos -= deltaX / pxPerCell;
   // Smoothed release velocity for the inertial scroll, bounded to keep a violent flick sane
   const float v = -deltaX / pxPerCell / std::max(ImGui::GetIO().DeltaTime, 1e-4f);
   m_wheelVel = std::clamp(m_wheelVel * 0.7f + v * 0.3f, -20.f, 20.f);
   m_wheelNavDir = 0; // A drag cancels a held navigation scroll
   m_wheelNavSince = 0;
}

void FrontendUI::HandleInput()
{
   // The in-game UI chrome owns the input while it is open over the wheel, and the loading
   // screen suspends it while a table file is being parsed
   if (m_liveUI.IsInGameUIOpened() || m_tableLoad)
   {
      m_wheelNavDir = m_wheelNavSince = 0;
      return;
   }
   InputManager &input = m_player->m_pininput;
   const int count = static_cast<int>(m_catalog.GetEntries().size());

   // Navigation: keyboard arrows, cabinet flippers and UI nav actions. A press steps the
   // selection one entry; held past a short delay the wheel scrolls continuously, its speed
   // ramping up with the hold duration (integrated in DrawThemedLayout with the wheel position)
   const bool prevHeld = input.IsPressed(input.GetUILeftActionId()) || input.IsPressed(input.GetUIUpActionId()) || input.IsPressed(input.GetLeftFlipperActionId())
      || ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_UpArrow);
   const bool nextHeld = input.IsPressed(input.GetUIRightActionId()) || input.IsPressed(input.GetUIDownActionId()) || input.IsPressed(input.GetRightFlipperActionId())
      || ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_DownArrow);
   const int navDir = (nextHeld ? 1 : 0) - (prevHeld ? 1 : 0);
   if (navDir != m_wheelNavDir)
   {
      m_wheelNavDir = navDir;
      m_wheelNavSince = navDir != 0 ? FrontendTimeMs() : 0;
      if (count > 0 && navDir != 0)
         SetSelectedIndex(((m_selected + navDir) % count + count) % count, true);
   }

   // Start (or Enter/Space) opens the table actions view, the launch ball button
   // starts the game directly on the selected table
   const bool tablePressed = input.IsPressed(input.GetStartActionId()) || ImGui::IsKeyDown(ImGuiKey_Enter) || ImGui::IsKeyDown(ImGuiKey_KeypadEnter) || ImGui::IsKeyDown(ImGuiKey_Space);
   const bool launchPressed = input.IsPressed(input.GetLaunchBallActionId());
   if (tablePressed && !m_confirmWasPressed)
      OpenActionsView();
   if (launchPressed && !m_launchWasPressed)
      LaunchSession(Player::PlayMode::Play);
   m_confirmWasPressed = tablePressed;
   m_launchWasPressed = launchPressed;

   // Magna buttons jump by one letter group in the catalog
   const bool magnaPrevPressed = input.IsPressed(input.GetLeftMagnaActionId());
   const bool magnaNextPressed = input.IsPressed(input.GetRightMagnaActionId());
   if (magnaPrevPressed && !m_leftMagnaWasPressed)
      JumpLetter(-1);
   if (magnaNextPressed && !m_rightMagnaWasPressed)
      JumpLetter(1);
   m_leftMagnaWasPressed = magnaPrevPressed;
   m_rightMagnaWasPressed = magnaNextPressed;
}

void FrontendUI::JumpLetter(const int dir)
{
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());
   if (count == 0)
      return;
   const auto letterOf = [&entries](const int index)
   {
      const string &title = entries[index].sortTitle;
      return title.empty() ? ' ' : title[0];
   };
   // Titles are sorted, so their first letters form ordered contiguous groups. Binary searching
   // the group bounds keeps letter jumps cheap even on very large catalogs.
   const auto letterBound = [&entries, letterOf](const char letter, const bool after)
   {
      int lo = 0, hi = static_cast<int>(entries.size());
      while (lo < hi)
      {
         const int mid = lo + (hi - lo) / 2;
         const unsigned char at = static_cast<unsigned char>(letterOf(mid));
         const unsigned char bound = static_cast<unsigned char>(letter);
         if (after ? at <= bound : at < bound)
            lo = mid + 1;
         else
            hi = mid;
      }
      return lo;
   };

   const char current = letterOf(m_selected);
   const int currentStart = letterBound(current, false);
   int target;
   if (dir > 0)
   {
      const int end = letterBound(current, true);
      target = end < count ? end : 0;
   }
   else
   {
      target = currentStart > 0 ? letterBound(letterOf(currentStart - 1), false) : letterBound(letterOf(count - 1), false);
   }
   if (target == currentStart)
      return; // A single letter group: nowhere to jump
   SetSelectedIndex(target, true);
   m_wheelPos = static_cast<float>(target);
}

void FrontendUI::OpenActionsView()
{
   // The in-game UI opens on the frontend's navigation root (the chrome's table of
   // contents), then navigates straight to the selected table's actions page
   EnsureSelectedDetails();
   m_liveUI.OpenInGameUI();
   m_liveUI.m_inGameUI.NavigateTo("frontend/table"s);
}

bool FrontendUI::HasSelection() const { return m_selected >= 0 && m_selected < static_cast<int>(m_catalog.GetEntries().size()); }

const TableCatalog::Entry *FrontendUI::GetSelected()
{
   if (!HasSelection())
      return nullptr;
   return &m_catalog.GetEntries()[m_selected];
}

void FrontendUI::EnsureSelectedDetails()
{
   if (HasSelection())
      m_catalog.EnsureEntryDetails(static_cast<size_t>(m_selected), true);
}

void FrontendUI::SetSelectedIndex(const int index, const bool userInitiated)
{
   if (index == m_selected)
      return;
   m_selected = index;
   if (userInitiated)
   {
      m_navigationDirty = true;
      m_lastWheelNavigationAt = FrontendTimeMs();
   }
}

void FrontendUI::RememberTablePath(const std::filesystem::path &tablePath, const bool updateResumePath)
{
   if (updateResumePath)
      s_resumeTablePath = tablePath;
   const string utf8 = tablePath.empty() ? string() : PathToUTF8(tablePath);
   Settings &appSettings = g_settingsService.GetAppSettings();
   if (appSettings.GetFrontend_LastTable() != utf8)
   {
      appSettings.SetFrontend_LastTable(utf8, false);
      m_navigationSaveAt = FrontendTimeMs() + 1000;
   }
}

void FrontendUI::PersistNavigationPosition()
{
   if (!m_navigationDirty)
      return;
   m_navigationDirty = false;
   if (HasSelection())
      RememberTablePath(m_catalog.GetEntries()[m_selected].tablePath, true);
}

const TableCatalog::Entry *FrontendUI::FindEntry(const std::filesystem::path &tablePath)
{
   for (const TableCatalog::Entry &entry : m_catalog.GetEntries())
      if (entry.tablePath == tablePath)
         return &entry;
   return nullptr;
}

void FrontendUI::RestartFrontendSession(const Player::VROverride vr)
{
   PinTable *const table = m_player->m_ptable;
   table->AddRef(); // The session switch takes an owned reference
   m_player->RequestSessionRestart(table, Player::PlayMode::Frontend, vr, false);
}

void FrontendUI::DrawImage(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, const float alpha, const bool autoRotate) const
{
   // Landscape media is shown rotated by 90 degrees clockwise, portrait media as is
   const bool rotate = autoRotate && image->GetWidth() > image->GetHeight();
   const float imgAspect = rotate ? static_cast<float>(image->GetHeight()) / static_cast<float>(image->GetWidth())
                                  : static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
   const float boxAspect = boxSize.x / boxSize.y;
   const ImVec2 size = (imgAspect >= boxAspect) ? ImVec2(boxSize.x, boxSize.x / imgAspect) : ImVec2(boxSize.y * imgAspect, boxSize.y);
   const ImVec2 pos(boxPos.x + (boxSize.x - size.x) * 0.5f, boxPos.y + (boxSize.y - size.y) * 0.5f);
   const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
   if (rotate)
      ImGui::GetWindowDrawList()->AddImageQuad(image, pos, ImVec2(pos.x + size.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(pos.x, pos.y + size.y), ImVec2(0.f, 1.f),
         ImVec2(0.f, 0.f), ImVec2(1.f, 0.f), ImVec2(1.f, 1.f), col);
   else
      ImGui::GetWindowDrawList()->AddImage(image, pos, ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), col);
}

void FrontendUI::DrawImageCover(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, const float alpha, const bool autoRotate) const
{
   // Same rotation rule as DrawImage: landscape media is turned by 90 degrees clockwise
   const bool rotate = autoRotate && image->GetWidth() > image->GetHeight();
   const float imgAspect = rotate ? static_cast<float>(image->GetHeight()) / static_cast<float>(image->GetWidth())
                                  : static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
   const float boxAspect = boxSize.x / boxSize.y;
   const ImVec2 size = (imgAspect >= boxAspect) ? ImVec2(boxSize.y * imgAspect, boxSize.y) : ImVec2(boxSize.x, boxSize.x / imgAspect);
   const ImVec2 pos(boxPos.x + (boxSize.x - size.x) * 0.5f, boxPos.y + (boxSize.y - size.y) * 0.5f);
   const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   drawList->PushClipRect(boxPos, ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y), true);
   if (rotate)
      drawList->AddImageQuad(image, pos, ImVec2(pos.x + size.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(pos.x, pos.y + size.y), ImVec2(0.f, 1.f), ImVec2(0.f, 0.f),
         ImVec2(1.f, 0.f), ImVec2(1.f, 1.f), col);
   else
      drawList->AddImage(image, pos, ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), col);
   drawList->PopClipRect();
}

void FrontendUI::DrawBackground(const TableCatalog::Entry *const selected) const
{
   if (selected == nullptr)
      return;
   std::filesystem::path key;
   bool rotate = false;
   std::shared_ptr<Sampler> image;
   if (selected->HasMedia(TableCatalog::MediaType::Backglass))
   {
      key = selected->GetMedia(TableCatalog::MediaType::Backglass).wstring() + L".blur";
      image = m_media->GetBlurred(*selected, TableCatalog::MediaType::Backglass);
   }
   if (image == nullptr && !selected->HasMedia(TableCatalog::MediaType::Backglass) && selected->HasMedia(TableCatalog::MediaType::Playfield))
   {
      key = selected->GetMedia(TableCatalog::MediaType::Playfield).wstring() + L".blur";
      image = m_media->GetBlurred(*selected, TableCatalog::MediaType::Playfield);
      rotate = true;
   }
   if (image != nullptr)
   {
      const float fadeIn = std::clamp(static_cast<float>(FrontendTimeMs() - m_media->GetReadyTime(key)) / 300.f, 0.f, 1.f);
      DrawImageCover(image, ImVec2(0.f, 0.f), ImGui::GetIO().DisplaySize, fadeIn, rotate);
   }
}

void FrontendUI::DrawPreviewImage(PreviewPane &pane, const TableCatalog::Entry *const selected, const TableCatalog::MediaType type, const ImVec2 &pos,
   const ImVec2 &size, const bool autoRotate)
{
   // Keep the current image while the target one is still being decoded, then crossfade to it
   const std::filesystem::path want = (selected != nullptr && selected->HasMedia(type)) ? selected->GetMedia(type) : std::filesystem::path {};
   DrawPreviewImage(pane, want, pos, size, autoRotate);
}

void FrontendUI::DrawPreviewImage(PreviewPane &pane, const std::filesystem::path &want, const ImVec2 &pos, const ImVec2 &size, const bool autoRotate)
{
   // The wanted media is requested on every frame, not only while it is being decoded: for
   // videos this keeps the decode session alive and the (persistent) sampler animates in place
   std::shared_ptr<Sampler> image;
   if (!want.empty())
      image = m_media->Get(want, static_cast<unsigned int>(std::max(size.x, size.y)), static_cast<unsigned int>(std::max(size.x, size.y)));
   if (want != pane.mediaPath)
   {
      if (image != nullptr || want.empty())
      {
         pane.previous = pane.current;
         pane.current = image;
         pane.mediaPath = want;
         pane.fadeStart = FrontendTimeMs();
      }
   }
   const float t = std::clamp(static_cast<float>(FrontendTimeMs() - pane.fadeStart) / 250.f, 0.f, 1.f);
   if (pane.previous != nullptr && t < 1.f)
      DrawImage(pane.previous, pos, size, 1.f - t, autoRotate);
   if (pane.current != nullptr)
      DrawImage(pane.current, pos, size, t, autoRotate);
}

void FrontendUI::Update()
{
   if (m_player == nullptr)
      m_player = g_pplayer;

   // An asynchronous table load completes on the main thread: adopt the result into a session
   // switch, or report the failure back on the wheel. The loading screen stays up through a
   // successful switch, the new session showing its own preparation screen when needed
   if (m_tableLoad && m_tableLoad->done.load(std::memory_order_acquire))
   {
      const std::shared_ptr<PendingTableLoad> load = std::move(m_tableLoad);
      PinTable *const table = load->table.exchange(nullptr);
      load->abandoned.store(true, std::memory_order_release);
      if (table != nullptr)
      {
         m_player->RequestSessionRestart(table, load->playMode, load->vr, false);
      }
      else
      {
         m_player->m_isLoading = false;
         m_loadingPage.reset();
         m_liveUI.PushNotification(std::format("Failed to load table '{}'", load->title), 4000);
      }
   }

   if (m_media == nullptr)
      m_media = std::make_unique<FrontendMedia>(m_player->m_renderer->m_renderDevice);
   SetupAuxRenderer();
   PollCatalogWatcher();
   if (!m_scanned && !m_catalogScan)
      Rescan();
   m_media->BeginFrame();

   UpdateTheme();

#ifdef ENABLE_XR
   // Headset probe, the same detection the player performs on session start: OpenXR runtime
   // present (instance created in VRDevice ctor) then a system for the HMD form factor. It is
   // retried every few seconds while unavailable so a headset connected or a runtime started
   // after the frontend is shown still enables the VR switch.
   if (!m_vrAvailable && !m_player->IsVR() && FrontendTimeMs() >= m_vrProbeAt)
   {
      m_vrProbeAt = FrontendTimeMs() + 3000;
      VRDevice probe(m_player->m_ptable->GetSettings());
      if (probe.IsOpenXRReady())
      {
         probe.SetupHMD();
         m_vrAvailable = probe.IsOpenXRHMDReady();
      }
   }
#endif

   HandleInput();
   PollCatalogScan();

   const ImGuiIO &io = ImGui::GetIO();
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());

   ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.04f, 1.f));
   ImGui::Begin(
      "##Frontend", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus);

   const TableCatalog::Entry *const selected = GetSelected();

   DrawThemedLayout(selected);
   PersistNavigationPosition();
   if (m_navigationSaveAt != 0 && FrontendTimeMs() >= m_navigationSaveAt)
   {
      m_navigationSaveAt = 0;
      g_settingsService.GetAppSettings().Save();
   }

#ifdef ENABLE_XR
   // Direct switch to VR, the same action as the stereo toggle: a clickable button at the top
   // center of the screen, only in desktop mode (there is no mouse in VR; exiting is done from
   // the stereo toggle or the table actions page), when a VR session can actually run (runtime
   // and HMD probed above)
   if (m_vrAvailable && !m_player->IsVR() && m_tableLoad == nullptr && !m_liveUI.IsInGameUIOpened())
   {
      const ImVec2 size = IconButtonInkSize(ICON_LC_RECTANGLE_GOGGLES);
      ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - size.x) * 0.5f, 8.f * m_liveUI.GetDPI()));
      if (IconButtonInk(ICON_LC_RECTANGLE_GOGGLES, "##ToggleVR"))
         RestartFrontendSession(Player::VROverride::ForceOn);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Enter VR");
   }
#endif
   ImGui::End();
   ImGui::PopStyleColor();

   // The loading page is rendered directly by the frontend while the table file parses on its
   // worker thread (the in-game UI stays closed: it would swallow the frontend's input)
   if (m_loadingPage != nullptr)
   {
      // Its window may sit below the opaque frontend one in ImGui's creation ordered window list
      // (the session's own startup loading screen created it first): hoist it on top
      if (ImGuiWindow *const wnd = ImGui::FindWindowByName("##LoadingScreen"))
         ImGui::BringWindowToDisplayFront(wnd);
      m_loadingPage->Update(io.DeltaTime);
      vector<InGameUI::InGameUIPane> panes;
      m_loadingPage->CollectPanes(panes);
      for (InGameUI::InGameUIPane &pane : panes)
         if (pane.placement == InGameUI::InGameUIPane::Placement::Modal && pane.render)
            pane.render(ImVec4(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y));
   }
}

FrontendUI::Presentation FrontendUI::ResolvePresentation() const
{
   const int bgSet = g_settingsService.GetActiveSettings().GetPlayer_BGSet();
   if (bgSet == 1)
      return Presentation::Cabinet;
   const ImGuiIO &io = ImGui::GetIO();
   // FSS is forced or auto-detected on portrait displays (phones, rotated screens); in XR the
   // frontend is a head locked quad layer in the HMD, where the full single screen layout fits best
   if (bgSet == 2 || io.DisplaySize.y > io.DisplaySize.x || m_player->IsVR())
      return Presentation::FSS;
   return Presentation::Desktop;
}

void FrontendUI::UpdateTheme()
{
   const ImGuiIO &io = ImGui::GetIO();
   const float aspect = io.DisplaySize.x / std::max(1.f, io.DisplaySize.y);
   const Settings &app = g_settingsService.GetAppSettings();
   const string key = app.GetFrontend_Theme() + '|' + app.GetFrontend_ThemeFolders() + '|' + std::format("{:.3f}", aspect) + '|' + std::to_string(static_cast<int>(ResolvePresentation()));
   if (key == m_themeKey)
      return;
   m_themeKey = key;

   // Theme roots: the bundled application assets first, then the user configured folders
   vector<std::filesystem::path> roots { g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "frontend"s };
   const string &folders = app.GetFrontend_ThemeFolders();
   size_t start = 0;
   while (start <= folders.size())
   {
      const size_t end = folders.find(';', start);
      const string folder = folders.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
         roots.emplace_back(folder);
      if (end == string::npos)
         break;
      start = end + 1;
   }
   m_themes = FrontendTheme::Discover(roots);

   const auto it = std::ranges::find_if(m_themes, [&](const FrontendTheme::Entry &e) { return e.name == app.GetFrontend_Theme(); });
   const FrontendTheme::Entry *const theme = it != m_themes.end() ? &*it : (m_themes.empty() ? nullptr : &m_themes.front());

   m_themeLayout = FrontendTheme::Layout {};
   m_themeUnderlay.reset();
   m_themeOverlay.reset();
   m_themeWheelEmpty.reset();
   m_themeUnderlayPath.clear();
   m_themeOverlayPath.clear();
   m_themeWheelEmptyPath.clear();

   if (theme != nullptr)
   {
      if (const std::filesystem::path layoutPath = FrontendTheme::SelectVariant(theme->folder, "layout"s, aspect); !layoutPath.empty())
         if (const std::shared_ptr<BaseTexture> img = BaseTexture::CreateFromFile(layoutPath); img != nullptr)
         {
            m_themeLayout = FrontendTheme::ParseLayout(*img);
            m_themeImgW = static_cast<int>(img->width());
            m_themeImgH = static_cast<int>(img->height());
            PLOGI << "Frontend theme '" << theme->name << "' loaded from " << PathToUTF8(layoutPath);
         }
      // Theme underlay/overlay are drawn covering the theme layout fitted to the display:
      // request the decode bound that matches, they would else be limited to media cell size
      const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
      const unsigned int dispW = static_cast<unsigned int>(displaySize.x), dispH = static_cast<unsigned int>(displaySize.y);
      if (const std::filesystem::path underlay = FrontendTheme::SelectVariant(theme->folder, "underlay"s, aspect, true); !underlay.empty())
      {
         m_themeUnderlayPath = underlay;
         m_themeUnderlay = m_media->Get(underlay, dispW, dispH);
      }
      if (const std::filesystem::path overlay = FrontendTheme::SelectVariant(theme->folder, "overlay"s, aspect, true); !overlay.empty())
      {
         m_themeOverlayPath = overlay;
         m_themeOverlay = m_media->Get(overlay, dispW, dispH);
      }
   }

   // Wheel placeholder for cells without media: theme specific variant first, then the shared
   // 'wheel-empty.webp' sitting next to the theme folders
   if (theme != nullptr)
      m_themeWheelEmptyPath = FrontendTheme::SelectVariant(theme->folder, "wheel-empty"s, aspect, true);
   for (const std::filesystem::path &root : roots)
      if (m_themeWheelEmptyPath.empty())
         m_themeWheelEmptyPath = FrontendTheme::SelectVariant(root, "wheel-empty"s, aspect, true);
   if (!m_themeWheelEmptyPath.empty())
      m_themeWheelEmpty = m_media->Get(m_themeWheelEmptyPath);

   // No usable layout map (no theme or no zones parsed): generate a default one matching the
   // presentation, so the whole UI always renders through the same themed code path
   const FrontendTheme::Layout &l = m_themeLayout;
   if (!(l.playfield.valid || l.backglass.valid || l.card.valid || l.menuBand.valid || l.infoBand.valid || l.infoPane.valid || l.wheelFirstColumn >= 0))
      MakeDefaultLayout();
}

void FrontendUI::MakeDefaultLayout()
{
   const ImGuiIO &io = ImGui::GetIO();
   m_themeImgW = std::max(1, static_cast<int>(io.DisplaySize.x));
   m_themeImgH = std::max(1, static_cast<int>(io.DisplaySize.y));

   FrontendTheme::Layout layout;
   const auto region = [](const float x0, const float y0, const float x1, const float y1)
   {
      FrontendTheme::Region r;
      r.xMin = x0, r.yMin = y0, r.xMax = x1, r.yMax = y1;
      r.valid = true;
      return r;
   };
   layout.wheelFirstColumn = 0;
   layout.wheelLastColumn = m_themeImgW - 1;
   layout.wheelColumns.assign(m_themeImgW, { 0.80f, 0.94f });
   switch (ResolvePresentation())
   {
   case Presentation::Cabinet:
      // The playfield media fills the whole window; other displays are fed through the ancillary renderer
      layout.playfield = region(0.f, 0.f, 1.f, 1.f);
      layout.infoPane = region(0.56f, 0.06f, 0.99f, 0.16f);
      break;
   case Presentation::FSS:
      // Single screen: backglass stacked above the projected playfield
      layout.backglass = region(0.01f, 0.06f, 0.60f, 0.34f);
      layout.infoPane = region(0.62f, 0.06f, 0.99f, 0.34f);
      layout.playfield = region(0.01f, 0.36f, 0.99f, 0.78f);
      break;
   default:
      // Desktop: playfield centered, backglass top left, table info top right
      layout.backglass = region(0.01f, 0.06f, 0.26f, 0.15f);
      layout.infoPane = region(0.55f, 0.06f, 0.99f, 0.15f);
      layout.playfield = region(0.01f, 0.17f, 0.99f, 0.78f);
      break;
   }
   m_themeLayout = layout;
}

void FrontendUI::DrawThemedLayout(const TableCatalog::Entry *const selected)
{
   const ImGuiIO &io = ImGui::GetIO();
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const float uiScale = m_liveUI.GetDPI();
   float scale, offsetX, offsetY;
   FrontendTheme::ComputeFit(m_themeImgW, m_themeImgH, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y), scale, offsetX, offsetY);
   const float imgW = m_themeImgW * scale, imgH = m_themeImgH * scale;

   // Background: the backglass (or playfield) cover fitted and blurred, under the theme's underlay
   DrawBackground(selected);

   // Theme media are requested on every frame: the fetch retries while they decode, and for
   // videos it keeps the decode session alive so the theme keeps animating
   if (!m_themeUnderlayPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeUnderlayPath, static_cast<unsigned int>(io.DisplaySize.x), static_cast<unsigned int>(io.DisplaySize.y)))
         m_themeUnderlay = sampler;
   if (!m_themeOverlayPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeOverlayPath, static_cast<unsigned int>(io.DisplaySize.x), static_cast<unsigned int>(io.DisplaySize.y)))
         m_themeOverlay = sampler;
   if (!m_themeWheelEmptyPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeWheelEmptyPath))
         m_themeWheelEmpty = sampler;
   if (m_themeUnderlay != nullptr)
   {
      const float fade = std::clamp(static_cast<float>(FrontendTimeMs() - m_media->GetReadyTime(m_themeUnderlayPath)) / 300.f, 0.f, 1.f);
      drawList->AddImage(m_themeUnderlay, ImVec2(offsetX, offsetY), ImVec2(offsetX + imgW, offsetY + imgH), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
         IM_COL32(255, 255, 255, static_cast<int>(fade * 255.f)));
   }

   const auto mapRegion = [&](const FrontendTheme::Region &r, ImVec2 &pos, ImVec2 &size)
   {
      pos = ImVec2(offsetX + r.xMin * imgW, offsetY + r.yMin * imgH);
      size = ImVec2((r.xMax - r.xMin) * imgW, (r.yMax - r.yMin) * imgH);
   };

   ImVec2 zonePos, zoneSize;
   if (m_themeLayout.playfield.valid)
   {
      mapRegion(m_themeLayout.playfield, zonePos, zoneSize);
      // Fall back to the backglass media when the table has no playfield image
      const std::filesystem::path want = (selected != nullptr && selected->HasMedia(TableCatalog::MediaType::Playfield))
         ? selected->GetMedia(TableCatalog::MediaType::Playfield)
         : ((selected != nullptr && selected->HasMedia(TableCatalog::MediaType::Backglass)) ? selected->GetMedia(TableCatalog::MediaType::Backglass) : std::filesystem::path {});
      DrawPreviewImage(m_panes[0], want, zonePos, zoneSize, true);
   }
   if (m_themeLayout.backglass.valid)
   {
      mapRegion(m_themeLayout.backglass, zonePos, zoneSize);
      DrawPreviewImage(m_panes[1], selected, TableCatalog::MediaType::Backglass, zonePos, zoneSize);
   }
   if (m_themeLayout.card.valid)
   {
      mapRegion(m_themeLayout.card, zonePos, zoneSize);
      DrawPreviewImage(m_panes[2], selected, TableCatalog::MediaType::GameHelp, zonePos, zoneSize);
   }
   if (m_themeLayout.infoPane.valid)
   {
      mapRegion(m_themeLayout.infoPane, zonePos, zoneSize);
      DrawInfoPane(selected, zonePos, zoneSize);
   }

   // The theme overlay sits between the media elements and the interactive ones (wheel, tabs, help)
   if (m_themeOverlay != nullptr)
   {
      const float overlayFade = std::clamp(static_cast<float>(FrontendTimeMs() - m_media->GetReadyTime(m_themeOverlayPath)) / 300.f, 0.f, 1.f);
      drawList->AddImage(m_themeOverlay, ImVec2(offsetX, offsetY), ImVec2(offsetX + imgW, offsetY + imgH), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
         IM_COL32(255, 255, 255, static_cast<int>(overlayFade * 255.f)));
   }

   // Table wheel inside the red band: unframed items sliding smoothly along the band. Each item is
   // centered horizontally at its (fractional) slot and scaled so its height fits the band's
   // interpolated Y bounds at that column, its middle resting on the band's center line
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());
   const FrontendTheme::Layout &layout = m_themeLayout;
   if (count > 0)
   {
      // Held navigation scrolls the wheel continuously at a speed ramping up with the hold
      // duration, the selection following the position so the centered entry stays in sync.
      // Otherwise the position eases toward the selection, taking the shortest wrap around path
      m_wheelPos = fmodf(fmodf(m_wheelPos, static_cast<float>(count)) + static_cast<float>(count), static_cast<float>(count));
      const uint32_t navHeldFor = m_wheelNavDir != 0 ? FrontendTimeMs() - m_wheelNavSince : 0;
      if (m_wheelDragging)
      {
         // The position was dragged by the pointer; the selection tracks the centered entry
         SetSelectedIndex(((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count, true);
      }
      else if (fabsf(m_wheelVel) > 0.f)
      {
         // Inertial scroll after a flick release: exponential decay, then the ease snaps to a cell
         m_wheelPos += m_wheelVel * io.DeltaTime;
         m_wheelVel *= std::exp(-6.f * io.DeltaTime);
         if (fabsf(m_wheelVel) < 0.4f)
            m_wheelVel = 0.f;
         SetSelectedIndex(((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count, true);
      }
      else if (navHeldFor > 400)
      {
         const float ramp = std::min(1.f, (navHeldFor - 400) / 1600.f);
         m_wheelPos += m_wheelNavDir * (2.5f + 9.5f * ramp * ramp) * io.DeltaTime;
         SetSelectedIndex(((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count, true);
      }
      else
      {
         const float delta = fmodf(m_selected - m_wheelPos + count + count * 0.5f, static_cast<float>(count)) - count * 0.5f;
         m_wheelPos += delta * std::min(1.f, io.DeltaTime * 8.f);
      }
   }
   if (count > 0 && layout.wheelFirstColumn >= 0)
   {
      const int firstCol = layout.wheelFirstColumn, lastCol = layout.wheelLastColumn;
      const float bandCenter = (firstCol + lastCol + 1) * 0.5f;
      const float colsPerCell = (lastCol + 1 - firstCol) / 7.f;
      const float pxPerCell = colsPerCell * scale;
      const bool wheelInteractive = !m_liveUI.IsInGameUIOpened() && m_tableLoad == nullptr;

      m_wheelDragging = false;
      if (wheelInteractive)
      {
         // The whole screen is a drag surface for the wheel (flick scroll on touch, drag scroll
         // with a mouse). Overlappable: without it, the first submitted item under the cursor
         // wins the hover and every cell and chrome button beneath it goes dead
         ImGui::SetNextItemAllowOverlap();
         ImGui::SetCursorScreenPos(ImVec2(0.f, 0.f));
         ImGui::InvisibleButton("##wheelDrag", io.DisplaySize);
         if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f * uiScale))
            ApplyWheelDrag(io.MouseDelta.x, pxPerCell);

         // The mouse wheel steps the selection one entry per notch, the position easing to it like
         // with keyboard navigation. Fractional trackpad deltas accumulate into steps
         if (!m_wheelDragging && ImGui::IsWindowHovered() && io.MouseWheel != 0.f)
         {
            m_wheelVel = 0.f; // A wheel scroll cancels an in-flight flick
            m_wheelWheelAccum += io.MouseWheel;
            while (m_wheelWheelAccum >= 1.f)
            {
               SetSelectedIndex((m_selected - 1 + count) % count, true);
               m_wheelWheelAccum -= 1.f;
            }
            while (m_wheelWheelAccum <= -1.f)
            {
               SetSelectedIndex((m_selected + 1) % count, true);
               m_wheelWheelAccum += 1.f;
            }
         }
      }


      const int kMin = -std::min(4, (count - 1) / 2), kMax = std::min(4, count / 2);
      const int wheelCenterColumn = std::clamp(static_cast<int>(floorf(bandCenter)), firstCol, lastCol);
      const float wheelCellHeight = (layout.wheelColumns[wheelCenterColumn].second - layout.wheelColumns[wheelCenterColumn].first) * imgH;
      const unsigned int wheelDecodeSize = static_cast<unsigned int>(std::max(64.f, wheelCellHeight * 2.f));
      drawList->PushClipRect(ImVec2(offsetX + firstCol * scale, 0.f), ImVec2(offsetX + (lastCol + 1) * scale, io.DisplaySize.y), true);
      // Farthest items first so the centered item draws on top
      for (int d = std::max(-kMin, kMax); d >= 0; d--)
         for (const int k : (d == 0 ? std::initializer_list<int> { 0 } : std::initializer_list<int> { d, -d }))
         {
            if (k < kMin || k > kMax)
               continue;
            const int index = ((m_selected + k) % count + count) % count;
            // Fractional slot offset on the circle: the eased position may sit just below 0 when the
            // selection wraps from the first to the last entry
            const float o = fmodf(static_cast<float>(m_selected + k) - m_wheelPos + count + count * 0.5f, static_cast<float>(count)) - count * 0.5f;
            const float alpha = std::clamp(3.6f - fabsf(o), 0.f, 1.f); // fade out at the band edges
            if (alpha <= 0.f)
               continue;
            const float centerCol = bandCenter + o * colsPerCell;
            const int c = std::clamp(static_cast<int>(floorf(centerCol)), firstCol, lastCol);
            const float yMin = layout.wheelColumns[c].first, yMax = layout.wheelColumns[c].second;
            if (yMax <= yMin)
               continue;
            const float h = (yMax - yMin) * imgH;
            const float cy = offsetY + (yMin + yMax) * 0.5f * imgH;
            const float cx = offsetX + centerCol * scale;

            const auto &entry = entries[index];
            const std::filesystem::path mediaPath = entry.HasMedia(TableCatalog::MediaType::Wheel) ? entry.GetMedia(TableCatalog::MediaType::Wheel) : std::filesystem::path {};
            const std::shared_ptr<Sampler> image = mediaPath.empty() ? nullptr : m_media->Get(mediaPath, wheelDecodeSize, wheelDecodeSize);
            // The wheel art fades in once decoded; the shared wheel-empty placeholder carries the
            // table name for media-less entries and stands in until the art is fully faded in
            const float artAlpha = image != nullptr ? std::clamp(static_cast<float>(FrontendTimeMs() - m_media->GetReadyTime(mediaPath)) / 300.f, 0.f, 1.f) : 0.f;
            ImVec2 cellSize(h, h);
            if (m_themeWheelEmpty != nullptr && (image == nullptr || artAlpha < 1.f))
            {
               cellSize.x = h * static_cast<float>(m_themeWheelEmpty->GetWidth()) / static_cast<float>(m_themeWheelEmpty->GetHeight());
               const ImVec2 pos(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f);
               drawList->AddImage(
                  m_themeWheelEmpty, pos, ImVec2(pos.x + cellSize.x, pos.y + cellSize.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f)));

               // Table name over the placeholder, word wrapped once per entry then just rescaled:
               // measuring text at an arbitrary, constantly changing font size while the wheel slides
               // continuously bakes new font variants, which stutters the navigation
               WheelTitle &wt = m_wheelTitles[entry.tablePath];
               if (wt.title != entry.title)
                  wt = WheelTitle { entry.title };
               if (wt.size0 == 0.f && !entry.title.empty())
               {
                  string title = entry.title;
                  std::ranges::replace(title, '_', ' '); // '_' is common in table names and wraps poorly
                  ImFont *const font = ImGui::GetFont();
                  wt.size0 = 20.f;
                  const float wrap0 = wt.size0 * (cellSize.x * 0.8f) / (cellSize.y * 0.14f);
                  for (const char *p = title.c_str(), *const end = p + title.size(); p < end;)
                  {
                     const char *lineEnd = font->CalcWordWrapPosition(wt.size0, p, end, wrap0);
                     while (lineEnd > p && lineEnd[-1] == ' ')
                        --lineEnd;
                     if (lineEnd <= p)
                        lineEnd = p + 1; // always progress, even for a narrow cell or unbreakable word
                     wt.lines.emplace_back(p, lineEnd);
                     p = lineEnd;
                     while (p < end && (*p == ' ' || *p == '\n'))
                        ++p;
                  }

                  // Ellipsize the last line when the wrapped title does not fit in the cell height
                  const int maxLines = static_cast<int>(floorf(0.9f / 0.14f));
                  if (static_cast<int>(wt.lines.size()) > maxLines)
                  {
                     wt.lines.resize(maxLines);
                     string &last = wt.lines.back();
                     const float dotsW = font->CalcTextSizeA(wt.size0, FLT_MAX, 0, "...").x;
                     while (!last.empty() && (last.back() == ' ' || font->CalcTextSizeA(wt.size0, FLT_MAX, 0, last.c_str()).x + dotsW > wrap0))
                        last.pop_back();
                     last += "...";
                  }
                  wt.widths.reserve(wt.lines.size());
                  for (const string &line : wt.lines)
                     wt.widths.push_back(font->CalcTextSizeA(wt.size0, FLT_MAX, 0, line.c_str()).x);
               }
               ImFont *const font = ImGui::GetFont();
               const float fontSize = std::max(8.f, roundf(cellSize.y * 0.14f)); // whole pixels bound the font bake variants
               const float scale = fontSize / wt.size0;
               const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
               float ty = cy - wt.lines.size() * fontSize * 0.5f;
               for (size_t i = 0; i < wt.lines.size(); ++i, ty += fontSize)
                  drawList->AddText(font, fontSize, ImVec2(cx - wt.widths[i] * scale * 0.5f, ty), col, wt.lines[i].c_str());
            }
            else if (image == nullptr)
            {
               const ImVec2 iconSize = ImGui::CalcTextSize(ICON_LC_IMAGE);
               drawList->AddText(ImVec2(cx - iconSize.x * 0.5f, cy - iconSize.y * 0.5f), IM_COL32(90, 90, 110, static_cast<int>(alpha * 200.f)), ICON_LC_IMAGE);
            }
            if (image != nullptr)
            {
               cellSize.x = h * static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
               const ImVec2 pos(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f);
               drawList->AddImage(
                  image, pos, ImVec2(pos.x + cellSize.x, pos.y + cellSize.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32(255, 255, 255, static_cast<int>(alpha * artAlpha * 255.f)));
            }

            ImGui::SetCursorScreenPos(ImVec2(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f));
            ImGui::PushID(k); // Id by slot: stable and unique even when the catalog wraps
            ImGui::InvisibleButton("##cell", cellSize);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f * uiScale))
               ApplyWheelDrag(io.MouseDelta.x, pxPerCell);
            if (ImGui::IsItemDeactivated())
            {
               const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(drag.x) < 8.f * uiScale && fabsf(drag.y) < 8.f * uiScale)
               {
                  if (index == m_selected)
                     OpenActionsView();
                  else
                     SetSelectedIndex(index, true);
               }
            }
            ImGui::PopID();
         }
      drawList->PopClipRect();

      // Prefetch a small margin past the visible items so scrolling feels instant
      for (int k = kMin - 3; k <= kMax + 3; k++)
      {
         if (k >= kMin && k <= kMax)
            continue; // already requested by the draw loop
         const int index = ((m_selected + k) % count + count) % count;
         const TableCatalog::Entry &e = entries[index];
         if (e.HasMedia(TableCatalog::MediaType::Wheel) && !TableCatalog::IsVideoFile(e.GetMedia(TableCatalog::MediaType::Wheel)))
            m_media->Get(
               e.GetMedia(TableCatalog::MediaType::Wheel), wheelDecodeSize, wheelDecodeSize); // Videos open when the cell actually shows; animated images are classified by the decode worker
      }
   }

   // Bottom chrome bar: the InGameUI desktop renderer's hint bar, drawn by its shared
   // rendering at the display's bottom edge — the theme layout does not place it
   DrawInfoBand();
}

void FrontendUI::DrawInfoBand()
{
   // The in-game UI chrome draws its own hint bar at the same place when it is open
   if (m_liveUI.IsInGameUIOpened())
      return;
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const ImGuiIO &io = ImGui::GetIO();

   // Contextual hints following the navigation state, drawn by the InGameUI
   // desktop renderer's shared bottom bar (same band height and chip styling)
   vector<InGameUI::DesktopHint> hints { { "Flippers", "navigate" }, { "Magna", "letter" }, { "Start", "table actions" }, { "Launch", "play" }, { "Esc", "quit" } };
   if (IsVRAvailable())
      hints.insert(hints.end() - 1, { "Stereo", m_player->IsVR() ? "desktop" : "VR" });
   InGameUI::RenderDesktopHintBar(m_player, drawList, ImVec2(0.f, io.DisplaySize.y - InGameUI::DesktopHintBarHeight(m_player)), io.DisplaySize.x, hints);
}

void FrontendUI::DrawInfoPane(const TableCatalog::Entry *const selected, const ImVec2 &pos, const ImVec2 &size)
{
   if (selected == nullptr)
      return;
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const float uiScale = m_liveUI.GetDPI();
   const float pad = 10.f * uiScale;
   drawList->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
   float y = pos.y + pad;
   const float textW = size.x - 2.f * pad;

   // Table title: bold, underlined, shrunk until it fits on one line
   ImFont *const bold = m_liveUI.GetOverlayBoldFont();
   float fontSize = std::min(30.f * uiScale, size.y * 0.22f);
   ImVec2 titleSize = bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, selected->title.c_str());
   while (fontSize > 10.f * uiScale && titleSize.x > textW)
   {
      fontSize *= 0.85f;
      titleSize = bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, selected->title.c_str());
   }
   const ImVec2 titlePos(pos.x + pad, y);
   const ImU32 titleCol = IM_COL32(255, 255, 255, 255);
   drawList->AddText(bold, fontSize, titlePos, titleCol, selected->title.c_str());
   const float underlineY = titlePos.y + titleSize.y + 2.f * uiScale;
   drawList->AddLine(ImVec2(titlePos.x, underlineY), ImVec2(titlePos.x + std::min(titleSize.x, textW), underlineY), titleCol, std::max(1.f, 1.5f * uiScale));
   y = underlineY + pad;

   // Manufacturer and year of publishing
   ImFont *const font = ImGui::GetFont();
   const float lineSize = std::min(18.f * uiScale, size.y * 0.14f);
   const ImU32 dimCol = IM_COL32(170, 170, 190, 230);
   string sub;
   if (!selected->info.manufacturer.empty())
      sub = selected->info.manufacturer;
   if (selected->info.year != 0)
      sub += (sub.empty() ? ""s : " "s) + std::format("({})", selected->info.year);
   if (!sub.empty())
   {
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, sub.c_str());
      y += lineSize * 1.3f;
   }

   if (!selected->info.authors.empty())
   {
      string authors = "by " + selected->info.authors.front();
      for (size_t i = 1; i < selected->info.authors.size(); i++)
         authors += ", " + selected->info.authors[i];
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, authors.c_str());
      y += lineSize * 1.3f;
   }
   if (selected->vpx.startCount > 0)
   {
      const string runTime = FormatRunTime(selected->vpx.runTime);
      const string line = std::format("Played {} time{}{}{}", selected->vpx.startCount, selected->vpx.startCount == 1 ? "" : "s", runTime.empty() ? "" : " - total ", runTime);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
      y += lineSize * 1.3f;
   }
   if (!selected->vpx.lastRun.empty())
   {
      const string line = "Last played: "s + selected->vpx.lastRun.substr(0, 10);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
      y += lineSize * 1.3f;
   }
   if (selected->user.favorite || selected->user.rating > 0)
   {
      string line;
      if (selected->user.favorite)
         line = "Favorite";
      if (selected->user.rating > 0)
         line += (line.empty() ? ""s : "  "s) + std::format("Rating {}/10", selected->user.rating);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
   }
   drawList->PopClipRect();
}

string FrontendUI::FormatRunTime(const int64_t seconds)
{
   if (seconds <= 0)
      return { };
   const int64_t hours = seconds / 3600;
   const int64_t minutes = (seconds % 3600) / 60;
   if (hours > 0)
      return std::format("{}h {:02}m", hours, minutes);
   return std::format("{}m", std::max<int64_t>(minutes, 1)); // Sub-minute sessions round up to avoid "0m"
}

}
