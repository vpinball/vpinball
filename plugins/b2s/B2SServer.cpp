// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <random>
#include <thread>

#include <cstddef> // for size_t, ptrdiff_t
// Define ssize_t for Windows
#if defined(_WIN32) && !defined(__SSIZE_T_DEFINED)
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif
#define __SSIZE_T_DEFINED
#endif

#include "base64.h"
#include "tinyxml2/tinyxml2.h"
#include "plugins/PluginStrings.h"

namespace B2S
{


static const char* usedLEDTypeValues[] = { "Auto", "Rendered", "Dream7" };
MSGPI_ENUM_VAL_SETTING(usedLEDTypeProp, "UsedLEDType", "Used LED type", "Force the LED display type (Auto = as declared by the .directb2s file)", true, 0, 3, usedLEDTypeValues, 0);
MSGPI_INT_VAL_SETTING(allAnimationSlowDownProp, "AllAnimationSlowDown", "Global animation slowdown", "Slowdown factor applied to all backglass animations", true, 1, 100, 1);
MSGPI_STRING_VAL_SETTING(animationSlowDownsProp, "AnimationSlowDowns", "Per-animation slowdowns", "Semicolon separated 'name=factor' animation slowdown overrides", true, "", 1024);

void B2SServer::RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId)
{
   msgApi->RegisterSetting(endpointId, &usedLEDTypeProp);
   msgApi->RegisterSetting(endpointId, &allAnimationSlowDownProp);
   msgApi->RegisterSetting(endpointId, &animationSlowDownsProp);
}

namespace
{
   // The plugin redefines its default "settings" sheet when a backglass file is loaded, to group the
   // settings under a section header and to expose the file editor (which the host appends as a
   // navigation entry below the last header). Items reuse the RegisterSetting ids, so they share the
   // same host persisted properties.

   int MSGPIAPI AllSlowdownGet(void*) { return allAnimationSlowDownProp_Get(); }
   void MSGPIAPI AllSlowdownSet(void*, int v) { allAnimationSlowDownProp_Set(v); }
   const char* MSGPIAPI SlowdownsGet(void*) { return animationSlowDownsProp_Get(); }
   void MSGPIAPI SlowdownsSet(void*, const char* v) { animationSlowDownsProp_Set(v); }
   int MSGPIAPI LedTypeGet(void*) { return usedLEDTypeProp_Get(); }
   void MSGPIAPI LedTypeSet(void*, int v) { usedLEDTypeProp_Set(v); }

   void FillHeader(PagedUIItemDef& item, const char* label)
   {
      item = PagedUIItemDef {};
      item.type = PAGEDUI_ITEM_HEADER;
      item.label = label;
   }

   unsigned int MSGPIAPI SettingsPageCount(void*) { return 1; }

   void MSGPIAPI SettingsPageInfo(void*, unsigned int, PagedUIPageInfo* info)
   {
      info->id = "settings";
      info->title = nullptr;
      info->info = nullptr;
   }

   unsigned int MSGPIAPI SettingsItemCount(void*, const char*) { return 8; }

   void MSGPIAPI SettingsGetItems(void*, const char*, PagedUIItemDef* items)
   {
      int pos = 0;
      FillHeader(items[pos++], "Backglass Settings");
      pos += B2SRenderer::FillSettingsItems(items + pos); // ShowGrill, DualMode, UpgradedSegmentRendering
      PagedUIItemDef& ledType = items[pos++];
      ledType = {};
      ledType.type = PAGEDUI_ITEM_ENUM;
      ledType.id = "UsedLEDType";
      ledType.label = "Used LED type";
      ledType.tooltip = "Force the LED display type (Auto = as declared by the .directb2s file)";
      ledType.storage = PAGEDUI_STORAGE_HOST;
      ledType.enumProp = { 0, 3, usedLEDTypeValues, 0, &LedTypeGet, &LedTypeSet };
      PagedUIItemDef& allSlowdown = items[pos++];
      allSlowdown = {};
      allSlowdown.type = PAGEDUI_ITEM_INT;
      allSlowdown.id = "AllAnimationSlowDown";
      allSlowdown.label = "Global animation slowdown";
      allSlowdown.tooltip = "Slowdown factor applied to all backglass animations";
      allSlowdown.storage = PAGEDUI_STORAGE_HOST;
      allSlowdown.intProp = { 1, 100, 1, nullptr, &AllSlowdownGet, &AllSlowdownSet };
      PagedUIItemDef& slowdowns = items[pos++];
      slowdowns = {};
      slowdowns.type = PAGEDUI_ITEM_STRING;
      slowdowns.id = "AnimationSlowDowns";
      slowdowns.label = "Per-animation slowdowns";
      slowdowns.tooltip = "Semicolon separated 'name=factor' animation slowdown overrides";
      slowdowns.storage = PAGEDUI_STORAGE_HOST;
      slowdowns.stringProp = { "", &SlowdownsGet, &SlowdownsSet };
      FillHeader(items[pos++], "Modify Backglass");
   }

   // Saving rewrites the whole .directb2s, can be too slow for the game thread, and also can fail: so saves are
   // queued to a worker thread, which writes a temporary file and renames it over the original, so that a failed
   // save can't leave a truncated file. Errors are reported on the main thread (log and notification)
   struct FileWriter
   {
      const MsgPluginAPI* msgApi = nullptr;
      unsigned int endpointId = 0;
      const VPXPluginAPI* vpxApi = nullptr;
      std::mutex mutex;
      std::condition_variable cv;
      std::thread thread;
      bool stop = false;
      vector<std::pair<std::filesystem::path, vector<B2SDMDDisplay>>> jobs; // Pending saves, at most one per file
      vector<string> errors; // Failed saves, not yet reported
   };
   FileWriter fileWriter;

   void MSGPIAPI ReportSaveErrors(void*)
   {
      vector<string> errors;
      {
         std::lock_guard lock(fileWriter.mutex);
         errors.swap(fileWriter.errors);
      }
      for (const string& error : errors)
      {
         LOGE(error);
         if (fileWriter.vpxApi != nullptr)
            fileWriter.vpxApi->PushNotification(error.c_str(), 10000);
      }
   }

