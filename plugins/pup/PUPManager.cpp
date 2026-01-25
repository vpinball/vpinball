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
   , m_endpointId(endpointId)
   , m_msgApi(msgApi)
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

void PUPManager::Start()
{
   LOGI("PUP Manager start");
   assert(!IsRunning());
   m_dofEventStream = std::make_unique<DOFEventStream>(m_msgApi, m_endpointId, [this](char c, int id, int value) { QueueDOFEvent(c, id, value); });
   m_dofEventStream->SetDMDHandler(
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
}

void PUPManager::Stop()
{
   LOGI("PUP Manager stop");
   assert(IsRunning());
   m_dofEventStream = nullptr;
}

void PUPManager::SetGameDir(const string& szRomName)
{
   assert(!IsRunning());

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

   Start();

   // Queue initial game event
   QueueDOFEvent('D', 0, 1);
}

void PUPManager::Unload()
{
   if (IsRunning())
      Stop();

   m_screenOrder.clear();
   m_screenMap.clear();

   m_dmd = nullptr;

   UnloadFonts();

   for (auto& playlist : m_playlists)
      delete playlist;
   m_playlists.clear();

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
   std::unique_lock lock(m_eventMutex);

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

   if (const std::unique_ptr<PUPCustomPos>& pCustomPos = pScreen->GetCustomPos(); pCustomPos)
   {
      const auto it = m_screenMap.find(pCustomPos->GetSourceScreen());
      std::shared_ptr<PUPScreen> parent;
      if (it != m_screenMap.end()) {
         parent = it->second;
      }
      else {
         switch (pCustomPos->GetSourceScreen()) {
         case 0: parent = std::move(PUPScreen::CreateFromCSV(this, R"(0,"Topper","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 1: parent = std::move(PUPScreen::CreateFromCSV(this, R"(1,"DMD 4x1","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 2: parent = std::move(PUPScreen::CreateFromCSV(this, R"(2,"Backglass 16x9","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 3: parent = std::move(PUPScreen::CreateFromCSV(this, R"(3,"Playfield","",,0,Off,0,)"s, m_playlists)); break;
         case 4: parent = std::move(PUPScreen::CreateFromCSV(this, R"(4,"Music","",,0,MusicOnly,0,)"s, m_playlists)); break;
         case 5: parent = std::move(PUPScreen::CreateFromCSV(this, R"(5,"FullDMD 16x9","",,0,ForceBack,0,)"s, m_playlists)); break;
         case 6: parent = std::move(PUPScreen::CreateFromCSV(this, R"(6,"Backglass 4x3-5x4","",,0,ForceBack,0,)"s, m_playlists)); break;
         default: parent = std::move(PUPScreen::CreateFromCSV(this, "("s + std::to_string(pCustomPos->GetSourceScreen()) + R"(,"","",,0,ForceBack,0,)"s, m_playlists)); break;
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

   LOGI("Screen added: screen={%s}", pScreen->ToString().c_str());

   return true;
}

bool PUPManager::AddScreen(int screenNum)
{
   std::unique_ptr<PUPScreen> pScreen = PUPScreen::CreateDefault(this, screenNum, m_playlists);
   return pScreen ? AddScreen(std::move(pScreen)) : false;
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
      m_dmd->Load(m_szPath.string().c_str(), "", src.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE2 ? 2 : 4);
      memset(m_idFrame.data(), 0, m_idFrame.size());
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

   if (src.width <= 256 && src.height == 64)
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
      const unsigned int ofsX = (128 - (src.width / 2)) / 2;
      for (unsigned int y = 0; y < 32; y++)
      {
         for (unsigned int x = 0; x < src.width / 2; x++)
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
                  if (/*px >= 0 &&*/ static_cast<unsigned int>(px) < src.width //
                     && /*py >= 0 &&*/ static_cast<unsigned int>(py) < src.height) // unsigned int tests include the >= 0 ones
                     lum += static_cast<float>(frame[py * src.width + px]) * weight;
                  sum += weight;
               }
            }
            m_idFrame[y * 128 + ofsX + x] = (uint8_t)roundf(lum / sum);
         }
      }
      return static_cast<int>(m_dmd->MatchIndexed(m_idFrame.data(), 128, 32));
   }

   return -1;
}

void PUPManager::QueueDOFEvent(char c, int id, int value)
{
   LOGD("DOF Event %c%03d = %d", c, id, value);

   std::lock_guard lock(m_eventMutex);
   for (const auto& [key, screen] : m_screenMap)
   {
      for (auto& [cmd, triggers] : screen->GetTriggers())
      {
         const bool wasTriggered = triggers[0]->IsTriggered();
         for (auto& trigger : triggers[0]->GetTriggers())
         {
            if (trigger.m_type == c && trigger.m_number == id)
            {
               trigger.m_value = value;
               break;
            }
         }
         const bool isTriggered = triggers[0]->IsTriggered();
         if (isTriggered && !wasTriggered)
         {
            for (auto trigger : triggers)
            {
               // Dispatch trigger action on main thread
               LOGD("Trigger %s", trigger->ToString().c_str());
               m_msgApi->RunOnMainThread(m_endpointId, 0.0, [](void* userData) { static_cast<PUPTrigger*>(userData)->Trigger()(); }, trigger);
            }
         }
      }
   }
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
      rootScreen = me->GetScreen(2); // select 2 or 6 (user settings ?)
      if (rootScreen == nullptr || rootScreen->GetCustomPos() != nullptr)
         rootScreen = me->GetScreen(6);
      padLeft = pupBGPadLeft_Get();
      padRight = pupBGPadRight_Get();
      padTop = pupBGPadTop_Get();
      padBottom = pupBGPadBottom_Get();
      break;
   case VPXWindowId::VPXWINDOW_ScoreView:
      rootScreen = me->GetScreen(5); // select 1 or 5 (user settings ?)
      if (rootScreen == nullptr || rootScreen->GetCustomPos() != nullptr)
         rootScreen = me->GetScreen(1);
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
   for (auto screen : me->m_screenOrder)
   {
      const PUPScreen* parent = screen.get();
      while (parent && parent != rootScreen.get())
         parent = parent->GetParent();
      if (parent)
         screens.push_back(screen);
   }
   #define LOG_RENDER 0
   #if LOG_RENDER
   std::stringstream renderLog;
   renderLog << "Back [";
   auto log = [](std::shared_ptr<PUPScreen> screen, int pass)
   {
      if (pass == 0 && screen->HasUnderlay())
         renderLog << screen->GetScreenNum() << "u ";
      else if (pass == 1 && (!screen->IsPop() || screen->IsMainPlaying()))
         renderLog << screen->GetScreenNum() << " ";
      else if (pass == 2 && screen->HasOverlay())
         renderLog << screen->GetScreenNum() << "o ";
   };
   #else
   auto log = [](std::shared_ptr<PUPScreen> screen, int pass) { };
   #endif
   for (int pass = 0; pass < 3; pass++)
      std::ranges::for_each(screens,
         [&renderCtx, pass, &log](const auto& screen)
         {
            const bool isBack = screen->IsBackgroundPlaying() || screen->GetMode() == PUPScreen::Mode::ForceBack || screen->GetMode() == PUPScreen::Mode::ForcePopBack;
            if (isBack)
            {
               log(screen, pass);
               screen->Render(renderCtx, pass);
            }
         });
   #if LOG_RENDER
   renderLog << "] Front [";
   #endif
   for (int pass = 0; pass < 3; pass++)
      std::ranges::for_each(screens,
         [&renderCtx, pass, &log](const auto& screen)
         {
            const bool isBack = screen->IsBackgroundPlaying() || screen->GetMode() == PUPScreen::Mode::ForceBack || screen->GetMode() == PUPScreen::Mode::ForcePopBack;
            if (!isBack && screen->GetMode() != PUPScreen::Mode::Off)
            {
               log(screen, pass);
               screen->Render(renderCtx, pass);
            }
         });
   #if LOG_RENDER
   renderLog << "]";
   LOGD("Render: %s", renderLog.str().c_str());
   #endif
   std::ranges::for_each(screens,
      [&renderCtx](const auto& screen)
      {
         screen->Render(renderCtx, 3);
      });

   return true;
}

void PUPManager::OnGetRenderer(const unsigned int eventId, void* context, void* msgData)
{
   auto me = static_cast<PUPManager*>(context);
   auto msg = static_cast<GetAncillaryRendererMsg*>(msgData);
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

}
