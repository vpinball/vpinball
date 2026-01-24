// license:GPLv3+

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "LibAv.h"

#include <filesystem>
#include <fstream>

namespace PUP {

MSGPI_FLOAT_VAL_SETTING(pupMainVolume, "MainVol", "Main Volume", "Overall volume", true, 0.f, 1.f, 0.01f, 1.f);

MSGPI_INT_VAL_SETTING(pupBGPadLeft, "BGPadLeft", "Backglass Left Pad", "Left Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadRight, "BGPadRight", "Backglass Right Pad", "Right Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadTop, "BGPadTop", "Backglass Top Pad", "Top Padding of backglass", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupBGPadBottom, "BGPadBottom", "Backglass Bottom Pad", "Bottom Padding of backglass", true, 0, 4096, 0);
MSGPI_STRING_VAL_SETTING(pupBGFrameOverlayPath, "BGFrameOverlay", "Backglass Frame Overlay", "Path to an image that will be rendered as an ovelray on the backglass display", true, "", 1024);

MSGPI_INT_VAL_SETTING(pupSVPadLeft, "SVPadLeft", "Score View Left Pad", "Left Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadRight, "SVPadRight", "Score View Right Pad", "Right Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadTop, "SVPadTop", "Score View Top Pad", "Top Padding of Score View", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupSVPadBottom, "SVPadBottom", "Score View Bottom Pad", "Bottom Padding of Score View", true, 0, 4096, 0);
MSGPI_STRING_VAL_SETTING(pupSVFrameOverlayPath, "SVFrameOverlay", "Score View Frame Overlay", "Path to an image that will be rendered as an ovelray on the Score View display", true, "", 1024);

MSGPI_INT_VAL_SETTING(pupTopperPadLeft, "TopperPadLeft", "Topper Left Pad", "Left Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadRight, "TopperPadRight", "Topper Right Pad", "Right Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadTop, "TopperPadTop", "Topper Top Pad", "Top Padding of topper", true, 0, 4096, 0);
MSGPI_INT_VAL_SETTING(pupTopperPadBottom, "TopperPadBottom", "Topper Bottom Pad", "Bottom Padding of topper", true, 0, 4096, 0);
MSGPI_STRING_VAL_SETTING(pupTopperFrameOverlayPath, "TopperFrameOverlay", "Topper Frame Overlay", "Path to an image that will be rendered as an ovelray on the topper display", true, "", 1024);

PUPManager::PUPManager(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::filesystem::path& rootPath)
   : m_szRootPath(rootPath)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
{
   msgApi->RegisterSetting(endpointId, &pupMainVolume);
   m_mainVolume = pupMainVolume_Get();
   msgApi->RegisterSetting(endpointId, &pupBGPadLeft);
   msgApi->RegisterSetting(endpointId, &pupBGPadRight);
   msgApi->RegisterSetting(endpointId, &pupBGPadTop);
   msgApi->RegisterSetting(endpointId, &pupBGPadBottom);
   //msgApi->RegisterSetting(endpointId, &pupBGFrameOverlayPath);
   msgApi->RegisterSetting(endpointId, &pupSVPadLeft);
   msgApi->RegisterSetting(endpointId, &pupSVPadRight);
   msgApi->RegisterSetting(endpointId, &pupSVPadTop);
   msgApi->RegisterSetting(endpointId, &pupSVPadBottom);
   //msgApi->RegisterSetting(endpointId, &pupSVFrameOverlayPath);
   msgApi->RegisterSetting(endpointId, &pupTopperPadLeft);
   msgApi->RegisterSetting(endpointId, &pupTopperPadRight);
   msgApi->RegisterSetting(endpointId, &pupTopperPadTop);
   msgApi->RegisterSetting(endpointId, &pupTopperPadBottom);
   //msgApi->RegisterSetting(endpointId, &pupTopperFrameOverlayPath);
   m_msgApi->SubscribeMsg(m_endpointId, m_getAuxRendererId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);

   m_msgApi->BroadcastMsg(m_endpointId, m_getVpxApiId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &m_vpxApi);
}

PUPManager::~PUPManager()
{
   Unload();
   m_msgApi->UnsubscribeMsg(m_getAuxRendererId, OnGetRenderer);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_getAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
   m_msgApi->FlushPendingCallbacks(m_endpointId);
   m_msgApi->ReleaseMsgID(m_getVpxApiId);
}

void PUPManager::SetGameDir(const string& szRomName)
{
   std::filesystem::path path;

   // First search for pupvideos along the table file
   if (m_vpxApi != nullptr)
   {
      VPXTableInfo tableInfo;
      m_vpxApi->GetTableInfo(&tableInfo);
      std::filesystem::path tablePath = tableInfo.path;
      path = find_case_insensitive_directory_path(tablePath.parent_path() / "pupvideos" / szRomName);
   }

   // If we did not find the pup folder along the table, search for it in the global 'pupvideos' path if defined
   if (path.empty() && !m_szRootPath.empty())
      path = find_case_insensitive_directory_path(m_szRootPath / szRomName);

   if (path.empty())
      return;

   if (path == m_szPath)
      return;

   std::lock_guard lock(m_queueMutex);

   m_szPath = path;
   LOGI("PUP path: %s", m_szPath.string().c_str());

   // Load Fonts
   LoadFonts();
}

void PUPManager::LoadConfig(const string& szRomName)
{
   Unload();

   SetGameDir(szRomName);

   // Set game dir will define the path to the pup files, or empty it if not found
   if (m_szPath.empty())
      return;

   // Load playlists
   LoadPlaylists();

   // Load Fonts
   LoadFonts();

   // Load screens and start them
   if (std::filesystem::path szScreensPath = find_case_insensitive_file_path(m_szPath / "screens.pup"); !szScreensPath.empty())
   {
      std::ifstream screensFile;
      screensFile.open(szScreensPath, std::ifstream::in);
      if (screensFile.is_open()) {
         string line;
         int i = 0;
         while (std::getline(screensFile, line)) {
            if (++i == 1)
               continue;
            std::unique_ptr<PUPScreen> pScreen = PUPScreen::CreateFromCSV(this, line, m_playlists);
            if (pScreen)
               AddScreen(std::move(pScreen));
         }
      }
      else {
         LOGE("Unable to load %s", szScreensPath.c_str());
      }
   }
   else {
      LOGI("No screens.pup file found");
   }

   // Queue initial game event
   QueueTriggerData({ 'D', 0, 1 });
}

void PUPManager::Unload()
{
   Stop();

   m_screenOrder.clear();
   m_screenMap.clear();

   UnloadFonts();

   for (auto& playlist : m_playlists)
      delete playlist;
   m_playlists.clear();

   m_dmd = nullptr;

   m_szPath.clear();
}

void PUPManager::UnloadFonts()
{
   for (auto& pFont : m_fonts)
      TTF_CloseFont(pFont);
   m_fonts.clear();
   m_fontMap.clear();
   m_fontFilenameMap.clear();
}

void PUPManager::LoadFonts()
{
   UnloadFonts();
   std::filesystem::path szFontsPath = find_case_insensitive_directory_path(m_szPath / "FONTS");
   if (!szFontsPath.empty())
   {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath))
      {
         if (entry.is_regular_file())
         {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf")
            {
               if (TTF_Font* pFont = TTF_OpenFont(szFontPath.c_str(), 8))
               {
                  AddFont(pFont, entry.path().filename().string());
               }
               else
               {
                  LOGE("Failed to load font: %s %s", szFontPath.c_str(), SDL_GetError());
               }
            }
         }
      }
   }
   else
   {
      LOGI("No FONTS folder found");
   }
}

