#include "core/stdafx.h"
#include "core/vpversion.h"
#include "core/TableDB.h"

#include "VPinballLib.h"
#include "VPXProgress.h"
#include "standalone/inc/webserver/WebServer.h"

#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "miniz/miniz.h"

#include <filesystem>

namespace VPinballLib {

VPinball::VPinball()
{
   EditableRegistry::RegisterEditable<Ball>();
   EditableRegistry::RegisterEditable<Bumper>();
   EditableRegistry::RegisterEditable<Decal>();
   EditableRegistry::RegisterEditable<DispReel>();
   EditableRegistry::RegisterEditable<Flasher>();
   EditableRegistry::RegisterEditable<Flipper>();
   EditableRegistry::RegisterEditable<Gate>();
   EditableRegistry::RegisterEditable<Kicker>();
   EditableRegistry::RegisterEditable<Light>();
   EditableRegistry::RegisterEditable<LightSeq>();
   EditableRegistry::RegisterEditable<Plunger>();
   EditableRegistry::RegisterEditable<Primitive>();
   EditableRegistry::RegisterEditable<Ramp>();
   EditableRegistry::RegisterEditable<Rubber>();
   EditableRegistry::RegisterEditable<Spinner>();
   EditableRegistry::RegisterEditable<Surface>();
   EditableRegistry::RegisterEditable<Textbox>();
   EditableRegistry::RegisterEditable<Timer>();
   EditableRegistry::RegisterEditable<Trigger>();
   EditableRegistry::RegisterEditable<HitTarget>();
}

void VPinball::Init(std::function<void*(Event, void*)> callback)
{
   SDL_SetMainReady();

#if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS))
   SDL_SetiOSEventPump(true);
#endif

   SDL_InitSubSystem(SDL_INIT_VIDEO);
   SDL_InitSubSystem(SDL_INIT_JOYSTICK);
   TTF_Init();

   g_pvp = new ::VPinball();
   g_pvp->SetLogicalNumberOfProcessors(SDL_GetNumLogicalCPUCores());
   g_pvp->m_settings.LoadFromFile(g_pvp->m_szMyPrefPath + "VPinballX.ini", true);

   Logger::GetInstance()->Init();
   Logger::GetInstance()->SetupLogger(true);

   PLOGI << "VPX - " << VP_VERSION_STRING_FULL_LITERAL;
   PLOGI << "m_logicalNumberOfProcessors=" << g_pvp->GetLogicalNumberOfProcessors();
   PLOGI << "m_szMyPath=" << g_pvp->m_szMyPath;
   PLOGI << "m_szMyPrefPath=" << g_pvp->m_szMyPrefPath;

   if (!DirExists(PATH_USER)) {
      PLOGI << "Creating user path: path=" << PATH_USER;
      std::filesystem::create_directory(PATH_USER);
   }

   m_pWebServer = new WebServer();
   m_eventCallback = callback;
}

string VPinball::GetVersionStringFull()
{
   return VP_VERSION_STRING_FULL_LITERAL;
}

void VPinball::Log(LogLevel level, const string& message)
{
   switch (level) {
      case LogLevel::Debug: PLOGD << message; break;
      case LogLevel::Info: PLOGI << message; break;
      case LogLevel::Warn: PLOGW << message; break;
      case LogLevel::Error: PLOGE << message; break;
    }
}

void* VPinball::SendEvent(Event event, void* data)
{
   if (event == Event::LiveUIUpdate) {
      {
         std::lock_guard<std::mutex> lock(s_instance.m_liveUIMutex);
         while (!s_instance.m_liveUIQueue.empty()) {
            auto task = s_instance.m_liveUIQueue.front();
            task();
            s_instance.m_liveUIQueue.pop();
         }
      }
      return nullptr;
   }
   else if (event == Event::PlayerStarted) {
#ifdef __APPLE__
      SDL_SetiOSAnimationCallback(g_pplayer->m_playfieldWnd->GetCore(), 1, &VPinball::GameLoop, nullptr);
#endif
   }
   else if (event == Event::Stopped) {
      s_instance.Cleanup();
   }

   return s_instance.m_eventCallback ? s_instance.m_eventCallback(event, data) : nullptr;
}

void VPinball::ResetLog()
{
   Logger::GetInstance()->Truncate();

   PLOGI << "VPX - " << VP_VERSION_STRING_FULL_LITERAL;
   PLOGI << "m_logicalNumberOfProcessors=" << g_pvp->GetLogicalNumberOfProcessors();
   PLOGI << "m_szMyPath=" << g_pvp->m_szMyPath;
   PLOGI << "m_szMyPrefPath=" << g_pvp->m_szMyPrefPath;
}

int VPinball::LoadValueInt(SettingsSection section, const string& key, int defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(static_cast<Settings::Section>(section), key, defaultValue);
}

float VPinball::LoadValueFloat(SettingsSection section, const string& key, float defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(static_cast<Settings::Section>(section), key, defaultValue);
}

string VPinball::LoadValueString(SettingsSection section, const string& key, const string& defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(static_cast<Settings::Section>(section), key, defaultValue);
}

void VPinball::SaveValueInt(SettingsSection section, const string& key, int value)
{
   g_pvp->m_settings.SaveValue(static_cast<Settings::Section>(section), key, value);
   g_pvp->m_settings.Save();
}

void VPinball::SaveValueFloat(SettingsSection section, const string& key, float value)
{
   g_pvp->m_settings.SaveValue(static_cast<Settings::Section>(section), key, value);
   g_pvp->m_settings.Save();
}

void VPinball::SaveValueString(SettingsSection section, const string& key, const string& value)
{
   g_pvp->m_settings.SaveValue(static_cast<Settings::Section>(section), key, value);
   g_pvp->m_settings.Save();
}

VPinballStatus VPinball::Uncompress(const string& source)
{
   PLOGI.printf("Uncompress: pSource=%s", source.c_str());

   mz_zip_archive zip_archive;
   memset(&zip_archive, 0, sizeof(zip_archive));

   mz_bool status = mz_zip_reader_init_file(&zip_archive, source.c_str(), 0);
   if (!status)
      return VPinballStatus::Failure;

   bool success = true;
   int file_count = static_cast<int>(mz_zip_reader_get_num_files(&zip_archive));

   for (int i = 0; i < file_count && success; i++) {
      mz_zip_archive_file_stat file_stat;
      if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
         success = false;
         continue;
      }

      string filename = file_stat.m_filename;
      if (filename.starts_with("__MACOSX") || filename.starts_with(".DS_Store"))
         continue;

      string path = std::filesystem::path(source).parent_path().append(filename);
      if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
         std::filesystem::create_directories(path);
      else if (!mz_zip_reader_extract_to_file(&zip_archive, i, path.c_str(), 0)) {
         success = false;
         continue;
      }

      ProgressData progressData = { (i * 100) / file_count };
      SendEvent(Event::ArchiveUncompressing, &progressData);
   }

