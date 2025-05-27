#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPCustomPos.h"

#include <filesystem>
#include <fstream>

namespace PUP {

PUPManager::PUPManager(MsgPluginAPI* msgApi, uint32_t endpointId, const string& rootPath)
   : m_szRootPath(rootPath)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
{
   constexpr unsigned int mapping4[] = { 0, 1, 4, 15 };
   for (int i = 0; i < 4; i++)
   {
      m_palette4[i * 3 + 0] = (mapping4[i] * 0xFF) / 0xF; // R
      m_palette4[i * 3 + 1] = (mapping4[i] * 0x45) / 0xF; // G
      m_palette4[i * 3 + 2] = (mapping4[i] * 0x00) / 0xF; // B
   }
   for (int i = 0; i < 16; i++)
   {
      m_palette16[i * 3 + 0] = (i * 0xFF) / 0xF; // R
      m_palette16[i * 3 + 1] = (i * 0x45) / 0xF; // G
      m_palette16[i * 3 + 2] = (i * 0x00) / 0xF; // B
   }

   m_msgApi->SubscribeMsg(m_endpointId, m_getAuxRendererId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);
}

PUPManager::~PUPManager()
{
   Stop();
   Unload();
   m_msgApi->UnsubscribeMsg(m_getAuxRendererId, OnGetRenderer);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_getAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
}

void PUPManager::LoadConfig(const string& szRomName)
{
   if (m_init)
   {
      LOGE("PUP already loaded");
      return;
   }

   if (m_szRootPath.empty())
   {
      LOGI("No pupvideos folder found, not initializing PUP");
      return;
   }

   m_szPath = find_case_insensitive_directory_path(m_szRootPath + szRomName);
   if (m_szPath.empty())
      return;

   LOGI("PUP path: %s", m_szPath.c_str());

   // Load playlists

   LoadPlaylists();

   // Load screens

   string szScreensPath = find_case_insensitive_file_path(m_szPath + "screens.pup");
   if (!szScreensPath.empty()) {
      std::ifstream screensFile;
      screensFile.open(szScreensPath, std::ifstream::in);
      if (screensFile.is_open()) {
         string line;
         int i = 0;
         while (std::getline(screensFile, line)) {
            if (++i == 1)
               continue;
            PUPScreen* pScreen = PUPScreen::CreateFromCSV(this, line, m_playlists);
            if (pScreen)
               AddScreen(pScreen);
         }
      }
      else {
         LOGE("Unable to load %s", szScreensPath.c_str());
      }
   }
   else {
      LOGI("No screens.pup file found");
   }

   // Determine child screens, creating missing top screen if needed
   
   for (int i = 0; i < 6; i++)
      if (!m_screenMap.contains(i)
         && (std::ranges::find_if(m_screenMap, [i](auto& entry) { return entry.second->GetCustomPos() && entry.second->GetCustomPos()->GetSourceScreen() == i; }) != m_screenMap.end()))
         switch (i)
         {
         case 0: AddScreen(PUPScreen::CreateFromCSV(this, "0,\"Topper\",\"\",,0,ForcePopBack,0,"s, m_playlists)); break;
         case 1: AddScreen(PUPScreen::CreateFromCSV(this, "0,\"DMD\",\"\",,0,ForcePopBack,0,"s, m_playlists)); break;
         case 2: AddScreen(PUPScreen::CreateFromCSV(this, "0,\"Backglass\",\"\",,0,ForcePopBack,0,"s, m_playlists)); break;
         case 3: break; // Playfield
         case 4: break; // Music
         case 5: AddScreen(PUPScreen::CreateFromCSV(this, "0,\"FullDMD\",\"\",,0,ForcePopBack,0,"s, m_playlists)); break;
         }
   for (auto& [key, pScreen] : m_screenMap) {
      PUPCustomPos* pCustomPos = pScreen->GetCustomPos();
      if (pCustomPos) {
         ankerl::unordered_dense::map<int, PUPScreen*>::const_iterator it = m_screenMap.find(pCustomPos->GetSourceScreen());
         if (it == m_screenMap.end())
            continue;
         PUPScreen* const pParentScreen = it->second;
         if (pParentScreen && pScreen != pParentScreen)
            pParentScreen->AddChild(pScreen);
      }
   }

   // Load Fonts

   string szFontsPath = find_case_insensitive_directory_path(m_szPath + "FONTS");
   if (!szFontsPath.empty()) {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath)) {
         if (entry.is_regular_file()) {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf")
            {
               if (TTF_Font* pFont = TTF_OpenFont(szFontPath.c_str(), 8))
               {
                  AddFont(pFont, entry.path().filename().string());
               }else{
                  LOGE("Failed to load font: %s %s", szFontPath.c_str(), SDL_GetError());
               }
            }
         }
      }
   }
   else {
      LOGI("No FONTS folder found");
   }

   // Setup DMD triggers
   m_dmd = std::make_unique<PUPDMD::DMD>();
   m_dmd->Load(m_szRootPath.c_str(), szRomName.c_str());
   m_dmd->SetLogCallback([](const char* format, va_list args, const void* userData) {
      char buffer[1024];
      vsnprintf(buffer, sizeof(buffer), format, args);
      LOGD(buffer);
      }, this);
   m_init = true;

   QueueTriggerData({ 'D', 0, 1 });
}

