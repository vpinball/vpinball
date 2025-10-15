#include "core/stdafx.h"

#include "core/TableDB.h"
#include "core/VPXPluginAPIImpl.h"
#include "core/extern.h"
#include "VPinballLib.h"
#include "VPXProgress.h"
#include "WebServer.h"

#include <zip.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <set>
#include <map>
#include <queue>
#include <mutex>
#include <nlohmann/json.hpp>

#if defined(__ANDROID__)
#define COMMAND_APP_INIT_COMPLETED 0x8001
#endif

MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginUnload();
MSGPI_EXPORT void MSGPIAPI B2SPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI B2SPluginUnload();
MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginUnload();
MSGPI_EXPORT void MSGPIAPI DOFPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI DOFPluginUnload();
MSGPI_EXPORT void MSGPIAPI DMDUtilPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI DMDUtilPluginUnload();
MSGPI_EXPORT void MSGPIAPI FlexDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI FlexDMDPluginUnload();
MSGPI_EXPORT void MSGPIAPI PinMAMEPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI PinMAMEPluginUnload();
MSGPI_EXPORT void MSGPIAPI PUPPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI PUPPluginUnload();
MSGPI_EXPORT void MSGPIAPI RemoteControlPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI RemoteControlPluginUnload();
MSGPI_EXPORT void MSGPIAPI ScoreViewPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI ScoreViewPluginUnload();
MSGPI_EXPORT void MSGPIAPI SerumPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI SerumPluginUnload();
MSGPI_EXPORT void MSGPIAPI WMPPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api);
MSGPI_EXPORT void MSGPIAPI WMPPluginUnload();

#ifdef __APPLE__
#include "VPinballLib_iOS.h"
#endif

namespace VPinballLib {

VPinballLib::VPinballLib()
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
   EditableRegistry::RegisterEditable<PartGroup>();
}

VPinballLib::~VPinballLib()
{
}

int VPinballLib::AppInit(int argc, char** argv)
{
   if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
      return 0;

   SDL_PropertiesID props = SDL_CreateProperties();
   SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Visual Pinball Player");
   #if defined(__ANDROID__)
      SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, true);
   #endif
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
   m_pWindow = SDL_CreateWindowWithProperties(props);
   SDL_DestroyProperties(props);

   if (!m_pWindow)
      return 0;

#ifdef __APPLE__
   if (!InitIOS(m_pWindow))
      return 0;
#endif

#ifdef __ANDROID__
   SDL_SendAndroidMessage(COMMAND_APP_INIT_COMPLETED, 0);
#endif

   return 1;
}

void VPinballLib::AppIterate()
{
   if (m_gameLoop) {
      m_gameLoop();

      if (g_pplayer && (g_pplayer->GetCloseState() == Player::CS_PLAYING
         || g_pplayer->GetCloseState() == Player::CS_USER_INPUT))
         return;

      CComObject<PinTable>* pActiveTable = g_pvp->GetActiveTable();

      if (g_pplayer->GetCloseState() == Player::CS_CLOSE_CAPTURE_SCREENSHOT) {
         if (m_captureInProgress)
            return;

         std::filesystem::path tablePath(pActiveTable->m_filename);
         string imageFilename = tablePath.stem().string() + ".jpg";
         string imagePath = tablePath.parent_path().string() + PATH_SEPARATOR_CHAR + imageFilename;

         if (std::filesystem::exists(imagePath)) {
            g_pplayer->SetCloseState(Player::CS_CLOSE_APP);
            return;
         }

         m_captureInProgress = true;

         g_pplayer->m_renderer->m_renderDevice->CaptureScreenshot(imagePath,
            [this, imagePath](bool success) {
               m_captureInProgress = false;

               if (success) {
                  PLOGI.printf("Screenshot saved: %s", imagePath.c_str());
               } 
               else {
                  PLOGE.printf("Failed to save screenshot: %s", imagePath.c_str());
               }

               g_pplayer->SetCloseState(Player::CS_CLOSE_APP);
            });

         return;
      }

      PLOGI.printf("Game Loop stopping");

      m_gameLoop = nullptr;

      delete g_pplayer;
      g_pplayer = nullptr;

      g_pvp->CloseTable(pActiveTable);
   }
}