   mz_zip_reader_end(&zip_archive);
   return success ? VPinballStatus::Success : VPinballStatus::Failure;
}

VPinballStatus VPinball::Compress(const string& source, const string& destination)
{
   PLOGI.printf("Compressing: pSource=%s, pDestination=%s", source.c_str(), destination.c_str());

   mz_zip_archive zip_archive;
   memset(&zip_archive, 0, sizeof(zip_archive));

   mz_bool status = mz_zip_writer_init_file(&zip_archive, destination.c_str(), 0);
   if (!status)
      return VPinballStatus::Failure;

   bool success = true;
   std::filesystem::path sourcePath(source);
   auto sourcePathLength = sourcePath.string().length();

   size_t totalFiles = 0;
   for (auto& p : std::filesystem::recursive_directory_iterator(source)) {
      if (!std::filesystem::is_directory(p.path()))
         totalFiles++;
   }

   size_t processedFiles = 0;

   auto add_to_zip = [this, &zip_archive, &sourcePathLength, &processedFiles, totalFiles](const std::filesystem::path& path) {
      if (std::filesystem::is_directory(path)) {
         string dir_in_zip = path.string().substr(sourcePathLength + 1) + "/";
         if (!mz_zip_writer_add_mem(&zip_archive, dir_in_zip.c_str(), nullptr, 0, MZ_NO_COMPRESSION))
            return false;
      }
      else {
         std::ifstream input(path, std::ios::binary);
         vector<char> buffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
         string file_in_zip = path.string().substr(sourcePathLength + 1);
         if (!mz_zip_writer_add_mem(&zip_archive, file_in_zip.c_str(), buffer.data(), buffer.size(), MZ_NO_COMPRESSION))
            return false;
         processedFiles++;

         ProgressData progressData = { (int)((processedFiles * 100) / totalFiles) };
         SendEvent(Event::ArchiveCompressing, &progressData);
      }
      return true;
   };

   for (auto& p : std::filesystem::recursive_directory_iterator(source)) {
      if (!add_to_zip(p.path())) {
         success = false;
         break;
      }
   }

   mz_zip_writer_finalize_archive(&zip_archive);
   mz_zip_writer_end(&zip_archive);

   return success ? VPinballStatus::Success : VPinballStatus::Failure;
}