   // Rewrites only the <DMD> children of the file's <Displays> element, leaving all other content (including the
   // embedded base64 images) untouched. Returns an error message, or an empty string on success
   string WriteDisplays(const std::filesystem::path& path, const vector<B2SDMDDisplay>& displays)
   {
      const string failed = "Failed to save the DMD displays to " + PluginStrings::PathToUTF8(path);
      std::ifstream in(path, std::ios::binary);
      if (!in)
         return failed + " (can't read it)";
      const std::string buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      in.close();
      tinyxml2::XMLDocument xmlDoc;
      if (xmlDoc.Parse(buffer.c_str(), buffer.size()) != tinyxml2::XML_SUCCESS)
         return failed + " (can't parse it)";
      tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("DirectB2SData");
      if (root == nullptr)
         return failed + " (not a B2S file)";
      auto* displaysNode = root->FirstChildElement("Displays");
      if (displaysNode == nullptr)
         displaysNode = static_cast<tinyxml2::XMLElement*>(root->InsertEndChild(xmlDoc.NewElement("Displays")));
      // Remove the previously authored DMD children; children of other display kinds are preserved
      while (tinyxml2::XMLElement* dmd = displaysNode->FirstChildElement("DMD"))
         displaysNode->DeleteChild(dmd);
      for (const B2SDMDDisplay& display : displays)
      {
         auto* const e = xmlDoc.NewElement("DMD");
         e->SetAttribute("Target", display.target == 1 ? "ScoreView" : "Backglass");
         if (!display.source.empty())
            e->SetAttribute("Source", display.source.c_str());
         e->SetAttribute("X", display.x);
         e->SetAttribute("Y", display.y);
         e->SetAttribute("W", display.w);
         e->SetAttribute("H", display.h);
         const PinballPlugin::OverlayDisplayOptions& o = display.options;
         if (o.style > 0 && o.style <= 8)
            e->SetAttribute("Style", PinballPlugin::OverlayDisplayStyleNames[o.style - 1]);
         if (o.sizeW > 0 || o.sizeH > 0)
            e->SetAttribute("Dots", std::format("{},{}", o.sizeW, o.sizeH).c_str());
         if (o.tintR != 1.f || o.tintG != 1.f || o.tintB != 1.f)
         {
            const float tint[] = { o.tintR, o.tintG, o.tintB };
            e->SetAttribute("Tint", PinballPlugin::OverlayFormatFloats(tint, 3).c_str());
         }
         if (o.glassTintR != 1.f || o.glassTintG != 1.f || o.glassTintB != 1.f)
         {
            const float tint[] = { o.glassTintR, o.glassTintG, o.glassTintB };
            e->SetAttribute("GlassTint", PinballPlugin::OverlayFormatFloats(tint, 3).c_str());
         }
         if (o.glassRoughness != 0.f)
            e->SetAttribute("GlassRoughness", PinballPlugin::OverlayFormatFloats(&o.glassRoughness, 1).c_str());
         if (o.glassAmbientR != 1.f || o.glassAmbientG != 1.f || o.glassAmbientB != 1.f)
         {
            const float ambient[] = { o.glassAmbientR, o.glassAmbientG, o.glassAmbientB };
            e->SetAttribute("GlassAmbient", PinballPlugin::OverlayFormatFloats(ambient, 3).c_str());
         }
         if (o.glassAreaX != 0 || o.glassAreaY != 0 || o.glassAreaW != 0 || o.glassAreaH != 0)
            e->SetAttribute("GlassArea", std::format("{},{},{},{}", o.glassAreaX, o.glassAreaY, o.glassAreaW, o.glassAreaH).c_str());
         if (o.glassPadL != 0.f || o.glassPadT != 0.f || o.glassPadR != 0.f || o.glassPadB != 0.f)
         {
            const float pad[] = { o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB };
            e->SetAttribute("GlassPad", PinballPlugin::OverlayFormatFloats(pad, 4).c_str());
         }
         if (!display.glassImage.empty())
         {
            auto* const image = xmlDoc.NewElement("GlassImage");
            image->SetAttribute("Value", to_base64(display.glassImage.data(), display.glassImage.size()).c_str());
            e->InsertEndChild(image);
         }
         displaysNode->InsertEndChild(e);
      }
      tinyxml2::XMLPrinter printer;
      xmlDoc.Print(&printer);
      std::filesystem::path tmpPath = path;
      tmpPath += ".tmp";
      std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
      if (out)
      {
         out << printer.CStr();
         out.close();
      }
      std::error_code ec;
      if (!out)
      {
         std::filesystem::remove(tmpPath, ec);
         return failed + " (can't write it)";
      }
      std::filesystem::rename(tmpPath, path, ec);
      if (ec)
      {
         std::filesystem::remove(tmpPath, ec);
         return failed + " (can't replace it)";
      }
      return {};
   }

   void FileWriterLoop()
   {
      std::unique_lock lock(fileWriter.mutex);
      while (true)
      {
         fileWriter.cv.wait(lock, [] { return fileWriter.stop || !fileWriter.jobs.empty(); });
         if (fileWriter.jobs.empty())
            return; // Stop requested, all saves done
         auto [path, displays] = std::move(fileWriter.jobs.front());
         fileWriter.jobs.erase(fileWriter.jobs.begin());
         lock.unlock();
         string error = WriteDisplays(path, displays);
         lock.lock();
         if (!error.empty())
         {
            fileWriter.errors.push_back(std::move(error));
            fileWriter.msgApi->RunOnMainThread(fileWriter.endpointId, 0., ReportSaveErrors, nullptr);
         }
      }
   }

   // Doc::commitEdit hook: queues the save, so the page reports the document as saved once queued and a failure
   // is notified later
   int CommitOverlayDoc(B2SOverlayDoc& doc)
   {
      if (doc.path.empty())
         return 1;
      {
         std::lock_guard lock(fileWriter.mutex);
         if (!fileWriter.thread.joinable())
            return 1;
         // A newer save of the same file replaces the pending one
         const auto job = std::ranges::find_if(fileWriter.jobs, [&doc](const auto& job) { return job.first == doc.path; });
         if (job != fileWriter.jobs.end())
            job->second = doc.displays;
         else
            fileWriter.jobs.emplace_back(doc.path, doc.displays);
      }
      fileWriter.cv.notify_one();
      return 0;
   }
}

void B2SServer::StartFileWriter(const MsgPluginAPI* msgApi, unsigned int endpointId, const VPXPluginAPI* vpxApi)
{
   assert(!fileWriter.thread.joinable());
   fileWriter.msgApi = msgApi;
   fileWriter.endpointId = endpointId;
   fileWriter.vpxApi = vpxApi;
   fileWriter.stop = false;
   fileWriter.thread = std::thread(FileWriterLoop);
}

void B2SServer::StopFileWriter()
{
   if (!fileWriter.thread.joinable())
      return;
   {
      std::lock_guard lock(fileWriter.mutex);
      fileWriter.stop = true;
   }
   fileWriter.cv.notify_one();
   fileWriter.thread.join(); // Pending saves are completed
   fileWriter.msgApi->FlushPendingCallbacks(fileWriter.endpointId); // Reports their errors
   fileWriter.msgApi = nullptr;
   fileWriter.vpxApi = nullptr;
}