void VPinballLib::AppEvent(SDL_Event* event)
{
   std::lock_guard<std::mutex> lock(m_eventMutex);
   if (m_gameLoop)
      m_eventQueue.push(*event);
   else {
      while (!m_eventQueue.empty())
         m_eventQueue.pop();
   }
}

bool VPinballLib::PollAppEvent(SDL_Event& event)
{
   std::lock_guard<std::mutex> lock(m_eventMutex);
   if (m_eventQueue.empty())
      return false;

   event = m_eventQueue.front();
   m_eventQueue.pop();
   return true;
}

void VPinballLib::Init(VPinballEventCallback callback)
{
   SetEventCallback(callback);

   SDL_RunOnMainThread([](void*) {
      g_pvp = new ::VPinball();
      g_pvp->SetLogicalNumberOfProcessors(SDL_GetNumLogicalCPUCores());
      g_pvp->m_settings.LoadFromFile(g_pvp->m_myPrefPath + "VPinballX.ini", true);
      g_pvp->m_settings.Save();

      Logger::GetInstance()->Init();
      Logger::GetInstance()->SetupLogger(true);

      PLOGI << "VPX - " << VP_VERSION_STRING_FULL_LITERAL;
      PLOGI << "m_logicalNumberOfProcessors=" << g_pvp->GetLogicalNumberOfProcessors();
      PLOGI << "m_myPath=" << g_pvp->m_myPath;
      PLOGI << "m_myPrefPath=" << g_pvp->m_myPrefPath;

      if (!DirExists(PATH_USER)) {
         std::error_code ec;
         if (std::filesystem::create_directory(PATH_USER, ec)) {
            PLOGI.printf("User path created: %s", PATH_USER.c_str());
         }
         else {
            PLOGE.printf("Unable to create user path: %s", PATH_USER.c_str());
         }
      }

      VPinballLib& lib = VPinballLib::Instance();
      lib.LoadPlugins();
      lib.UpdateWebServer();
   }, nullptr, true);
}

void VPinballLib::SetEventCallback(VPinballEventCallback callback)
{
   m_eventCallback = [callback](VPINBALL_EVENT event, void* data) -> void* {
      thread_local string jsonString;
      const char* jsonData = nullptr;

      if (data != nullptr) {
         nlohmann::json j;

         switch(event) {
            case VPINBALL_EVENT_LOADING_ITEMS:
            case VPINBALL_EVENT_LOADING_SOUNDS:
            case VPINBALL_EVENT_LOADING_IMAGES:
            case VPINBALL_EVENT_LOADING_FONTS:
            case VPINBALL_EVENT_LOADING_COLLECTIONS:
            case VPINBALL_EVENT_PRERENDERING: {
               ProgressData* progressData = (ProgressData*)data;
               j["progress"] = progressData->progress;
               jsonString = j.dump();
               jsonData = jsonString.c_str();
               break;
            }
            case VPINBALL_EVENT_RUMBLE: {
               RumbleData* rumbleData = (RumbleData*)data;
               j["lowFrequencyRumble"] = rumbleData->lowFrequencyRumble;
               j["highFrequencyRumble"] = rumbleData->highFrequencyRumble;
               j["durationMs"] = rumbleData->durationMs;
               jsonString = j.dump();
               jsonData = jsonString.c_str();
               break;
            }
            case VPINBALL_EVENT_SCRIPT_ERROR: {
               ScriptErrorData* scriptErrorData = (ScriptErrorData*)data;
               j["error"] = (int)scriptErrorData->error;
               j["line"] = scriptErrorData->line;
               j["position"] = scriptErrorData->position;
               j["description"] = scriptErrorData->description;
               jsonString = j.dump();
               jsonData = jsonString.c_str();
               break;
            }
            case VPINBALL_EVENT_WEB_SERVER: {
               WebServerData* webServerData = (WebServerData*)data;
               j["url"] = webServerData->url;
               jsonString = j.dump();
               jsonData = jsonString.c_str();
               break;
            }
            case VPINBALL_EVENT_COMMAND: {
               CommandData* commandData = (CommandData*)data;
               j["command"] = commandData->command;
               j["data"] = commandData->data;
               jsonString = j.dump();
               jsonData = jsonString.c_str();
               break;
            }
            default:
               break;
         }
      }

      callback(event, jsonData);
      return nullptr;
   };
}