void VPinball::UpdateWebServer()
{
   if (m_pWebServer->IsRunning())
      m_pWebServer->Stop();

   if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServer", false))
      m_pWebServer->Start();
}

VPinballStatus VPinball::ResetIni()
{
   string iniFilePath = g_pvp->m_szMyPrefPath + "VPinballX.ini";
   if (std::remove(iniFilePath.c_str()) != 0)
      return VPinballStatus::Failure;

   g_pvp->m_settings.LoadFromFile(iniFilePath, true);
   return VPinballStatus::Success;
}

VPinballStatus VPinball::Load(const string& source)
{
   VPXProgress progress;
   g_pvp->LoadFileName(source, true, &progress);
   if (g_pvp->GetActiveTable())
      return VPinballStatus::Success;

   Cleanup();
   return VPinballStatus::Failure;
}

VPinballStatus VPinball::ExtractScript(const string& source)
{
   VPXProgress progress;
   bool success = false;
   g_pvp->LoadFileName(source, false, &progress);

   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();
   if (pActiveTable) {
      string scriptFilename = source;
      if (ReplaceExtensionFromFilename(scriptFilename, "vbs"))
         pActiveTable->m_pcv->SaveToFile(scriptFilename);
      success = true;
   }

   Cleanup();
   return success ? VPinballStatus::Success : VPinballStatus::Failure;
}

VPinballStatus VPinball::Play()
{
   m_gameLoop = nullptr;
   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();

   if (!pActiveTable)
      return VPinballStatus::Failure;

   pActiveTable->Play(0);

   return VPinballStatus::Success;
}

VPinballStatus VPinball::Stop()
{
   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();

   if (!pActiveTable)
      return VPinballStatus::Failure;

   pActiveTable->QuitPlayer(Player::CS_CLOSE_APP);

   return VPinballStatus::Success;
}

void VPinball::SetPlayState(int enable)
{
   if (!g_pplayer)
      return;

   g_pplayer->SetPlayState(enable);
   g_pplayer->m_renderer->DisableStaticPrePass(!enable);
}

void VPinball::ToggleFPS()
{
   if (!g_pplayer)
      return;

   g_pplayer->m_liveUI->ToggleFPS();
}

void VPinball::GetTableOptions(TableOptions& tableOptions)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   tableOptions.globalEmissionScale = g_pplayer->m_renderer->m_globalEmissionScale;
   tableOptions.exposure = g_pplayer->m_renderer->m_exposure;
   tableOptions.toneMapper = (int)g_pplayer->m_renderer->m_toneMapper;
   tableOptions.globalDifficulty = pLiveTable->m_globalDifficulty;
   tableOptions.musicVolume = g_pplayer->m_MusicVolume;
   tableOptions.soundVolume = g_pplayer->m_SoundVolume;
}

void VPinball::SetTableOptions(const TableOptions& tableOptions)
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this, tableOptions]() {
      ProcessSetTableOptions(tableOptions);
   });
}

void VPinball::SetDefaultTableOptions()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this]() {
      ProcessSetDefaultTableOptions();
   });
}

void VPinball::ResetTableOptions()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this]() {
      ProcessResetTableOptions();
   });
}