void PUPManager::LoadPlaylists()
{
   std::filesystem::path szPlaylistsPath = find_case_insensitive_file_path(GetPath() / "playlists.pup");
   std::ifstream playlistsFile;
   playlistsFile.open(szPlaylistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      ankerl::unordered_dense::set<std::string> lowerPlaylistNames;
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;
         PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSV(this, line);
         if (pPlaylist) {
            string folderNameLower = lowerCase(pPlaylist->GetFolder());
            if (lowerPlaylistNames.find(folderNameLower) == lowerPlaylistNames.end()) {
               m_playlists.push_back(pPlaylist);
               lowerPlaylistNames.insert(folderNameLower);
            }
            else {
               LOGE("Duplicate playlist: playlist=%s", pPlaylist->ToString().c_str());
               delete pPlaylist;
            }
         }
      }
   }
}

bool PUPManager::AddScreen(std::shared_ptr<PUPScreen> pScreen)
{
   std::unique_lock lock(m_queueMutex);

   if (std::shared_ptr<PUPScreen> existing = GetScreen(pScreen->GetScreenNum()); existing)
   {
      LOGI("Replacing previously defined PUP screen: existing={%s} ne<={%s}", existing->ToString(false).c_str(), pScreen->ToString(false).c_str());
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

   const std::unique_ptr<PUPCustomPos>& pCustomPos = pScreen->GetCustomPos();
   if (pCustomPos) {
      const auto it = m_screenMap.find(pCustomPos->GetSourceScreen());
      std::shared_ptr<PUPScreen> parent;
      if (it != m_screenMap.end()) {
         parent = it->second;
      }
      else {
         lock.unlock();
         switch (pCustomPos->GetSourceScreen()) {
         case 0: parent = std::move(PUPScreen::CreateFromCSV(this, R"(0,"Topper","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 1: parent = std::move(PUPScreen::CreateFromCSV(this, R"(1,"DMD","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 2: parent = std::move(PUPScreen::CreateFromCSV(this, R"(2,"Backglass","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 3: parent = std::move(PUPScreen::CreateFromCSV(this, R"(3,"Playfield","",,0,Off,0,)"s, m_playlists)); break;
         case 4: parent = std::move(PUPScreen::CreateFromCSV(this, R"(4,"Music","",,0,MusicOnly,0,)"s, m_playlists)); break;
         case 5: parent = std::move(PUPScreen::CreateFromCSV(this, R"(5,"FullDMD","",,0,ForceBack,0,)"s, m_playlists)); break;
         }
         if (parent)
            AddScreen(parent);
         lock.lock();
      }
      if (parent && pScreen != parent)
         parent->AddChild(pScreen);
   }

   if (!m_isRunning) {
      lock.unlock();
      Start();
   }

   LOGI("Screen added: screen={%s}", pScreen->ToString().c_str());

   return true;
}

bool PUPManager::AddScreen(int screenNum)
{
   std::unique_ptr<PUPScreen> pScreen = PUPScreen::CreateDefault(this, screenNum, m_playlists);
   if (!pScreen)
      return false;

   return AddScreen(std::move(pScreen));
}

void PUPManager::SendScreenToBack(const PUPScreen* screen)
{
   LOGD("Send screen to back %d", screen->GetScreenNum());
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
   LOGD("Send screen to front %d", screen->GetScreenNum());
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
      LOGE("Screen not found: screenNum=%d", screenNum);
   }
   return nullptr;
}

bool PUPManager::AddFont(TTF_Font* pFont, const string& szFilename)
{
   if (!pFont)
      return false;

   m_fonts.push_back(pFont);

   const string szFamilyName = string(TTF_GetFontFamilyName(pFont));

   const string szNormalizedFamilyName = lowerCase(string_replace_all(szFamilyName, "  "s, ' '));
   m_fontMap[szNormalizedFamilyName] = pFont;

   string szStyleName = string(TTF_GetFontStyleName(pFont));
   if (szStyleName != "Regular")
   {
      const string szFullName = szFamilyName + ' ' + szStyleName;
      const string szNormalizedFullName = lowerCase(string_replace_all(szFullName, "  "s, ' '));
      m_fontMap[szNormalizedFullName] = pFont;
   }

   const string szNormalizedFilename = lowerCase(szFilename.substr(0, szFilename.length() - 4));
   m_fontFilenameMap[szNormalizedFilename] = pFont;

   LOGI("Font added: familyName=%s, styleName=%s, filename=%s", szFamilyName.c_str(), szStyleName.c_str(), szFilename.c_str());

   return true;
}

TTF_Font* PUPManager::GetFont(const string& szFont)
{
   string szNormalizedFamilyName = lowerCase(string_replace_all(szFont, "  "s, ' '));

   ankerl::unordered_dense::map<string, TTF_Font*>::const_iterator it = m_fontMap.find(szNormalizedFamilyName);
   if (it != m_fontMap.end())
      return it->second;
   it = m_fontFilenameMap.find(lowerCase(szFont));
   if (it != m_fontFilenameMap.end())
      return it->second;

   return nullptr;
}

void PUPManager::QueueTriggerData(PUPTriggerData data)
{
   if (data.value == 0)
      return;
   {
      std::lock_guard lock(m_queueMutex);
      m_triggerDataQueue.push_back(data);
   }
   m_queueCondVar.notify_one();
}

void PUPManager::ProcessQueue()
{
   SetThreadName("PUPManager.ProcessQueue"s);
   while (m_isRunning)
   {
      std::unique_lock lock(m_queueMutex);
      m_queueCondVar.wait_for(lock, std::chrono::microseconds(16666), [this] { return !m_triggerDataQueue.empty() || !m_triggerDmdQueue.empty() || !m_isRunning; });

      if (!m_isRunning)
      {
         while (!m_triggerDmdQueue.empty())
         {
            delete[] m_triggerDmdQueue.front();
            m_triggerDmdQueue.pop();
         }
         break;
      }

      int dmdTrigger = -1;
      while (!m_triggerDmdQueue.empty())
      {
         const uint8_t* const __restrict frame = m_triggerDmdQueue.front();
         m_triggerDmdQueue.pop();

         // Unsupported frame format ? just flush the queue
         if (m_dmdId.identifyFormat != CTLPI_DISPLAY_ID_FORMAT_BITPLANE2 && m_dmdId.identifyFormat != CTLPI_DISPLAY_ID_FORMAT_BITPLANE4)
         {
            delete[] frame;
            continue;
         }

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
            m_dmd->Load(m_szPath.string().c_str(), "", m_dmdId.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE2 ? 2 : 4);
            memset(m_idFrame, 0, sizeof(m_idFrame));
         }

         // Reproduce legacy behavior for backward compatibility (scaling, padding, coloring)
         // This is very hacky and should be replaced by identification against the raw identify frame.
         if (m_dmdId.width == 128 && m_dmdId.height == 32)
         {
            dmdTrigger = m_dmd->MatchIndexed(frame, 128, 32);
         }
         else if (m_dmdId.width == 128 && m_dmdId.height == 16)
         {
            memcpy(&m_idFrame[128 * 8], frame, 128 * 16);
            dmdTrigger = m_dmd->MatchIndexed(m_idFrame, 128, 32);
         }
         else if (m_dmdId.width <= 256 && m_dmdId.height == 64)
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
            const unsigned int ofsX = (128 - (m_dmdId.width / 2)) / 2;
            for (unsigned int y = 0; y < 32; y++)
            {
               for (unsigned int x = 0; x < m_dmdId.width / 2; x++)
               {
                  float lum = 0., sum = 0.;
                  constexpr int radius = 1;
                  for (int dx = 1 - radius; dx <= radius; dx++)
                  {
                     for (int dy = 1 - radius; dy <= radius; dy++)
                     {
                        const int px = x * 2 + dx;
                        const int py = y * 2 + dy;
                        const float weight = radius * radius - fabsf((float)dx - 0.5f) * fabsf((float)dy - 0.5f);
                        if (/*px >= 0 &&*/ static_cast<unsigned int>(px) < m_dmdId.width
                           && /*py >= 0 &&*/ static_cast<unsigned int>(py) < m_dmdId.height) // unsigned int tests include the >= 0 ones
                           lum += static_cast<float>(frame[py * m_dmdId.width + px]) * weight;
                        sum += weight;
                     }
                  }
                  m_idFrame[y * 128 + ofsX + x] = (int)roundf(lum / sum);
               }
            }
            dmdTrigger = m_dmd->MatchIndexed(m_idFrame, 128, 32);
         }
         else
         {
            // Unsupported DMD format (would need to implement a dedicated stretch fit, matching what is used elsewhere)
         }
         delete[] frame;

         if (dmdTrigger == 0) // 0 is unmatched for libpupdmd, but D0 is init trigger for PUP
            dmdTrigger = -1;
         else
         {
            // Broadcast event on plugin message bus (avoid holding any reference as we don't know when this event will be processed and maybe the manager will be deleted by then)
            struct DmdEvent
            {
               const MsgPluginAPI* msgApi;
               uint32_t endpointId;
               unsigned int onDmdTriggerId;
               int dmdTrigger;
            };
            DmdEvent* event = new DmdEvent();
            *event = { m_msgApi, m_endpointId, m_onDmdTriggerId, dmdTrigger };
            m_msgApi->RunOnMainThread(m_endpointId, 0.0, [](void* userData) {
               DmdEvent* event = static_cast<DmdEvent*>(userData);
               event->msgApi->BroadcastMsg(event->endpointId, event->onDmdTriggerId, &event->dmdTrigger);
               delete event;
            }, event);
         }
      }

      for (const auto& [key, screen] : m_screenMap)
      {
         for (auto& [cmd, triggers] : screen->GetTriggers())
         {
            bool wasTriggered = triggers[0]->IsTriggered();
            for (auto& trigger : triggers[0]->GetTriggers())
            {
               // Trigger value automatically get back to 0 unless held (for example by a controller switch)
               trigger.m_value = 0;

               switch (trigger.m_type)
               {
               case 'W': // PinMAME switch state
                  for (unsigned int i = 0; i < m_pinmameInputSrc.nInputs; i++)
                     if (m_pinmameInputSrc.inputDefs[i].groupId == 0x0001 && m_pinmameInputSrc.inputDefs[i].deviceId == trigger.m_number)
                        trigger.m_value = m_pinmameInputSrc.GetInputState(i) ? 1 : 0;
                  break;
               case 'N': // PinMAME mech state
                  // FIXME implement
                  break;
               case 'L': // PinMAME lamp state
                  if (0 < trigger.m_number && static_cast<unsigned int>(trigger.m_number) <= m_nPMLamps)
                     trigger.m_value = m_pinmameDevSrc.GetFloatState(m_PMLampIndex + trigger.m_number - 1) > 0.5f ? 1 : 0;
                  break;
               case 'S': // PinMAME solenoid state
                  if (0 < trigger.m_number && static_cast<unsigned int>(trigger.m_number) <= m_nPMSolenoids)
                     trigger.m_value = m_pinmameDevSrc.GetFloatState(m_PMSolenoidIndex + trigger.m_number - 1) > 0.5f ? 1 : 0;
                  break;
               case 'G': // PinMAME GI state
                  // FIXME likely needs legacy value (0..8 for WPC, 0/9 for others) for backward compatibility
                  if (0 < trigger.m_number && static_cast<unsigned int>(trigger.m_number) <= m_nPMGIs)
                     trigger.m_value = m_pinmameDevSrc.GetFloatState(m_PMGIIndex + trigger.m_number - 1) > 0.5f ? 1 : 0;
                  break;
               case 'D': // PinMAME Segment display state (also DMD frame identification Id implemented above)
                  // FIXME implement alphanum segments
                  trigger.m_value = (trigger.m_number == dmdTrigger) ? 1 : 0;
                  break;
               case 'E': // B2S Controller generic input state
                  // FIXME implement
                  break;
               case 'B': // B2S Controller score digit
                  // FIXME implement
                  break;
               case 'C': // B2S Controller score
                  // FIXME implement
                  break;
               }
               // Apply triggers defined through scripting, after controller events to allow overriding them
               for (const auto& triggerData : m_triggerDataQueue)
                  if ((trigger.m_type == triggerData.type) && (trigger.m_number == triggerData.number))
                     trigger.m_value = triggerData.value;
            }
            bool isTriggered = triggers[0]->IsTriggered();
            if (isTriggered && !wasTriggered)
            {
               for (auto trigger : triggers)
               {
                  // Dispatch trigger action to main thread
                  m_msgApi->RunOnMainThread(m_endpointId, 0.0, [](void* userData) { 
                     static_cast<PUPTrigger*>(userData)->Trigger()();
                     }, trigger);
               }
            }
         }
      }

      // Clear script triggers
      m_triggerDataQueue.clear();
   }
}

void PUPManager::Start()
{
   if (m_isRunning)
      return;

   LOGI("PUP start");

   m_isRunning = true;
   m_thread = std::thread(&PUPManager::ProcessQueue, this);

   // Subscribe to message bus events
   m_getDmdSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_onSerumTriggerId = m_msgApi->GetMsgID("Serum", "OnDmdTrigger");
   m_onDmdTriggerId = m_msgApi->GetMsgID("PinUp", "OnDmdTrigger");

   m_getDevSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   m_onDevSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);

   m_getInputSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
   m_onInputSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG);

   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDevSrcChangedId, OnDevSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onInputSrcChangedId, OnInputSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSerumTriggerId, OnSerumTrigger, this);
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
   OnDevSrcChanged(m_onDevSrcChangedId, this, nullptr);
   OnInputSrcChanged(m_onInputSrcChangedId, this, nullptr);
   
   assert(m_pollDmdContext == nullptr);
   m_pollDmdContext = new PollDmdContext(this);
   OnPollDmd(m_pollDmdContext);
}

void PUPManager::Stop()
{
   if (!m_isRunning)
      return;

   assert(m_pollDmdContext);
   m_pollDmdContext->valid = false;
   m_pollDmdContext = nullptr;

   {
      std::lock_guard lock(m_queueMutex);
      m_isRunning = false;
   }

   m_queueCondVar.notify_all();
   if (m_thread.joinable())
      m_thread.join();

   m_msgApi->UnsubscribeMsg(m_onDmdSrcChangedId, OnDMDSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onDevSrcChangedId, OnDevSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onInputSrcChangedId, OnInputSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onSerumTriggerId, OnSerumTrigger);
   delete[] m_b2sInputSrc.inputDefs;
   memset(&m_b2sInputSrc, 0, sizeof(m_b2sInputSrc));
   delete[] m_pinmameInputSrc.inputDefs;
   memset(&m_pinmameInputSrc, 0, sizeof(m_pinmameInputSrc));
   delete[] m_pinmameDevSrc.deviceDefs;
   memset(&m_pinmameDevSrc, 0, sizeof(m_pinmameDevSrc));
   m_nPMSolenoids = 0;
   m_PMGIIndex = -1;
   m_nPMGIs = 0;
   m_PMLampIndex = -1;
   m_nPMLamps = 0;

   m_msgApi->ReleaseMsgID(m_getDevSrcId);
   m_msgApi->ReleaseMsgID(m_onDevSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getInputSrcId);
   m_msgApi->ReleaseMsgID(m_onInputSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getDmdSrcId);
   m_msgApi->ReleaseMsgID(m_onDmdSrcChangedId);
   m_msgApi->ReleaseMsgID(m_onSerumTriggerId);
   m_msgApi->ReleaseMsgID(m_onDmdTriggerId);
}

int PUPManager::Render(VPXRenderContext2D* const renderCtx, void* context)
{
   PUPManager* me = static_cast<PUPManager*>(context);

   if (float volume = pupMainVolume_Get(); volume != me->m_mainVolume)
   {
      me->m_mainVolume = volume;
      for (const auto& [key, screen] : me->m_screenMap)
         screen->SetMainVolume(volume);
   }

   int padLeft = 0;
   int padRight = 0;
   int padTop = 0;
   int padBottom = 0;
   std::shared_ptr<PUPScreen> rootScreen = nullptr;
   switch (renderCtx->window)
   {
   case VPXWindowId::VPXWINDOW_Topper:
      rootScreen = me->GetScreen(0);
      padLeft = pupTopperPadLeft_Get();
      padRight = pupTopperPadRight_Get();
      padTop = pupTopperPadTop_Get();
      padBottom = pupTopperPadBottom_Get();
      break;
   case VPXWindowId::VPXWINDOW_Backglass:
      rootScreen = me->GetScreen(2);
      padLeft = pupBGPadLeft_Get();
      padRight = pupBGPadRight_Get();
      padTop = pupBGPadTop_Get();
      padBottom = pupBGPadBottom_Get();
      break;
   case VPXWindowId::VPXWINDOW_ScoreView:
      rootScreen = me->GetScreen(5); // TODO select 1 or 5 (user settings ?)
      padLeft = pupSVPadLeft_Get();
      padRight = pupSVPadRight_Get();
      padTop = pupSVPadTop_Get();
      padBottom = pupSVPadBottom_Get();
      break;
   default: break;
   }
   if (rootScreen == nullptr || rootScreen->GetCustomPos() != nullptr)
      return false;

   if (!LibAV::LibAV::GetInstance().isLoaded)
      return false;

   if (me->m_vpxApi)
   {
      double gameTime = me->m_vpxApi->GetGameTime();
      for (const auto& [key, screen] : me->m_screenMap)
         screen->SetGameTime(gameTime);
   }

   renderCtx->srcWidth = renderCtx->outWidth;
   renderCtx->srcHeight = renderCtx->outHeight;
   rootScreen->SetBounds(padLeft, padTop, static_cast<int>(renderCtx->srcWidth) - padLeft - padRight, static_cast<int>(renderCtx->srcHeight) - padTop - padBottom);

   // Sort background screens before other screen, this is done on every render as state may have changed (end of main play and resume of background play)
   std::stable_partition(me->m_screenOrder.begin(), me->m_screenOrder.end(), [](const auto& a) { return a->IsBackgroundPlaying(); });

   // Helper to log screen order to help debug rendering order
   if (false && renderCtx->window == VPXWindowId::VPXWINDOW_Backglass)
   {
      std::stringstream ss;
      for (auto screen : me->m_screenOrder)
      {
         const PUPScreen* parent = screen.get();
         while (parent && parent != rootScreen.get())
            parent = parent->GetParent();
         if (parent && screen->GetMode() != PUPScreen::Mode::Off && screen->GetMode() != PUPScreen::Mode::MusicOnly)
            ss << (ss.str().empty() ? "" : ", ") << screen->GetScreenNum() << (screen->IsBackgroundPlaying() ? 'B' : 'F');
      }
      LOGD("PUP Screen order: %s", ss.str().c_str());
   }

   // Render all children of rootScreen according to the global shared render order
   // This is done in 2 passes:
   // - first the video
   // - overlays
   // - active labels
   for (int pass = 0; pass < 3; pass++)
   {
      for (auto screen : me->m_screenOrder)
      {
         const PUPScreen* parent = screen.get();
         while (parent && parent != rootScreen.get())
            parent = parent->GetParent();
         if (parent)
            screen->Render(renderCtx, pass);
      }
   }

   return true;
}

void PUPManager::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   PUPManager* me = static_cast<PUPManager*>(context);
   GetAncillaryRendererMsg* msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   static constexpr AncillaryRendererDef entry = { "PUP", "PinUp Player", "Renderer for PinUp player backglass", nullptr, Render };
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


///////////////////////////////////////////////////////////////////////////////
// State polling for DMD keyframe identification and trigger detection
//

// Poll for an identify frame at least every 60 times per seconds
void PUPManager::OnPollDmd(void* userData)
{
   PollDmdContext* ctx = static_cast<PollDmdContext*>(userData);
   if (!ctx->valid)
   {
      // End of polling (we own the context object, so delete it)
      delete ctx;
      return;
   }
   std::lock_guard lock(ctx->manager->m_queueMutex);
   if (ctx->manager->m_dmdId.id.id != 0 && ctx->manager->m_dmdId.GetIdentifyFrame)
   {
      DisplayFrame idFrame = ctx->manager->m_dmdId.GetIdentifyFrame(ctx->manager->m_dmdId.id);
      if (idFrame.frameId != ctx->manager->m_lastFrameId && idFrame.frame)
      {
         ctx->manager->m_lastFrameId = idFrame.frameId;
         uint8_t* frame = new uint8_t[ctx->manager->m_dmdId.width * ctx->manager->m_dmdId.height];
         memcpy(frame, idFrame.frame, ctx->manager->m_dmdId.width * ctx->manager->m_dmdId.height);
         ctx->manager->m_triggerDmdQueue.push(frame);
         ctx->manager->m_queueCondVar.notify_one();
      }
   }
   ctx->manager->m_msgApi->RunOnMainThread(ctx->manager->m_endpointId, 1.0 / 60.0, OnPollDmd, ctx);
}

// Broadcasted by Serum plugin when frame triggers are identified
void PUPManager::OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   unsigned int* trigger = static_cast<unsigned int*>(eventData);
   me->QueueTriggerData({ 'D', static_cast<int>(*trigger), 1 });
}