B2SServer::B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, const PagedUIPluginAPI* const pagedUIApi, ScriptClassDef* serverClassDef)
   : m_controllerClassProxy(msgApi, endpointId, "PinMAME_"s, "PinMAME_Controller"s, "B2S_"s, serverClassDef)
   , m_controllerProxy(m_controllerClassProxy)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_vpxApi(vpxApi)
   , m_pagedUIApi(pagedUIApi)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_ancillaryRendererDef({ "B2S", "B2S Backglass & FullDMD", "Renderer for directb2s backglass files", this, OnRender })
   , m_onStateChangeEventId(msgApi->GetMsgID("B2S", "OnStateChange:1"))
   , m_exposedControllers(msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG)
   , m_exposedStates(msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG)
   , m_audioSrc(msgApi, endpointId, CTLPI_AUDIO_GET_SRC_MSG, CTLPI_AUDIO_ON_SRC_CHG_MSG)
   , m_onAudioUpdateId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG))
{
   VPXTableInfo tableInfo {};
   m_vpxApi->GetTableInfo(&tableInfo);

   const std::filesystem::path tablePath = PluginStrings::PathFromNative(tableInfo.path);
   m_tableDir = tablePath.parent_path();
   m_tableName = PluginStrings::PathToUTF8(tablePath.stem());
   TryLoadB2S();

   // Bind the shared overlay document to the plugin data model (see DMDOverlay/DMDOverlay.h):
   // two views, bounds in artwork pixels rendered through the default full-view flip (no
   // mapQuad hook), glass image embedded as base64, persistence by CommitOverlayDoc
   static const char* targetNames[] = { "Backglass", "Score view" };
   m_overlayDoc.vpxApi = m_vpxApi;
   m_overlayDoc.log = [](const char* func, int line, unsigned int level, const std::string& message) { LPILog_CPP(func, line, level, message); };
   m_overlayDoc.targetCount = 2;
   m_overlayDoc.targetNames = targetNames;
   m_overlayDoc.detectable = true;
   m_overlayDoc.hasGlass = [](const B2SDMDDisplay& display) { return !display.glassImage.empty(); };
   m_overlayDoc.glassBytes = [](B2SDMDDisplay& display) -> const vector<uint8_t>*
   {
      if (display.glassImage.empty())
      {
         display.glassKey = 0;
         return nullptr;
      }
      size_t hash = 14695981039346656037ull;
      for (const uint8_t byte : display.glassImage)
         hash = (hash ^ byte) * 1099511628211ull;
      display.glassKey = hash;
      return &display.glassImage;
   };
   m_overlayDoc.setGlass = [](B2SDMDDisplay& display, const std::filesystem::path& picked)
   {
      if (!DMDOverlay::ReadFileBytes(picked, display.glassImage))
      {
         LOGE("Failed to read glass image file: "s + PluginStrings::PathToUTF8(picked));
         return false;
      }
      return true;
   };
   m_overlayDoc.clearGlass = [](B2SDMDDisplay& display) { display.glassImage.clear(); };
   m_overlayDoc.commitEdit = &CommitOverlayDoc;

   m_msgApi->SubscribeMsg(m_endpointId, m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   // Locate the 'Switch' setter on the proxied PinMAME controller (used to pulse switches from animations)
   for (unsigned int i = 0; i < serverClassDef->nMembers; i++)
      if (serverClassDef->members[i].name.name == "Switch"sv && serverClassDef->members[i].nArgs == 2)
         m_setSwitchIndex = static_cast<int>(i);

   m_b2sName = "b2s::"sv;
   SetB2SName(""s);

   m_audioSrc.SetItem({ { m_endpointId, 0 }, { 0, 0 }, "B2S Sounds", "Sounds embedded in the backglass file", CTLPI_AUDIO_TARGET_BACKGLASS });
}

B2SServer::~B2SServer()
{
   m_msgApi->FlushPendingCallbacks(m_endpointId);

   if (m_loadedB2S.valid())
      m_loadedB2S.wait();
   m_renderer = nullptr;

   m_gameRunning = false;
   m_exposedControllers.ClearItems();
   m_exposedStates.ClearItems();
   m_audioSrc.ClearItems();

   if (m_overlaySheetRegistered && m_pagedUIApi != nullptr)
   {
      m_pagedUIApi->UnregisterSheet(m_endpointId, "backglass");
      m_pagedUIApi->UnregisterSheet(m_endpointId, "settings");
   }

   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
   m_msgApi->ReleaseMsgID(m_onStateChangeEventId);
   m_msgApi->ReleaseMsgID(m_onAudioUpdateId);

   if (m_onDestroyHandler)
      m_onDestroyHandler(this);
}

// Compatibility version reported to scripts, matching the B2S backglass server series the plugin emulates
string B2SServer::GetB2SServerVersion() const { return "2.3.1"s; }

double B2SServer::GetB2SBuildVersion() const { return 20301.0999; }

string B2SServer::GetB2SServerDirectory() const
{
   const std::filesystem::path& dir = m_workingDir.empty() ? m_tableDir : m_workingDir;
   return PluginStrings::PathToUTF8(dir);
}

std::shared_ptr<B2STable> B2SServer::LoadB2SFile(const std::filesystem::path& path)
{
   std::shared_ptr<B2STable> b2s;
   try
   {
      std::ifstream file(path, std::ios::binary);
      if (!file)
      {
         LOGE("Failed to open B2S file: " + PluginStrings::PathToUTF8(path));
         return b2s;
      }
      const std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      tinyxml2::XMLDocument b2sTree;
      if (b2sTree.Parse(buffer.c_str(), buffer.size()) != tinyxml2::XML_SUCCESS)
      {
         const char* const error = b2sTree.ErrorStr();
         LOGE("Failed to parse B2S file: " + PluginStrings::PathToUTF8(path) + " (" + (error ? error : "") + ')');
         return b2s;
      }
      if (b2sTree.FirstChildElement("DirectB2SData"))
         b2s = std::make_shared<B2STable>(*b2sTree.FirstChildElement("DirectB2SData"));
      else
         LOGE("Invalid B2S file: " + PluginStrings::PathToUTF8(path));
   }
   catch (...)
   {
      LOGE("Failed to load B2S file: " + PluginStrings::PathToUTF8(path));
   }
   return b2s;
}

// Backglass file lookup: exact match on the table name, then 'foldername.directb2s' for file layouts
// where the table shares a folder with its companion files (b2s, pup, flex, music, ...)
std::filesystem::path B2SServer::FindB2SFile() const
{
   const std::filesystem::path dir = m_workingDir.empty() ? m_tableDir : m_workingDir;
   if (dir.empty() || m_tableName.empty())
      return {};
   const auto tryFile = [&dir](const string& name) { return name.empty() ? std::filesystem::path() : find_case_insensitive_file_path(dir / (name + ".directb2s"s)); };

   if (auto file = tryFile(m_tableName); !file.empty())
      return file;

   if (const string folderName = PluginStrings::PathToUTF8(dir.filename()); !folderName.empty())
      if (auto file = tryFile(folderName); !file.empty())
         return file;

   return {};
}

void B2SServer::TryLoadB2S()
{
   std::lock_guard lock(m_b2sMutex);
   if (m_b2s || m_loadedB2S.valid())
      return;
   const std::filesystem::path b2sFilename = FindB2SFile();
   if (b2sFilename.empty())
      return;
   m_b2sPath = b2sFilename;
   RegisterOverlaySheet();
   // B2S file format is heavily unoptimized so perform loading asynchronously (all assets are directly included in the XML file using Base64 encoding)
   m_loadedB2S = std::async(std::launch::async, LoadB2SFile, b2sFilename);
}

void B2SServer::SetTableName(const string& tableName)
{
   if (tableName == m_tableName)
      return;
   m_tableName = tableName;
   TryLoadB2S(); // Re-evaluate the backglass file if none was found yet
}

void B2SServer::SetWorkingDir(const string& workingDir)
{
   m_workingDir = PluginStrings::PathFromNative(workingDir.c_str());
   TryLoadB2S();
}

void B2SServer::RegisterOverlaySheet()
{
   if (m_pagedUIApi == nullptr || m_overlaySheetRegistered || m_b2sPath.empty())
      return;
   // The plugin declared "settings" sheet replaces the implicit default page (same items, sectioned)
   PagedUISheetDef settingsSheet {};
   settingsSheet.version = 1;
   settingsSheet.id = "settings";
   settingsSheet.name = "Backglass Settings";
   settingsSheet.description = "Settings of the backglass renderer";
   settingsSheet.GetPageCount = &SettingsPageCount;
   settingsSheet.GetPageInfo = &SettingsPageInfo;
   settingsSheet.GetItemCount = &SettingsItemCount;
   settingsSheet.GetItems = &SettingsGetItems;
   m_pagedUIApi->RegisterSheet(m_endpointId, &settingsSheet);

   m_overlayDoc.path = m_b2sPath;
   m_overlayDoc.requestRefresh = [this]() { m_pagedUIApi->RequestSheetRefresh(m_endpointId, "backglass"); };
   const string name = "Edit \""s + PluginStrings::PathToUTF8(m_b2sPath.filename()) + "\"";
   PagedUISheetDef sheet {};
   sheet.version = 1;
   sheet.id = "backglass";
   sheet.name = name.c_str();
   sheet.description = "Layout of the loaded backglass document";
   DMDOverlay::InitEditSheetDef(&sheet, &m_overlayDoc);
   m_pagedUIApi->RegisterSheet(m_endpointId, &sheet);
   m_overlaySheetRegistered = true;
}


int B2SServer::GetAnimationSlowDown(const string& name) const { return B2SAnimationSlowDown(animationSlowDownsProp_Get(), name); }

int B2SServer::GetAllAnimationSlowDown() const { return allAnimationSlowDownProp_Get(); }

int B2SServer::GetUsedLEDType() const { return usedLEDTypeProp_Get(); }

static std::string CreateGuidString()
{
   std::random_device rd;
   std::mt19937_64 gen(rd());
   std::uniform_int_distribution<uint64_t> dist;

   uint64_t hi = dist(gen);
   uint64_t lo = dist(gen);

   // Set UUID version (4) and variant bits per RFC 4122
   hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
   lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

   char buf[37];
   std::snprintf(buf, sizeof(buf),
      "%08llx-%04llx-%04llx-%04llx-%012llx",
      (hi >> 32) & 0xFFFFFFFFULL,
      (hi >> 16) & 0xFFFFULL,
      hi & 0xFFFFULL,
      (lo >> 48) & 0xFFFFULL,
      lo & 0xFFFFFFFFFFFFULL);

   return std::string(buf);
}

string B2SServer::GetB2SName() const { return m_b2sName; }

void B2SServer::SetB2SName(const std::string& b2sName)
{
   if (b2sName == m_b2sName)
      return;
   m_b2sName = b2sName;
   string id = trim_string(b2sName);
   if (id.empty())
      m_controllerGameId = "b2s::" + CreateGuidString();
   else
      m_controllerGameId = "b2s::" + string_to_lower(id);

   if (m_gameRunning)
      m_exposedControllers.SetItem({ m_endpointId, m_controllerGameId.c_str() });
   else
      m_exposedControllers.ClearItems();
}

int B2SServer::OnRender(VPXRenderContext2D* ctx, void* userData)
{
   if ((ctx->window != VPXWindowId::VPXWINDOW_Backglass) && (ctx->window != VPXWindowId::VPXWINDOW_ScoreView))
      return false;

   auto me = static_cast<B2SServer*>(userData);
   if (!me->m_launchBackglass)
      return false;
   if (me->m_renderer)
      return me->m_renderer->Render(ctx, me);

   if (std::shared_ptr<B2STable> b2s = me->AcquireB2STable())
   {
      if (me->m_renderer == nullptr)
         me->m_renderer = std::make_unique<B2SRenderer>(me->m_msgApi, me->m_vpxApi, me->m_endpointId, b2s);
      return me->m_renderer->Render(ctx, me);
   }

   // Until loaded, we assume that the file will succeed loading with the expected backglass/score view
   // (a failed load invalidates the future, so it is not retried on the next frames)
   return me->m_loadedB2S.valid();
}

std::shared_ptr<B2STable> B2SServer::AcquireB2STable()
{
   std::lock_guard lock(m_b2sMutex);
   if (!m_b2s && m_loadedB2S.valid() && m_loadedB2S.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
   {
      m_b2s = m_loadedB2S.get();
      // Document editing: the live configuration starts from the loaded file
      m_overlayDoc.displays = m_b2s->m_dmdDisplays;
      for (auto& display : m_overlayDoc.displays)
         display.doc = &m_overlayDoc;
      // Artwork bounds of the edited views, used to limit the overlay rect editing ranges
      const B2SImage& backImage = m_b2s->m_backglassImage.m_image ? m_b2s->m_backglassImage : m_b2s->m_backglassOffImage.m_image ? m_b2s->m_backglassOffImage : m_b2s->m_backglassOnImage;
      if (backImage.m_image)
         if (const VPXTextureInfo* texInfo = GetTextureInfo(backImage.m_image))
         {
            m_overlayDoc.viewW[0] = texInfo->width;
            m_overlayDoc.viewH[0] = texInfo->height;
         }
      if (m_b2s->m_dmdImage.m_image)
         if (const VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_dmdImage.m_image))
         {
            m_overlayDoc.viewW[1] = texInfo->width;
            m_overlayDoc.viewH[1] = texInfo->height;
         }
      if (m_pagedUIApi != nullptr)
         m_pagedUIApi->RequestSheetRefresh(m_endpointId, "backglass");
      // Pre-register all animation-involved illumination groups so that animation group writes on the
      // render thread update existing map nodes instead of inserting while the script thread may read them
      for (const auto* animations : { &m_b2s->m_backglassAnimations, &m_b2s->m_dmdAnimations })
         for (const auto& animation : *animations)
            for (const string& group : animation.GetLightsInvolved())
               if (!group.empty())
                  m_groupStates[group];
   }
   return m_b2s;
}

void B2SServer::OnGetRenderer(const unsigned int, void* userData, void* msgData)
{
   auto me = static_cast<B2SServer*>(userData);
   auto msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   if ((msg->window == VPXWindowId::VPXWINDOW_Backglass) || (msg->window == VPXWindowId::VPXWINDOW_ScoreView))
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = me->m_ancillaryRendererDef;
      msg->count++;
   }
}