void VPinball::SaveTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* pTable = g_pplayer->m_pEditorTable;
   PinTable* pLiveTable = g_pplayer->m_ptable;
   pTable->m_settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, true);
   pTable->m_settings.SaveValue(Settings::Player, "DynamicDayNight"s, false);
   pTable->m_settings.SaveValue(Settings::Player, "EmissionScale"s, g_pplayer->m_renderer->m_globalEmissionScale);
   pTable->m_settings.SaveValue(Settings::TableOverride, "Exposure"s, g_pplayer->m_renderer->m_exposure);
   pTable->m_settings.SaveValue(Settings::TableOverride, "ToneMapper"s, g_pplayer->m_renderer->m_toneMapper);
   pTable->m_settings.SaveValue(Settings::TableOverride, "Difficulty"s, pLiveTable->m_globalDifficulty);
   pTable->m_settings.SaveValue(Settings::Player, "MusicVolume"s, g_pplayer->m_MusicVolume);
   pTable->m_settings.SaveValue(Settings::Player, "SoundVolume"s, g_pplayer->m_SoundVolume);
}

int VPinball::GetCustomTableOptionsCount()
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return 0;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   size_t count = pLiveTable->m_settings.GetTableSettings().size();
   return (int)count;
}

void VPinball::GetCustomTableOption(int index, CustomTableOption& customTableOption)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   const Settings::OptionDef& optionDef = pLiveTable->m_settings.GetTableSettings()[index];
   customTableOption.section = (SettingsSection)optionDef.section;
   customTableOption.id = optionDef.id.c_str();
   customTableOption.name = optionDef.name.c_str();
   customTableOption.showMask = optionDef.showMask;
   customTableOption.minValue = optionDef.minValue;
   customTableOption.maxValue = optionDef.maxValue;
   customTableOption.step = optionDef.step;
   customTableOption.defaultValue = optionDef.defaultValue;
   customTableOption.unit = (OptionUnit)optionDef.unit;
   customTableOption.literals = optionDef.tokenizedLiterals.c_str();
   customTableOption.value = pLiveTable->m_settings.LoadValueWithDefault(optionDef.section, optionDef.id, optionDef.defaultValue);
}

void VPinball::SetCustomTableOption(const CustomTableOption& customTableOption)
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);

   m_liveUIQueue.push([this, customTableOption]() {
      ProcessSetCustomTableOption(customTableOption);
   });
}

void VPinball::SetDefaultCustomTableOptions()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);

   m_liveUIQueue.push([this]() {
      ProcessSetDefaultCustomTableOptions();
   });
}

void VPinball::ResetCustomTableOptions()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);

   m_liveUIQueue.push([this]() {
      ProcessResetCustomTableOptions();
   });
}

void VPinball::SaveCustomTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* pTable = g_pplayer->m_pEditorTable;
   PinTable* pLiveTable = g_pplayer->m_ptable;
   size_t count = pLiveTable->m_settings.GetTableSettings().size();
   for (size_t index = 0; index < count; ++index) {
      const Settings::OptionDef& optionDef = pLiveTable->m_settings.GetTableSettings()[index];
      pTable->m_settings.SaveValue(optionDef.section, optionDef.id, pLiveTable->m_settings.LoadValueWithDefault(optionDef.section, optionDef.name, optionDef.defaultValue));
   }
}

void VPinball::GetViewSetup(ViewSetup& viewSetup)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   ::ViewSetup& viewSetupDef = pLiveTable->mViewSetups[pLiveTable->m_BG_current_set];
   viewSetup.viewMode = viewSetupDef.mMode;
   viewSetup.sceneScaleX = viewSetupDef.mSceneScaleX;
   viewSetup.sceneScaleY = viewSetupDef.mSceneScaleY;
   viewSetup.sceneScaleZ = viewSetupDef.mSceneScaleZ;
   if (viewSetupDef.mMode == VLM_WINDOW) {
      viewSetup.viewX = pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f);
      viewSetup.viewY = pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f);
      viewSetup.viewZ = pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f);
   }
   else {
      viewSetup.viewX = viewSetupDef.mViewX;
      viewSetup.viewY = viewSetupDef.mViewY;
      viewSetup.viewZ = viewSetupDef.mViewZ;
   }
   viewSetup.lookAt = viewSetupDef.mLookAt;
   viewSetup.viewportRotation = viewSetupDef.mViewportRotation;
   viewSetup.fov = viewSetupDef.mFOV;
   viewSetup.layback = viewSetupDef.mLayback;
   viewSetup.viewHOfs = viewSetupDef.mViewHOfs;
   viewSetup.viewVOfs = viewSetupDef.mViewVOfs;
   viewSetup.windowTopZOfs = viewSetupDef.mWindowTopZOfs;
   viewSetup.windowBottomZOfs = viewSetupDef.mWindowBottomZOfs;
}