void PUPManager::OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   std::lock_guard lock(me->m_queueMutex);
   me->m_dmdId.id.id = 0;
   unsigned int largest = 128;
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDmdSrcId, &getSrcMsg);
   getSrcMsg = { getSrcMsg.count, 0, new DisplaySrcId[getSrcMsg.count] };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDmdSrcId, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].width >= largest && getSrcMsg.entries[i].GetIdentifyFrame)
      {
         me->m_dmdId = getSrcMsg.entries[i];
         largest = getSrcMsg.entries[i].width;
      }
   }
   delete[] getSrcMsg.entries;
}

void PUPManager::OnDevSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   std::lock_guard lock(me->m_queueMutex);
   delete[] me->m_pinmameDevSrc.deviceDefs;
   me->m_PMSolenoidIndex = -1;
   me->m_nPMSolenoids = 0;
   me->m_PMGIIndex = -1;
   me->m_nPMGIs = 0;
   me->m_PMLampIndex = -1;
   me->m_nPMLamps = 0;
   memset(&me->m_pinmameDevSrc, 0, sizeof(me->m_pinmameDevSrc));
   GetDevSrcMsg getSrcMsg = { 1024, 0, new DevSrcId[1024] };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDevSrcId, &getSrcMsg);
   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      me->m_msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      DevSrcId* devSrc = nullptr;
      if (info.id != nullptr && info.id == "PinMAME"s)
         devSrc = &me->m_pinmameDevSrc;
      else
         continue;
      *devSrc = getSrcMsg.entries[i];
      if (devSrc->deviceDefs)
      {
         devSrc->deviceDefs = new DeviceDef[devSrc->nDevices];
         memcpy(devSrc->deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
      }
   }
   delete[] getSrcMsg.entries;

   // Map PinMAME devices
   for (unsigned int i = 0; i < me->m_pinmameDevSrc.nDevices; i++)
   {
      if (me->m_pinmameDevSrc.deviceDefs[i].groupId == 0x0100)
      {
         if (me->m_PMGIIndex == -1)
            me->m_PMGIIndex = i;
         me->m_nPMGIs++;
      }
      else if (me->m_pinmameDevSrc.deviceDefs[i].groupId == 0x0200)
      {
         if (me->m_PMLampIndex == -1)
            me->m_PMLampIndex = i;
         me->m_nPMLamps++;
      }
      else if ((me->m_PMGIIndex == -1) && (me->m_PMLampIndex == -1))
      {
         if (me->m_PMSolenoidIndex == -1)
            me->m_PMSolenoidIndex = i;
         me->m_nPMSolenoids++;
      }
   }
}