void B2SServer::ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   const ScriptClassMemberDef& member = m_controllerProxy.GetProxyClass().GetProxyClassDef()->members[memberIndex];
   const char* methodName = member.name.name;

   // Without the PinMAME plugin, forwarded calls would raise script errors and the table would never load (not even
   // allowing to fix the settings in game): they are skipped (returning defaults) and the user is told once a ROM is
   // set. Tables without a ROM (EM, originals) only call Run/Stop, whose bookkeeping below still runs
   if (!HasController())
   {
      if (!m_controllerMissingReported && member.nArgs == 1 && methodName == "GameName"sv && pArgs[0].vString.string && pArgs[0].vString.string[0])
      {
         m_controllerMissingReported = true;
         static constexpr const char* message = "This table runs a ROM through the B2S plugin, which needs the PinMAME plugin. Enable the PinMAME plugin or disable the B2S plugin in the plugin settings.";
         LOGE(message);
         m_vpxApi->PushNotification(message, 10000);
      }
   }
   else
      m_controllerProxy.ForwardCall(me, memberIndex, pArgs, pRet);

   if (methodName == "GameName"sv)
   {
      SetB2SName("");
   }
   else if (methodName == "Run"sv)
   {
      if (!m_gameRunning)
      {
         m_gameRunning = true;
         m_exposedControllers.SetItem({ m_endpointId, m_controllerGameId.c_str() });
      }
   }
   else if (methodName == "Stop"sv)
   {
      if (m_gameRunning)
      {
         m_gameRunning = false;
         m_exposedControllers.ClearItems();
      }
   }
}