void VPinball::SetViewSetup(const ViewSetup& viewSetup)
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this, viewSetup]() {  
      ProcessSetViewSetup(viewSetup);
   });
}

void VPinball::SetDefaultViewSetup()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this]() {
      ProcessSetDefaultViewSetup();
   });
}

void VPinball::ResetViewSetup()
{
   std::lock_guard<std::mutex> lock(m_liveUIMutex);
   
   m_liveUIQueue.push([this]() {
      ProcessResetViewSetup();
   });
}

void VPinball::SaveViewSetup()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;
   ViewSetupID id = pLiveTable->m_BG_current_set;

   pLiveTable->mViewSetups[id].SaveToTableOverrideSettings(pTable->m_settings, (ViewSetupID)id);
   if (pTable->m_BG_current_set == BG_FULLSCREEN) {
      // Player position is saved as an override (not saved if equal to app settings)
      pTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerX", pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f), true);
      pTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerY", pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f), true);
      pTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ", pLiveTable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f), true);
   }
}

void VPinball::GameLoop(void* pUserData)
{
   if (!s_instance.m_gameLoop) {
       return;
   }

   if (g_pplayer && s_instance.m_gameLoop)
      s_instance.m_gameLoop();

   if (g_pplayer && (g_pplayer->GetCloseState() == Player::CS_PLAYING || g_pplayer->GetCloseState() == Player::CS_USER_INPUT))
      return;

   PLOGI.printf("Game Loop stopping");

   s_instance.m_gameLoop = nullptr;

   delete g_pplayer;
   g_pplayer = nullptr;

   SendEvent(Event::Stopped, nullptr);
}

void VPinball::ProcessSetTableOptions(const TableOptions& options)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   g_pplayer->m_renderer->m_globalEmissionScale = options.globalEmissionScale;
   g_pplayer->m_renderer->m_exposure = options.exposure;
   g_pplayer->m_renderer->m_toneMapper = (ToneMapper)options.toneMapper;
   pLiveTable->m_globalDifficulty = options.globalDifficulty;
   g_pplayer->m_MusicVolume = options.musicVolume;
   g_pplayer->m_SoundVolume = options.soundVolume;

   g_pplayer->m_renderer->MarkShaderDirty();
   pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessSetDefaultTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;

   // TODO undo Day/Night, difficulty, ...

   g_pplayer->m_renderer->MarkShaderDirty();
   pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessResetTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;
   pTable->m_settings.DeleteValue(Settings::Player, "OverrideTableEmissionScale"s);
   pTable->m_settings.DeleteValue(Settings::Player, "DynamicDayNight"s);
   g_pplayer->m_renderer->m_globalEmissionScale = pTable->m_globalEmissionScale;
   g_pplayer->m_renderer->m_exposure = pTable->m_settings.LoadValueWithDefault(Settings::TableOverride, "Exposure"s, pTable->GetExposure());
   g_pplayer->m_renderer->m_toneMapper = (ToneMapper)pTable->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, pTable->GetToneMapper());
   pLiveTable->m_globalDifficulty = g_pvp->m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, pTable->m_difficulty);
   g_pplayer->m_MusicVolume = pTable->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
   g_pplayer->m_SoundVolume = pTable->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);

   g_pplayer->m_renderer->MarkShaderDirty();
   pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessSetCustomTableOption(const CustomTableOption& customTableOption)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   pLiveTable->m_settings.SaveValue((Settings::Section)customTableOption.section, customTableOption.id, customTableOption.value);
   if (customTableOption.section == SettingsSection::TableOption)
      pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessSetDefaultCustomTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;

   // TODO

   g_pplayer->m_renderer->MarkShaderDirty();
   pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessResetCustomTableOptions()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* pTable = g_pplayer->m_pEditorTable;
   PinTable* pLiveTable = g_pplayer->m_ptable;
   size_t count = pLiveTable->m_settings.GetTableSettings().size();
   for (size_t index = 0; index < count; ++index) {
      const Settings::OptionDef& optionDef = pLiveTable->m_settings.GetTableSettings()[index];
      pLiveTable->m_settings.DeleteValue(optionDef.section, optionDef.id);
   }

   g_pplayer->m_renderer->MarkShaderDirty();
   pLiveTable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1);
}