void PUPManager::Unload()
{
   if (!m_init)
      return;

   for (auto& [key, pScreen] : m_screenMap)
      delete pScreen;
   m_screenMap.clear();

   for (auto& pFont : m_fonts)
      TTF_CloseFont(pFont);
   m_fonts.clear();
   m_fontMap.clear();
   m_fontFilenameMap.clear();

   for (auto& playlist : m_playlists)
      delete playlist;
   m_playlists.clear();

   m_dmd = nullptr;

   m_szPath.clear();
   m_init = false;
}

void PUPManager::LoadPlaylists()
{
   string szPlaylistsPath = find_case_insensitive_file_path(GetPath() + "playlists.pup");
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

bool PUPManager::AddScreen(PUPScreen* pScreen)
{
   if (HasScreen(pScreen->GetScreenNum())) {
      LOGE("Duplicate screen: screen={%s}", pScreen->ToString(false).c_str());
      delete pScreen;
      return false;
   }

   m_screenMap[pScreen->GetScreenNum()] = pScreen;

   LOGI("Screen added: screen={%s}", pScreen->ToString().c_str());

   return true;
}

bool PUPManager::AddScreen(int screenNum)
{
   PUPScreen* pScreen = PUPScreen::CreateDefault(this, screenNum, m_playlists);
   if (!pScreen)
      return false;

   return AddScreen(pScreen);
}

bool PUPManager::HasScreen(int screenNum)
{
   ankerl::unordered_dense::map<int, PUPScreen*>::const_iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end();
}

PUPScreen* PUPManager::GetScreen(int screenNum) const
{
   if (!m_init) {
      LOGE("Getting screen before initialization");
   }

   ankerl::unordered_dense::map<int, PUPScreen*>::const_iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
}

bool PUPManager::AddFont(TTF_Font* pFont, const string& szFilename)
{
   if (!pFont)
      return false;

   m_fonts.push_back(pFont);

   const string szFamilyName = string(TTF_GetFontFamilyName(pFont));

   const string szNormalizedFamilyName = lowerCase(string_replace_all(szFamilyName, "  "s, " "s));
   m_fontMap[szNormalizedFamilyName] = pFont;

   string szStyleName = string(TTF_GetFontStyleName(pFont));
   if (szStyleName != "Regular")
   {
      const string szFullName = szFamilyName + ' ' + szStyleName;
      const string szNormalizedFullName = lowerCase(string_replace_all(szFullName, "  "s, " "s));
      m_fontMap[szNormalizedFullName] = pFont;
   }

   const string szNormalizedFilename = lowerCase(szFilename.substr(0, szFilename.length() - 4));
   m_fontFilenameMap[szNormalizedFilename] = pFont;

   LOGI("Font added: familyName=%s, styleName=%s, filename=%s", szFamilyName.c_str(), szStyleName.c_str(), szFilename.c_str());

   return true;
}

TTF_Font* PUPManager::GetFont(const string& szFont)
{
   string szNormalizedFamilyName = lowerCase(string_replace_all(szFont, "  "s, " "s));

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
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_triggerDataQueue.push_back({ data.type, data.number, data.value });
   }
   m_queueCondVar.notify_one();
}

