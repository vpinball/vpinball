#pragma once

#include "common.h"

#include "CorePlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // In PupDMD: std::map<uint16_t,PUPDMD::Hash,std::less<uint16_t>,std::allocator<std::pair<const uint16_t,PUPDMD::Hash>>> » needs dll-interface
#include "pupdmd.h"
#pragma warning(pop)

#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <SDL3_ttf/SDL_ttf.h>

#define PUP_SCREEN_TOPPER             0
#define PUP_SETTINGS_TOPPERX          320
#define PUP_SETTINGS_TOPPERY          30
#define PUP_SETTINGS_TOPPERWIDTH      290
#define PUP_SETTINGS_TOPPERHEIGHT     75
#define PUP_ZORDER_TOPPER             300

#define PUP_SCREEN_BACKGLASS          2
#define PUP_SETTINGS_BACKGLASSX       PUP_SETTINGS_TOPPERX
#define PUP_SETTINGS_BACKGLASSY       PUP_SETTINGS_TOPPERY + PUP_SETTINGS_TOPPERHEIGHT + 5
#define PUP_SETTINGS_BACKGLASSWIDTH   290
#define PUP_SETTINGS_BACKGLASSHEIGHT  218
#define PUP_ZORDER_BACKGLASS          150

#define PUP_SCREEN_DMD                1
#define PUP_SETTINGS_DMDX             PUP_SETTINGS_TOPPERX
#define PUP_SETTINGS_DMDY             PUP_SETTINGS_BACKGLASSY + PUP_SETTINGS_BACKGLASSHEIGHT + 5
#define PUP_SETTINGS_DMDWIDTH         290
#define PUP_SETTINGS_DMDHEIGHT        75
#define PUP_ZORDER_DMD                200

#define PUP_SCREEN_PLAYFIELD          3
#define PUP_SETTINGS_PLAYFIELDX       PUP_SETTINGS_TOPPERX + PUP_SETTINGS_TOPPERWIDTH + 5
#define PUP_SETTINGS_PLAYFIELDY       PUP_SETTINGS_TOPPERY
#define PUP_SETTINGS_PLAYFIELDWIDTH   216
#define PUP_SETTINGS_PLAYFIELDHEIGHT  384
#define PUP_ZORDER_PLAYFIELD          150

#define PUP_SCREEN_FULLDMD            5
#define PUP_SETTINGS_FULLDMDX         PUP_SETTINGS_TOPPERX
#define PUP_SETTINGS_FULLDMDY         PUP_SETTINGS_DMDY + 5
#define PUP_SETTINGS_FULLDMDWIDTH     290
#define PUP_SETTINGS_FULLDMDHEIGHT    150
#define PUP_ZORDER_FULLDMD            200

typedef struct {
   char type;
   int number;
   int value;
} PUPTriggerData;

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;

class PUPManager
{
public:
   PUPManager(MsgPluginAPI* msgApi, uint32_t endpointId, const string& rootPath);
   ~PUPManager();

   const string& GetRootPath() { return m_szRootPath; }

   bool IsInit() { return m_init; }
   void LoadConfig(const string& szRomName);
   void Unload();
   const string& GetPath() { return m_szPath; }
   bool AddScreen(PUPScreen* pScreen);
   bool AddScreen(int screenNum);
   bool HasScreen(int screenNum);
   PUPScreen* GetScreen(int screenNum);
   bool AddFont(TTF_Font* pFont, const string& szFilename);
   TTF_Font* GetFont(const string& szFamily);
   void QueueTriggerData(PUPTriggerData data);
   void Start();
   void Stop();

private:

   void ProcessQueue();
   void LoadPlaylists();

   bool m_init = false;
   const string m_szRootPath;
   string m_szPath;
   std::map<int, PUPScreen*> m_screenMap;
   vector<TTF_Font*> m_fonts;
   std::map<string, TTF_Font*> m_fontMap;
   std::map<string, TTF_Font*> m_fontFilenameMap;
   vector<PUPTriggerData> m_triggerDataQueue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning = false;
   std::thread m_thread;
   vector<PUPPlaylist*> m_playlists;

   MsgPluginAPI* const m_msgApi;
   const uint32_t m_endpointId;
   unsigned int m_getAuxRendererId = 0, m_onAuxRendererChgId = 0;
   unsigned int m_onDmdSrcChangedId = 0, m_getDmdSrcId = 0, m_getIdentifyDmdId = 0;
   unsigned int m_onDevSrcChangedId = 0, m_getDevSrcId = 0;
   unsigned int m_onInputSrcChangedId = 0, m_getInputSrcId = 0;
   unsigned int m_onSerumTriggerId = 0;
   DevSrcId m_pinmameDevSrc { 0 };
   unsigned int m_nPMSolenoids = 0;
   int m_PMGIIndex = -1;
   unsigned int m_nPMGIs = 0;
   int m_PMLampIndex = -1;
   unsigned int m_nPMLamps = 0;
   InputSrcId m_pinmameInputSrc { 0 };
   InputSrcId m_b2sInputSrc { 0 };

   unsigned int m_lastFrameId = 0;
   CtlResId m_dmdId { 0 };
   std::chrono::high_resolution_clock::time_point m_lastFrameTick;
   std::unique_ptr<PUPDMD::DMD> m_dmd;
   std::queue<GetRawDmdMsg> m_triggerDmdQueue;
   uint8_t m_rgbFrame[128 * 32 * 3] { 0 };
   uint8_t m_palette4[4 * 3] { 0 };
   uint8_t m_palette16[16 * 3] { 0 };
   void QueueDMDFrame(const GetRawDmdMsg& getDmdMsg);

   static int Render(VPXRenderContext2D* const renderCtx, void* context);
   static void OnGetRenderer(const unsigned int eventId, void* userData, void* eventData);
   static void OnGetIdentifyDMD(const unsigned int eventId, void* userData, void* eventData);
   static void OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData);
   static void OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData);
   static void OnDevSrcChanged(const unsigned int eventId, void* userData, void* eventData);
   static void OnInputSrcChanged(const unsigned int eventId, void* userData, void* eventData);
   static void OnPollDmd(void* userData);
};