void VPinball::ProcessSetViewSetup(const ViewSetup& viewSetup)
{
   if (!(g_pplayer && g_pplayer->m_ptable))
      return;

   PinTable* pLiveTable = g_pplayer->m_ptable;
   ::ViewSetup& viewSetupDef = pLiveTable->mViewSetups[pLiveTable->m_BG_current_set];
   bool isWindow = viewSetup.viewMode == (int)VLM_WINDOW;
   viewSetupDef.mMode = (ViewLayoutMode)viewSetup.viewMode;
   viewSetupDef.mSceneScaleX = viewSetup.sceneScaleX;
   viewSetupDef.mSceneScaleY = viewSetup.sceneScaleY;
   viewSetupDef.mSceneScaleZ = viewSetup.sceneScaleZ;
   if (isWindow) {
      pLiveTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerX"s, viewSetup.viewX);
      pLiveTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerY"s, viewSetup.viewY);
      pLiveTable->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, viewSetup.viewZ);
   }
   else {
      viewSetupDef.mViewX = viewSetup.viewX;
      viewSetupDef.mViewY = viewSetup.viewY;
      viewSetupDef.mViewZ = viewSetup.viewZ;
   }
   viewSetupDef.mLookAt = viewSetup.lookAt;
   viewSetupDef.mViewportRotation = viewSetup.viewportRotation;
   viewSetupDef.mFOV = viewSetup.fov;
   viewSetupDef.mLayback = viewSetup.layback;
   viewSetupDef.mViewHOfs = viewSetup.viewHOfs;
   viewSetupDef.mViewVOfs = viewSetup.viewVOfs;
   viewSetupDef.mWindowTopZOfs = viewSetup.windowTopZOfs;
   viewSetupDef.mWindowBottomZOfs = viewSetup.windowBottomZOfs;

   if (isWindow)
      viewSetupDef.SetWindowModeFromSettings(pLiveTable);

   g_pplayer->m_renderer->InitLayout();
}