void PUPManager::OnInputSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   std::lock_guard lock(me->m_queueMutex);
   delete[] me->m_pinmameInputSrc.inputDefs;
   memset(&me->m_pinmameInputSrc, 0, sizeof(me->m_pinmameInputSrc));
   delete[] me->m_b2sInputSrc.inputDefs;
   memset(&me->m_b2sInputSrc, 0, sizeof(me->m_b2sInputSrc));
   GetInputSrcMsg getSrcMsg = { 1024, 0, new InputSrcId[1024] };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getInputSrcId, &getSrcMsg);
   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      me->m_msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      InputSrcId* inputSrc = nullptr;
      if (info.id != nullptr && info.id == "PinMAME"s)
         inputSrc = &me->m_pinmameInputSrc;
      else if (info.id != nullptr && info.id == "B2S"s)
         inputSrc = &me->m_b2sInputSrc;
      else
         continue;
      *inputSrc = getSrcMsg.entries[i];
      if (inputSrc->inputDefs)
      {
         inputSrc->inputDefs = new DeviceDef[inputSrc->nInputs];
         memcpy(inputSrc->inputDefs, getSrcMsg.entries[i].inputDefs, getSrcMsg.entries[i].nInputs * sizeof(DeviceDef));
      }
      break;
   }
   delete[] getSrcMsg.entries;
}

}