void PUPManager::ProcessQueue()
{
   SetThreadName("PUPManager.ProcessQueue");
   while (m_isRunning)
   {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_queueCondVar.wait_for(lock, std::chrono::nanoseconds(16666), [this] { return !m_triggerDataQueue.empty() || !m_triggerDmdQueue.empty() || !m_isRunning; });

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
         uint8_t* frame = m_triggerDmdQueue.front();
         m_triggerDmdQueue.pop();

         uint8_t* palette;
         if (m_dmdId.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE2)
            palette = m_palette4;
         else if (m_dmdId.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE4)
            palette = m_palette16;
         else
            return;

         // Reproduce legacy behavior for backward compatibility (scaling, padding, coloring)
         // This is very hacky and should be replaced by identification against the raw identify frame.
         if (m_dmdId.width == 128 && m_dmdId.height == 32)
         {
            for (unsigned int i = 0; i < 128 * 32; i++)
               memcpy(&m_rgbFrame[i * 3], &palette[frame[i] * 3], 3);
         }
         else if (m_dmdId.width == 128 && m_dmdId.height < 32)
         {
            const unsigned int ofsY = ((32 - m_dmdId.height) / 2) * 128;
            for (unsigned int i = 0; i < 128 * 16; i++)
               memcpy(&m_rgbFrame[(ofsY + i) * 3], &palette[frame[i] * 3], 3);
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
                  const int l = (int)roundf(lum / sum);
                  memcpy(&m_rgbFrame[(y * 128 + ofsX + x) * 3], &palette[l * 3], 3);
               }
            }
         }
         else
         {
            // Unsupported DMD format (would need to implement a dedicated stretch fit, matching what is used elsewhere)
         }
         delete[] frame;

         dmdTrigger = m_dmd->Match(m_rgbFrame, 128, 32, false);
         if (dmdTrigger == 0) // 0 is unmatched for libpupdmd, but D0 is init trigger for PUP
            dmdTrigger = -1;
      }

      for (auto& [key, screen] : m_screenMap)
      {
         for (auto& [cmd, triggers] : screen->GetTriggers())
         {
            bool wasTriggered = triggers[0]->IsTriggered();
            for (auto& trigger : triggers[0]->GetTriggers())
            {
               switch (trigger.m_type)
               {
               case 'W': // PinMAME switch state
                  if (trigger.m_number < static_cast<int>(m_pinmameInputSrc.nInputs))
                     trigger.m_value = m_pinmameInputSrc.GetInputState(trigger.m_number - 1) ? 1 : 0;
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
                     trigger.m_value = m_pinmameDevSrc.GetFloatState(trigger.m_number - 1) > 0.5f ? 1 : 0;
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
               for (auto& triggerData : m_triggerDataQueue)
                  if ((trigger.m_type == triggerData.type) && (trigger.m_number == triggerData.number))
                     trigger.m_value = triggerData.value;
            }
            bool isTriggered = triggers[0]->IsTriggered();
            if (isTriggered && !wasTriggered)
            {
               for (auto trigger : triggers)
               {
                  PUPTriggerRequest* pRequest = new PUPTriggerRequest();
                  pRequest->pTrigger = trigger;
                  screen->QueueTriggerRequest(pRequest);
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
   if (!m_init || m_isRunning)
      return;

   LOGI("PUP start");

   m_isRunning = true;
   m_thread = std::thread(&PUPManager::ProcessQueue, this);

   for (auto& [key, pScreen] : m_screenMap)
      pScreen->Start();

   // Subscribe to message bus events
   m_getDmdSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_onSerumTriggerId = m_msgApi->GetMsgID("Serum", "OnDmdTrigger");

   m_getDevSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   m_onDevSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);

   m_getInputSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
   m_onInputSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG);

   memset(m_rgbFrame, 0, sizeof(m_rgbFrame));
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDevSrcChangedId, OnDevSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onInputSrcChangedId, OnInputSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSerumTriggerId, OnSerumTrigger, this);
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
   OnDevSrcChanged(m_onDevSrcChangedId, this, nullptr);
   OnInputSrcChanged(m_onInputSrcChangedId, this, nullptr);
   OnPollDmd(this);
}

void PUPManager::Stop()
{
   if (!m_isRunning)
      return;

   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
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
}

int PUPManager::Render(VPXRenderContext2D* const renderCtx, void* context)
{
   PUPManager* me = static_cast<PUPManager*>(context);
   if (!me->m_init)
      return false;
   PUPScreen* screen = nullptr;
   switch (renderCtx->window)
   {
   case VPXAnciliaryWindow::VPXWINDOW_Topper: screen = me->GetScreen(0); break;
   case VPXAnciliaryWindow::VPXWINDOW_Backglass: screen = me->GetScreen(2); break;
   case VPXAnciliaryWindow::VPXWINDOW_ScoreView: screen = me->GetScreen(5); break; // TODO select 1 or 5 (user settings ?)
   }
   if (screen == nullptr || screen->GetCustomPos() != nullptr)
      return false;

   renderCtx->srcWidth = renderCtx->outWidth;
   renderCtx->srcHeight = renderCtx->outHeight;
   screen->SetActive(true);
   screen->SetSize(static_cast<int>(renderCtx->outWidth), static_cast<int>(renderCtx->outHeight));
   screen->Render(renderCtx);
   return true;
}

void PUPManager::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   PUPManager* me = static_cast<PUPManager*>(context);
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   static AnciliaryRendererDef entry = { "PUP", "PinUp Player", "Renderer for PinUp player backglass", nullptr, Render };
   if (msg->window == VPXAnciliaryWindow::VPXWINDOW_Backglass || msg->window == VPXAnciliaryWindow::VPXWINDOW_ScoreView || msg->window == VPXAnciliaryWindow::VPXWINDOW_Topper)
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
// FIXME replace by event listening
void PUPManager::OnPollDmd(void* userData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   if (me->m_dmdId.id.id != 0 && me->m_dmdId.GetIdentifyFrame)
   {
      DisplayFrame idFrame = me->m_dmdId.GetIdentifyFrame(me->m_dmdId.id);
      if (idFrame.frameId != me->m_lastFrameId && idFrame.frame)
      {
         me->m_lastFrameId = idFrame.frameId;
         uint8_t* frame = new uint8_t[me->m_dmdId.width * me->m_dmdId.height];
         memcpy(frame, idFrame.frame, me->m_dmdId.width * me->m_dmdId.height);
         {
            std::lock_guard<std::mutex> lock(me->m_queueMutex);
            me->m_triggerDmdQueue.push(frame);
         }
         me->m_queueCondVar.notify_one();
      }
   }
   me->m_msgApi->RunOnMainThread(1.0 / 60.0, OnPollDmd, me);
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
   std::unique_lock<std::mutex> lock(me->m_queueMutex);
   delete[] me->m_pinmameDevSrc.deviceDefs;
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
      if (info.id != nullptr && strcmp(info.id, "PinMAME") == 0)
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
         me->m_nPMSolenoids++;
   }
}

void PUPManager::OnInputSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   PUPManager* me = static_cast<PUPManager*>(userData);
   std::unique_lock<std::mutex> lock(me->m_queueMutex);
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
      if (info.id != nullptr && strcmp(info.id, "PinMAME") == 0)
         inputSrc = &me->m_pinmameInputSrc;
      else if (info.id != nullptr && strcmp(info.id, "B2S") == 0)
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