// Game states

struct B2SPluginEvent
{
   uint8_t type;
   int32_t index;
   int32_t value;
};

void B2SServer::UpdateStateSrc()
{
   // Ensure that no other thread is using the exposed states while we are updating them
   m_exposedStates.ClearItems();

   {
      m_lampStateDefs.clear();
      m_lampStateIds.clear();
      m_lampStateIds.reserve(m_lampStates.size());
      for (const auto& [id, _] : m_lampStates)
         m_lampStateIds.push_back({ this, id });
      std::ranges::sort(m_lampStateIds, [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
      m_lampStateNames.resize(m_lampStates.size());
      for (size_t index = 0; index < m_lampStateIds.size(); ++index)
      {
         const auto id = m_lampStateIds[index].id;
         m_lampStateNames[index] = std::format("Illumination #{}", id);
         if (id >= 0 && id < 64 && (m_defaultStateNameMask & (1ull << id))) // Script provided ids may be out of the mask range
         {
            switch (id)
            {
            case 25: m_lampStateNames[index] = "Player #1 Active"sv; break;
            case 26: m_lampStateNames[index] = "Player #2 Active"sv; break;
            case 27: m_lampStateNames[index] = "Player #3 Active"sv; break;
            case 28: m_lampStateNames[index] = "Player #4 Active"sv; break;
            case 30: m_lampStateNames[index] = "Player Up"sv; break;
            case 31: m_lampStateNames[index] = "Can Play"sv; break;
            case 32: m_lampStateNames[index] = "Ball In Play"sv; break;
            case 33: m_lampStateNames[index] = "Tilt"sv; break;
            case 34: m_lampStateNames[index] = "Match"sv; break;
            case 35: m_lampStateNames[index] = "Game Over"sv; break;
            case 36: m_lampStateNames[index] = "Shoot Again"sv; break;
            }
         }
         m_lampStateDefs.emplace_back(StateDef {
            m_lampStateNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_CUSTOM, &m_lampStateIds[index], GetLampState, nullptr });
      }
   }

   {
      m_playerScoreStateDefs.clear();
      m_playerScoreIds.clear();
      m_playerScoreIds.reserve(m_playerScores.size());
      for (const auto& [id, _] : m_playerScores)
         m_playerScoreIds.push_back({ this, id });
      std::ranges::sort(m_playerScoreIds, [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
      m_playerScoreNames.resize(m_playerScores.size());
      for (size_t index = 0; index < m_playerScoreIds.size(); ++index)
      {
         const auto id = m_playerScoreIds[index].id;
         m_playerScoreNames[index] = std::format("Player Score #{}", id);
         if (id == 29 && (m_defaultStateNameMask & (1ull << 29)))
            m_playerScoreNames[index] = "Credits"sv;
         m_playerScoreStateDefs.emplace_back(StateDef {
            m_playerScoreNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, &m_playerScoreIds[index], GetPlayerScore, nullptr });
      }
   }

   {
      m_scoreDigitStateDefs.clear();
      m_scoreDigitIds.clear();
      m_scoreDigitIds.reserve(m_scoreDigits.size());
      for (const auto& [id, _] : m_scoreDigits)
         m_scoreDigitIds.push_back({ this, id });
      std::ranges::sort(m_scoreDigitIds, [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
      m_scoreDigitNames.resize(m_scoreDigits.size());
      for (size_t index = 0; index < m_scoreDigitIds.size(); ++index)
      {
         const auto id = m_scoreDigitIds[index].id;
         m_scoreDigitNames[index] = std::format("Digit Score #{}", id);
         m_scoreDigitStateDefs.emplace_back(StateDef {
            m_scoreDigitNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, &m_scoreDigitIds[index], GetScoreDigit, nullptr });
      }
   }

   m_exposedStates.AddItems({ //
      { .id = { m_endpointId, 1 }, //
         .name = "Illuminations",
         .desc = "Lamp states",
         .nStates = static_cast<unsigned int>(m_lampStateDefs.size()),
         .stateDefs = m_lampStateDefs.data() },
      { .id = { m_endpointId, 2 },
         .name = "Scores (players)",
         .desc = "Player score",
         .nStates = static_cast<unsigned int>(m_playerScoreStateDefs.size()),
         .stateDefs = m_playerScoreStateDefs.data() },
      { .id = { m_endpointId, 3 },
         .name = "Scores (digits)",
         .desc = "Individual digit (reel) scores",
         .nStates = static_cast<unsigned int>(m_scoreDigitStateDefs.size()),
         .stateDefs = m_scoreDigitStateDefs.data() } });
}

void MSGPIAPI B2SServer::GetLampState(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<float*>(pResult) = static_cast<float>(ctx->me->GetLampState(ctx->id));
}

void MSGPIAPI B2SServer::GetPlayerScore(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(ctx->me->GetPlayerScore(ctx->id));
}

void MSGPIAPI B2SServer::GetScoreDigit(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(ctx->me->GetScoreDigit(ctx->id));
}


// B2SSetPos

void B2SServer::B2SSetPos(int id, int xpos, int ypos)
{
   // id is a lamp ROM id (the B2SID of scripted bulbs): moves all bulbs driven by this lamp.
   // Bulbs of an illumination group locked by a running animation are not moved (reference behavior).
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (const auto* bulbs : { &b2s->m_backglassIlluminations, &b2s->m_dmdIlluminations })
      for (const auto& bulb : *bulbs)
         if (bulb->m_romId > 0 && bulb->m_romId == id && bulb->m_romIdType == B2SRomIDType::Lamp && (bulb->m_name.empty() || !IsIlluminationLocked(bulb->m_name)))
         {
            bulb->m_locationX = xpos;
            bulb->m_locationY = ypos;
         }
}


// B2SSetScore / B2SSetScorePlayer

void B2SServer::ApplyScoreDigit(int digit, int value, bool roll)
{
   if (auto it = m_scoreDigits.find(digit); it != m_scoreDigits.end())
   {
      it->second.value = value;
      it->second.roll = it->second.roll || roll;
      it->second.segMask = -1;
   }
   else
   {
      m_exposedStates.ClearItems();
      ScoreDigit& digitState = m_scoreDigits[digit];
      digitState.value = value;
      digitState.roll = roll;
      UpdateStateSrc();
   }

   B2SPluginEvent event { 'B', digit, value };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

void B2SServer::ApplyScoreSegments(int digit, int segMask)
{
   if (auto it = m_scoreDigits.find(digit); it != m_scoreDigits.end())
      it->second.segMask = segMask;
   else
   {
      m_exposedStates.ClearItems();
      m_scoreDigits[digit].segMask = segMask;
      UpdateStateSrc();
   }
}

void B2SServer::B2SSetLED(int digit, int value)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   const B2SScore* display = b2s->FindScoreDigitDisplay(digit);
   if (display == nullptr || display->m_ledSegments == 0)
      return;
   ApplyScoreSegments(digit, B2SSegmentTranslateBitCode(static_cast<uint32_t>(value), display->m_ledSegments));
}

void B2SServer::B2SSetLED(int digit, const string& value)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   const B2SScore* display = b2s->FindScoreDigitDisplay(digit);
   if (display == nullptr || display->m_ledSegments == 0)
      return;
   ApplyScoreSegments(digit, B2SSegmentCharMask(value.empty() ? ' ' : value[0], display->m_ledSegments));
}

void B2SServer::B2SSetLEDDisplay(int display, const string& text)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   const B2SScore* scoreDisplay = b2s->FindScoreDisplay(display);
   if (scoreDisplay == nullptr || scoreDisplay->m_ledSegments == 0)
      return;
   // Dream7 Text setter: chars fill the digits left to right, '.' merges into the previous digit,
   // digits beyond the text length are left unchanged
   const int dotBit = scoreDisplay->m_ledSegments == 14 ? 0x8000 : 0x0080;
   int digitIndex = 0;
   for (size_t i = 0; i < text.length() && digitIndex < scoreDisplay->m_digits; i++)
   {
      const char c = text[i];
      if (c == '.' && digitIndex > 0)
      {
         const int digit = scoreDisplay->m_resolvedStartDigit + digitIndex - 1;
         const int existing = GetScoreDigitSegments(digit);
         ApplyScoreSegments(digit, (existing >= 0 ? existing : 0) | dotBit);
         continue;
      }
      ApplyScoreSegments(scoreDisplay->m_resolvedStartDigit + digitIndex, B2SSegmentCharMask(c, scoreDisplay->m_ledSegments));
      digitIndex++;
   }
}

void B2SServer::B2SSetScore(int display, int value)
{
   // 'display' is a display id (Score/@ID): distribute the score over the display digits
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   const B2SScore* scoreDisplay = b2s->FindScoreDisplay(display);
   if (scoreDisplay == nullptr || scoreDisplay->m_digits <= 0)
      return;
   const vector<int> digits = scoreDisplay->DistributeScore(value);
   for (size_t i = 0; i < digits.size(); i++)
      ApplyScoreDigit(scoreDisplay->m_resolvedStartDigit + static_cast<int>(i), digits[i], true);
}

void B2SServer::B2SSetScoreDigit(int digit, int value) { ApplyScoreDigit(digit, value, false); }

void B2SServer::B2SSetReel(int digit, int value) { ApplyScoreDigit(digit, value, true); }

int B2SServer::GetScoreDigit(int digit) const
{
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? 0 : it->second.value.load();
}

int B2SServer::GetScoreDigitSegments(int digit) const
{
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? -1 : it->second.segMask.load();
}

bool B2SServer::ConsumeScoreDigitRoll(int digit)
{
   const auto it = m_scoreDigits.find(digit);
   return it != m_scoreDigits.end() && it->second.roll.exchange(false);
}

void B2SServer::B2SSetScorePlayer(int playerno, int score)
{
   if (auto it = m_playerScores.find(playerno); it != m_playerScores.end())
   {
      it->second = score;
   }
   else
   {
      m_exposedStates.ClearItems();
      m_playerScores[playerno] = score;
      UpdateStateSrc();
   }

   B2SPluginEvent event { 'C', playerno, score };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

int B2SServer::GetPlayerScore(int player) const
{
   // This function is exposed through:
   // - scripting API which guarantees single-threaded access on the MsgAPI thread, which is the thread where m_playerScores mutates
   // - Controller advertised state, which is accessed from any thread, but only when advertised. m_playerScores onyl mutates when states are NOT advertised
   const auto it = m_playerScores.find(player);
   return it == m_playerScores.end() ? 0 : it->second.load();
}


// B2SSetData and helpers that are allowed to:
// - change main image illumination
// - change picture box state (illumination, flipbook)
// - change reel illumination
// - start/stop animations

void B2SServer::B2SSetData(int b2sId, const string& value, bool sendPluginEvent)
{
   if (is_string_numeric(value, 0))
      B2SSetData(b2sId, string_to_int(value, 0), sendPluginEvent);
}

void B2SServer::B2SSetData(int b2sId, int value, bool sendPluginEvent)
{
   LOGD(std::format("B2SSetData {}={}", b2sId, value));

   if (auto it = m_lampStates.find(b2sId); it != m_lampStates.end())
   {
      it->second.value = static_cast<float>(value);
      it->second.stamp = ++m_lampStamp;
   }
   else
   {
      m_exposedStates.ClearItems();
      LampState& lampState = m_lampStates[b2sId];
      lampState.value = static_cast<float>(value);
      lampState.stamp = ++m_lampStamp;
      UpdateStateSrc();
   }

   if (sendPluginEvent)
   {
      B2SPluginEvent event { 'E', b2sId, value };
      m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
   }

   // Script writes in the lamp-id space also drive IDJoin animation triggers (mirrors the reference's MyB2SSetData)
   {
      std::lock_guard lock(m_animationTriggerMutex);
      if (m_pendingAnimationTriggers.size() < 1024) // Bound the queue when no backglass is rendering to drain it
         m_pendingAnimationTriggers.emplace_back(b2sId, value);
   }
}

std::vector<std::pair<int, int>> B2SServer::DrainAnimationTriggers()
{
   std::lock_guard lock(m_animationTriggerMutex);
   std::vector<std::pair<int, int>> triggers;
   triggers.swap(m_pendingAnimationTriggers);
   return triggers;
}

void B2SServer::B2SSetData(const std::string& group, const std::string& value)
{
   if (is_string_numeric(value, 0))
      B2SSetData(group, string_to_int(value, 0));
}

void B2SServer::B2SSetData(const std::string& group, int value)
{
   // Same as B2SSetData, applied to an illumination group (all bulbs sharing this name), without plugin event.
   // The reference does not check illumination locks on group writes, so animations' own group writes and
   // external group writes both apply.
   if (group.empty())
      return;
   LampState& groupState = m_groupStates[group];
   groupState.value = static_cast<float>(value);
   groupState.stamp = ++m_lampStamp;
}

float B2SServer::GetLampState(int b2sId) const
{
   const auto it = m_lampStates.find(b2sId);
   return it == m_lampStates.end() ? 0.f : it->second.value.load();
}

bool B2SServer::GetScriptedLampState(int b2sId, float& state, uint64_t* stamp) const
{
   const auto it = m_lampStates.find(b2sId);
   if (it == m_lampStates.end())
      return false;
   state = it->second.value.load();
   if (stamp != nullptr)
      *stamp = it->second.stamp.load();
   return true;
}

bool B2SServer::GetBulbState(const B2SBulb& bulb, float& state, bool* fromLampChannel) const
{
   // Resolves the effective scripted state of a bulb: most recent write wins between its lamp channel, its
   // illumination group and the ROM state (the reference routes all of them through the same shared state).
   // The lamp channel is the B2SID of scripted bulbs, or the RomID of lamp-typed ROM bulbs, as both are
   // registered in the reference's UsedRomLampIDs collection.
   // While a bulb's illumination group is locked by a running animation, only group writes apply.
   const bool locked = !bulb.m_name.empty() && IsIlluminationLocked(bulb.m_name);
   const int lampId = (bulb.m_romIdType == B2SRomIDType::Lamp && bulb.m_romId > 0) ? bulb.m_romId : -1;
   const int romValue = bulb.m_b2sId >= 0 ? bulb.m_b2sValue : 0;
   bool scripted = false;
   bool lampWon = false;
   uint64_t stamp = 0;
   // A bulb only falls under scripted control once its lamp id has actually been written; before that it keeps
   // its initial state or follows the ROM lamp channel, as the reference does. Lamp-id writes apply
   // RomInverted (value-matched writes and group writes don't).
   if (lampId > 0 && !locked)
      if (const auto it = m_lampStates.find(lampId); it != m_lampStates.end())
      {
         const float value = it->second.value.load();
         state = romValue > 0 ? (static_cast<int>(value) == romValue ? 1.f : 0.f) : ((value != 0.f) != bulb.m_romInverted ? 1.f : 0.f);
         stamp = it->second.stamp.load();
         scripted = true;
         lampWon = true;
      }
   if (!bulb.m_name.empty())
      if (const auto it = m_groupStates.find(bulb.m_name); it != m_groupStates.end() && it->second.stamp.load() > stamp)
      {
         const float value = it->second.value.load();
         state = romValue > 0 ? (static_cast<int>(value) == romValue ? 1.f : 0.f) : (value != 0.f ? 1.f : 0.f);
         stamp = it->second.stamp.load();
         scripted = true;
         lampWon = false;
      }
   // A ROM change observed after the last script write wins the channel
   if (scripted && stamp <= bulb.m_romStamp)
      scripted = false;
   if (fromLampChannel != nullptr)
      *fromLampChannel = lampWon;
   return scripted;
}

bool B2SServer::IsIlluminationLocked(const string& group) const
{
   std::lock_guard lock(m_illuminationLockMutex);
   return m_illuminationLocks.find(group) != m_illuminationLocks.end();
}

void B2SServer::StartAnimation(const string& animationName, bool reverse)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (auto* animations : { &b2s->m_backglassAnimations, &b2s->m_dmdAnimations })
      for (auto& animation : *animations)
         if (animation.m_name == animationName)
            animation.Start(reverse);
}

void B2SServer::StopAnimation(const string& animationName)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (auto* animations : { &b2s->m_backglassAnimations, &b2s->m_dmdAnimations })
      for (auto& animation : *animations)
         if (animation.m_name == animationName)
            animation.Stop();
}

void B2SServer::B2SStartRotation()
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (auto* bulbs : { &b2s->m_backglassIlluminations, &b2s->m_dmdIlluminations })
      for (auto& bulb : *bulbs)
         if (bulb->m_snippitType == B2SSnippitType::SelfRotatingImage)
            bulb->StartRotation();
}

void B2SServer::B2SStopRotation()
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (auto* bulbs : { &b2s->m_backglassIlluminations, &b2s->m_dmdIlluminations })
      for (auto& bulb : *bulbs)
         if (bulb->m_snippitType == B2SSnippitType::SelfRotatingImage)
            bulb->StopRotation();
}

// Embedded sounds: one audio source ("B2S Sounds"), one stream per sound name.
// Calls may come from the script thread or the render thread (reel rolls): stream
// mutations are always dispatched to the MsgAPI thread through RunOnMainThread.

void B2SServer::StopSoundStream(uint32_t streamResId)
{
   AudioUpdateMsg msg {};
   msg.sourceId = { m_endpointId, 0 };
   msg.streamId = { m_endpointId, streamResId };
   msg.buffer = nullptr; // Immediate stream destruction
   m_msgApi->BroadcastMsg(m_endpointId, m_onAudioUpdateId, &msg);
}

void B2SServer::B2SStartSound(const string& soundName) { B2SPlaySound(soundName); }

void B2SServer::B2SPlaySound(const string& soundName)
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (const B2SSound& sound : b2s->m_sounds)
   {
      if (sound.m_name != soundName || sound.m_wav == nullptr)
         continue;
      WavData wav;
      if (!DecodeWav(*sound.m_wav, wav))
      {
         LOGW("B2SPlaySound: unsupported WAV format for sound '"s + soundName + "'"s);
         return;
      }
      struct PlayCtx
      {
         B2SServer* me;
         string name;
         WavData wav;
      };
      m_msgApi->RunOnMainThread(
         m_endpointId, 0.,
         [](void* userData)
         {
            PlayCtx* const ctx = static_cast<PlayCtx*>(userData);
            B2SServer* const me = ctx->me;
            // Restart semantics: an already playing sound is stopped then replayed (as the reference does)
            if (const auto it = me->m_soundStreams.find(ctx->name); it != me->m_soundStreams.end())
               me->StopSoundStream(it->second);
            AudioUpdateMsg msg {};
            msg.sourceId = { me->m_endpointId, 0 };
            msg.streamId = { me->m_endpointId, me->m_nextSoundStreamId };
            msg.channelFormat = (ctx->wav.channels == 1) ? CTLPI_AUDIO_FORMAT_CHANNEL_MONO : CTLPI_AUDIO_FORMAT_CHANNEL_STEREO;
            msg.sampleFormat = ctx->wav.isFloat ? CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT : CTLPI_AUDIO_FORMAT_SAMPLE_INT16;
            msg.sampleRate = ctx->wav.sampleRate;
            msg.volume = 1.f;
            msg.bufferSize = static_cast<unsigned int>(ctx->wav.pcm.size());
            msg.buffer = ctx->wav.pcm.data();
            me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_onAudioUpdateId, &msg);
            me->m_soundStreams[ctx->name] = me->m_nextSoundStreamId++;
            delete ctx;
         },
         new PlayCtx { this, soundName, std::move(wav) });
      return;
   }
}