void VPinball::ProcessSetDefaultViewSetup()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;
   ViewSetupID id = pLiveTable->m_BG_current_set;
   ::ViewSetup& viewSetup = pLiveTable->mViewSetups[id];
   viewSetup.mViewportRotation = 0.f;
   const bool portrait = g_pplayer->m_playfieldWnd->GetWidth() < g_pplayer->m_playfieldWnd->GetHeight();
   switch (id)
   {
      case BG_DESKTOP:
      case BG_FSS:
         if (id == BG_DESKTOP && !portrait)
         { // Desktop
            viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopMode"s, VLM_CAMERA);
            viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamX"s, 0.f));
            viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamY"s, 20.f));
            viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamZ"s, 70.f));
            viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleX"s, 1.f);
            viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleY"s, 1.f);
            viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleZ"s, 1.f);
            viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopFov"s, 50.f);
            viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopLookAt"s, 25.0f);
            viewSetup.mViewHOfs = 0.0f;
            viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopViewVOfs"s, 14.f);
         }
         else
         { // FSS
            viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSMode"s, VLM_CAMERA);
            viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamX"s, 0.f));
            viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamY"s, 20.f));
            viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamZ"s, 70.f));
            viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleX"s, 1.f);
            viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleY"s, 1.f);
            viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleZ"s, 1.f);
            viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSFov"s, 77.f);
            viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSLookAt"s, 50.0f);
            viewSetup.mViewHOfs = 0.0f;
            viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSViewVOfs"s, 22.f);
         }
         break;
      case BG_FULLSCREEN:
         {
            const float screenWidth = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
            const float screenHeight = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
            if (screenWidth <= 1.f || screenHeight <= 1.f)
            {
               //PushNotification("You must setup your screen size before using Window mode"s, 5000);
            }
            else
            {
               float topHeight = pLiveTable->m_glassTopHeight;
               float bottomHeight = pLiveTable->m_glassBottomHeight;
               if (bottomHeight == topHeight)
               { // If table does not define the glass position (for table without it, when loading we set the glass as horizontal)
                  TableDB db;
                  db.Load();
                  int bestSizeMatch = db.GetBestSizeMatch(pLiveTable->GetTableWidth(), pLiveTable->GetHeight(), topHeight);
                  if (bestSizeMatch >= 0)
                  {
                     bottomHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassBottom);
                     topHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassTop);
                     char textBuf1[MAXNAMEBUFFER], textBuf2[MAXNAMEBUFFER];
                     sprintf_s(textBuf1, sizeof(textBuf1), "%.02f", db.m_data[bestSizeMatch].glassBottom);
                     sprintf_s(textBuf2, sizeof(textBuf2), "%.02f", db.m_data[bestSizeMatch].glassTop);
                     //PushNotification("Missing glass position guessed to be "s + textBuf1 + "\" / " + textBuf2 + "\" (" + db.m_data[bestSizeMatch].name + ')', 5000);
                  }
                  else
                  {
                     //PushNotification("The table is missing glass position and no good guess was found."s, 5000);
                  }
               }
               const float scale = (screenHeight / pLiveTable->GetTableWidth()) * (pLiveTable->GetHeight() / screenWidth);
               const bool isFitted = (viewSetup.mViewHOfs == 0.f) && (viewSetup.mViewVOfs == -2.8f) && (viewSetup.mSceneScaleY == scale) && (viewSetup.mSceneScaleX == scale);
               viewSetup.mMode = VLM_WINDOW;
               viewSetup.mViewHOfs = 0.f;
               viewSetup.mViewVOfs = isFitted ? 0.f : -2.8f;
               viewSetup.mSceneScaleX = scale;
               viewSetup.mSceneScaleY = isFitted ? 1.f : scale;
               viewSetup.mWindowBottomZOfs = bottomHeight;
               viewSetup.mWindowTopZOfs = topHeight;
               //PushNotification(isFitted ? "POV reset to default values (stretch to fit)"s : "POV reset to default values (no stretching)"s, 5000);
            }
            break;
         }
      default:
         break;
   }

   g_pplayer->m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
   g_pplayer->m_renderer->InitLayout();
}

void VPinball::ProcessResetViewSetup()
{
   if (!(g_pplayer && g_pplayer->m_pEditorTable && g_pplayer->m_ptable))
      return;

   PinTable* const pTable = g_pplayer->m_pEditorTable;
   PinTable* const pLiveTable = g_pplayer->m_ptable;
   ViewSetupID id = pLiveTable->m_BG_current_set;
   pLiveTable->mViewSetups[id] = pTable->mViewSetups[id];
   pLiveTable->mViewSetups[id].ApplyTableOverrideSettings(pLiveTable->m_settings, (ViewSetupID)id);

   g_pplayer->m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
   g_pplayer->m_renderer->InitLayout();
}

void VPinball::Cleanup()
{
   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();
   if (pActiveTable)
      g_pvp->CloseTable(pActiveTable);

   delete g_pvp;
   g_pvp = new ::VPinball();
   g_pvp->m_settings.LoadFromFile(g_pvp->m_szMyPrefPath + "VPinballX.ini", true);
   g_pvp->SetLogicalNumberOfProcessors(SDL_GetNumLogicalCPUCores());
   
   {
      std::lock_guard<std::mutex> lock(m_liveUIMutex);
      while (!m_liveUIQueue.empty())
         m_liveUIQueue.pop();
   }
}

VPinball VPinball::s_instance;

} // namespace VPinballLib