void VPinballLib::SendEvent(VPINBALL_EVENT event, void* data)
{
   auto callback = Instance().m_eventCallback;
   if (callback)
      callback(event, data);

   if (event == VPINBALL_EVENT_PLAYER_STARTED || event == VPINBALL_EVENT_PLAYER_CLOSED)
      WebServer::BroadcastStatus();
}

void VPinballLib::LoadPlugins()
{
   static constexpr struct {
      const char* id;
      void (*load)(uint32_t, const MsgPluginAPI*);
      void (*unload)();
   } plugins[] = {
      { "PinMAME",       &PinMAMEPluginLoad,       &PinMAMEPluginUnload       },
      { "AlphaDMD",      &AlphaDMDPluginLoad,      &AlphaDMDPluginUnload      },
      { "B2S",           &B2SPluginLoad,           &B2SPluginUnload           },
      { "B2SLegacy",     &B2SLegacyPluginLoad,     &B2SLegacyPluginUnload     },
      { "DOF",           &DOFPluginLoad,           &DOFPluginUnload           },
      { "DMDUtil",       &DMDUtilPluginLoad,       &DMDUtilPluginUnload       },
      { "FlexDMD",       &FlexDMDPluginLoad,       &FlexDMDPluginUnload       },
      { "PUP",           &PUPPluginLoad,           &PUPPluginUnload           },
      { "RemoteControl", &RemoteControlPluginLoad, &RemoteControlPluginUnload },
      { "ScoreView",     &ScoreViewPluginLoad,     &ScoreViewPluginUnload     },
      { "Serum",         &SerumPluginLoad,         &SerumPluginUnload         },
      { "WMP",           &WMPPluginLoad,           &WMPPluginUnload           }
   };

   for (size_t i = 0; i < std::size(plugins); ++i) {
      if (VPXPluginAPIImpl::GetInstance().getAPI().GetOption(
             plugins[i].id, "Enable",
             VPX_OPT_SHOW_UI, "Enable plugin",
             0.f, 1.f, 1.f, 0.f,
             VPXPluginAPI::NONE,
             nullptr
         )) {
         auto& p = plugins[i];
         auto plugin = MsgPI::MsgPluginManager::GetInstance().RegisterPlugin(
            p.id, p.id, p.id,
            "", "", "",
            p.load, p.unload
         );
         plugin->Load(&MsgPI::MsgPluginManager::GetInstance().GetMsgAPI());
         m_plugins.push_back(plugin);
      }
   }
}

void VPinballLib::UnloadPlugins()
{
   for (auto it = m_plugins.rbegin(); it != m_plugins.rend(); ++it)
      (*it)->Unload();
   m_plugins.clear();
}

void VPinballLib::Log(VPINBALL_LOG_LEVEL level, const string& message)
{
   switch (level) {
      case VPINBALL_LOG_LEVEL_DEBUG:
         PLOGD << message;
         break;
      case VPINBALL_LOG_LEVEL_INFO:
         PLOGI << message;
         break;
      case VPINBALL_LOG_LEVEL_WARN:
         PLOGW << message;
         break;
      case VPINBALL_LOG_LEVEL_ERROR:
         PLOGE << message;
         break;
   }
}

void VPinballLib::ResetLog()
{
   Logger::GetInstance()->Truncate();
}

int VPinballLib::LoadValueInt(const string& sectionName, const string& key, int defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(Settings::GetSection(sectionName), key, defaultValue);
}