void B2SServer::B2SStopSound(const string& soundName)
{
   struct StopCtx
   {
      B2SServer* me;
      string name;
   };
   m_msgApi->RunOnMainThread(
      m_endpointId, 0.,
      [](void* userData)
      {
         StopCtx* const ctx = static_cast<StopCtx*>(userData);
         B2SServer* const me = ctx->me;
         if (const auto it = me->m_soundStreams.find(ctx->name); it != me->m_soundStreams.end())
         {
            me->StopSoundStream(it->second);
            me->m_soundStreams.erase(it);
         }
         delete ctx;
      },
      new StopCtx { this, soundName });
}

void B2SServer::B2SStopAllAnimations()
{
   const std::shared_ptr<B2STable> b2s = AcquireB2STable();
   if (b2s == nullptr)
      return;
   for (auto* animations : { &b2s->m_backglassAnimations, &b2s->m_dmdAnimations })
      for (auto& animation : *animations)
         animation.Stop();
}

bool B2SServer::GetB2SIsAnimationRunning(const string& animationName) const
{
   std::lock_guard lock(m_b2sMutex);
   if (m_b2s == nullptr)
      return false;
   for (const auto* animations : { &m_b2s->m_backglassAnimations, &m_b2s->m_dmdAnimations })
      for (const auto& animation : *animations)
         if (animation.m_name == animationName)
            return animation.IsRunning();
   return false;
}