float VPinballLib::LoadValueFloat(const string& sectionName, const string& key, float defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(Settings::GetSection(sectionName), key, defaultValue);
}

string VPinballLib::LoadValueString(const string& sectionName, const string& key, const string& defaultValue)
{
   return g_pvp->m_settings.LoadValueWithDefault(Settings::GetSection(sectionName), key, defaultValue);
}

void VPinballLib::SaveValueInt(const string& sectionName, const string& key, int value)
{
   g_pvp->m_settings.SaveValue(Settings::GetSection(sectionName), key, value);
   g_pvp->m_settings.Save();
}

void VPinballLib::SaveValueFloat(const string& sectionName, const string& key, float value)
{
   g_pvp->m_settings.SaveValue(Settings::GetSection(sectionName), key, value);
   g_pvp->m_settings.Save();
}

void VPinballLib::SaveValueString(const string& sectionName, const string& key, const string& value)
{
   g_pvp->m_settings.SaveValue(Settings::GetSection(sectionName), key, value);
   g_pvp->m_settings.Save();
}

VPINBALL_STATUS VPinballLib::ResetIni()
{
   string iniFilePath = g_pvp->m_myPrefPath + "VPinballX.ini";
   if (!std::filesystem::remove(iniFilePath))
    return VPINBALL_STATUS_FAILURE;

   g_pvp->m_settings.LoadFromFile(iniFilePath, true);
   g_pvp->m_settings.Save();
   return VPINBALL_STATUS_SUCCESS;
}

void VPinballLib::UpdateWebServer()
{
   m_webServer.Update();
}

VPINBALL_STATUS VPinballLib::LoadTable(const string& tablePath)
{
   VPXProgress progress;
   g_pvp->LoadFileName(tablePath, true, &progress);

   bool success = g_pvp->GetActiveTable() != nullptr;
   return success ? VPINBALL_STATUS_SUCCESS : VPINBALL_STATUS_FAILURE;
}

VPINBALL_STATUS VPinballLib::ExtractTableScript()
{
   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();
   if (!pActiveTable)
      return VPINBALL_STATUS_FAILURE;

   string tempPath = string(g_pvp->m_myPrefPath) + "temp_script.vbs";
   pActiveTable->m_pcv->SaveToFile(tempPath);

   std::filesystem::path tablePath(pActiveTable->m_filename);
   string vbsFilename = tablePath.stem().string() + ".vbs";

   string destPath = tablePath.parent_path().string() + PATH_SEPARATOR_CHAR + vbsFilename;

   try {
      std::filesystem::copy_file(tempPath, destPath, std::filesystem::copy_options::overwrite_existing);
      std::filesystem::remove(tempPath);
   }
   
   catch (const std::exception& e) {
      PLOGE.printf("Failed to save script file: %s", e.what());
      std::filesystem::remove(tempPath);
      g_pvp->CloseTable(pActiveTable);
      return VPINBALL_STATUS_FAILURE;
   }

   g_pvp->CloseTable(pActiveTable);

   return VPINBALL_STATUS_SUCCESS;
}

VPINBALL_STATUS VPinballLib::Play()
{
   if (m_gameLoop)
      return VPINBALL_STATUS_FAILURE;

   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();
   if (!pActiveTable)
      return VPINBALL_STATUS_FAILURE;

   if (!SDL_RunOnMainThread([](void*) {
      g_pvp->DoPlay(0);
   }, nullptr, true))
      return VPINBALL_STATUS_FAILURE;

   return VPINBALL_STATUS_SUCCESS;
}

VPINBALL_STATUS VPinballLib::Stop()
{
   CComObject<PinTable>* const pActiveTable = g_pvp->GetActiveTable();
   if (!pActiveTable)
      return VPINBALL_STATUS_FAILURE;

   pActiveTable->QuitPlayer(Player::CS_CLOSE_APP);

   return VPINBALL_STATUS_SUCCESS;
}

}