void B2SServer::PulseSwitch(int switchId)
{
   if (m_setSwitchIndex < 0 || !HasController())
      return;
   // Pulse the switch on the message API thread: set on, then released after 200ms (as the reference implementation)
   const auto thunk = [](void* userData)
   {
      const auto ctx = static_cast<std::pair<B2SServer*, int>*>(userData);
      ScriptVariant args[2];
      args[0].vInt = ctx->second < 0 ? -ctx->second : ctx->second;
      args[1].vBool = ctx->second >= 0 ? 1 : 0;
      ctx->first->m_controllerProxy.ForwardCall(ctx->first, ctx->first->m_setSwitchIndex, args, nullptr);
      delete ctx;
   };
   m_msgApi->RunOnMainThread(m_endpointId, 0., thunk, new std::pair<B2SServer*, int> { this, switchId });
   m_msgApi->RunOnMainThread(m_endpointId, 0.2, thunk, new std::pair<B2SServer*, int> { this, -switchId });
}

B2SAnimationEffects B2SServer::GetAnimationEffects()
{
   B2SAnimationEffects fx;
   fx.setGroup = [this](const string& group, bool on)
   {
      if (const auto it = m_groupStates.find(group); it != m_groupStates.end())
      {
         it->second.value = on ? 1.f : 0.f;
         it->second.stamp = ++m_lampStamp;
      }
   };
   fx.getGroup = [this](const string& group) -> float
   {
      const auto it = m_groupStates.find(group);
      return it == m_groupStates.end() ? 0.f : it->second.value.load();
   };
   fx.lockGroup = [this](const string& group)
   {
      std::lock_guard lock(m_illuminationLockMutex);
      m_illuminationLocks[group]++;
   };
   fx.unlockGroup = [this](const string& group)
   {
      std::lock_guard lock(m_illuminationLockMutex);
      if (const auto it = m_illuminationLocks.find(group); it != m_illuminationLocks.end())
      {
         if (it->second > 1)
            it->second--;
         else
            m_illuminationLocks.erase(it);
      }
   };
   fx.pulseSwitch = [this](int switchId) { PulseSwitch(switchId); };
   fx.setScoreDisplaysHidden = [this](bool hidden) { m_scoreDisplaysHidden = hidden; };
   fx.allLightsOff = [this]()
   {
      for (auto& [id, lampState] : m_lampStates)
      {
         lampState.value = 0.f;
         lampState.stamp = ++m_lampStamp;
      }
      for (auto& [group, groupState] : m_groupStates)
      {
         groupState.value = 0.f;
         groupState.stamp = ++m_lampStamp;
      }
   };
   fx.snapshotAllLights = [this]() -> std::unordered_map<string, float>
   {
      std::unordered_map<string, float> snapshot;
      for (const auto& [id, lampState] : m_lampStates)
         snapshot["#"s + std::to_string(id)] = lampState.value.load();
      for (const auto& [group, groupState] : m_groupStates)
         snapshot[group] = groupState.value.load();
      return snapshot;
   };
   fx.restoreAllLights = [this](const std::unordered_map<string, float>& snapshot)
   {
      for (const auto& [key, value] : snapshot)
      {
         if (!key.empty() && key[0] == '#')
         {
            if (const auto it = m_lampStates.find(string_to_int(key.substr(1), 0)); it != m_lampStates.end())
            {
               it->second.value = value;
               it->second.stamp = ++m_lampStamp;
            }
         }
         else if (const auto it = m_groupStates.find(key); it != m_groupStates.end())
         {
            it->second.value = value;
            it->second.stamp = ++m_lampStamp;
         }
      }
   };
   return fx;
}
}
