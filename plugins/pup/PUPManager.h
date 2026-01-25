// license:GPLv3+

#pragma once

#include "DOFStreamEvent.h"

#include "common.h"

#include "plugins/ControllerPlugin.h"
#include "plugins/VPXPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // In PupDMD: std::map<uint16_t,PUPDMD::Hash,std::less<uint16_t>,std::allocator<std::pair<const uint16_t,PUPDMD::Hash>>> needs dll-interface
#include "pupdmd.h"
#pragma warning(pop)

#include <unordered_dense.h>
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
#define PUP_SETTINGS_BACKGLASSY       (PUP_SETTINGS_TOPPERY + PUP_SETTINGS_TOPPERHEIGHT + 5)
#define PUP_SETTINGS_BACKGLASSWIDTH   290
#define PUP_SETTINGS_BACKGLASSHEIGHT  218
#define PUP_ZORDER_BACKGLASS          150

#define PUP_SCREEN_DMD                1
#define PUP_SETTINGS_DMDX             PUP_SETTINGS_TOPPERX
#define PUP_SETTINGS_DMDY             (PUP_SETTINGS_BACKGLASSY + PUP_SETTINGS_BACKGLASSHEIGHT + 5)
#define PUP_SETTINGS_DMDWIDTH         290
#define PUP_SETTINGS_DMDHEIGHT        75
#define PUP_ZORDER_DMD                200

#define PUP_SCREEN_PLAYFIELD          3
#define PUP_SETTINGS_PLAYFIELDX       (PUP_SETTINGS_TOPPERX + PUP_SETTINGS_TOPPERWIDTH + 5)
#define PUP_SETTINGS_PLAYFIELDY       PUP_SETTINGS_TOPPERY
#define PUP_SETTINGS_PLAYFIELDWIDTH   216
#define PUP_SETTINGS_PLAYFIELDHEIGHT  384
#define PUP_ZORDER_PLAYFIELD          150

#define PUP_SCREEN_FULLDMD            5
#define PUP_SETTINGS_FULLDMDX         PUP_SETTINGS_TOPPERX
#define PUP_SETTINGS_FULLDMDY         (PUP_SETTINGS_DMDY + 5)
#define PUP_SETTINGS_FULLDMDWIDTH     290
#define PUP_SETTINGS_FULLDMDHEIGHT    150
#define PUP_ZORDER_FULLDMD            200

namespace PUP {

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;

class PUPManager final
{
public:
   PUPManager(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::filesystem::path& rootPath);
   ~PUPManager();

   const MsgPluginAPI* GetMsgAPI() const { return m_msgApi; }
   const std::filesystem::path& GetRootPath() const { return m_szRootPath; }

   void SetGameDir(const string& szRomName);
   void LoadConfig(const string& szRomName);
   void Unload();
   const std::filesystem::path& GetPath() const { return m_szPath; }
   bool AddScreen(std::shared_ptr<PUPScreen> pScreen);
   bool AddScreen(int screenNum);
   std::shared_ptr<PUPScreen> GetScreen(int screenNum, bool logMissing = false) const;
   void SendScreenToBack(const PUPScreen* screen);
   void SendScreenToFront(const PUPScreen* screen);
   bool AddFont(TTF_Font* pFont, const string& szFilename);
   TTF_Font* GetFont(const string& szFont);

   void QueueDOFEvent(char c, int id, int value);

private:
   void UnloadFonts();
   void LoadFonts();
   void LoadPlaylists();

   void Start();
   bool IsRunning() const { return m_dofEventStream != nullptr; }
   void Stop();

   float m_mainVolume = 1.f;
   std::filesystem::path m_szRootPath;
   std::filesystem::path m_szPath;
   vector<std::shared_ptr<PUPScreen>> m_screenOrder;
   ankerl::unordered_dense::map<int, std::shared_ptr<PUPScreen>> m_screenMap;
   vector<TTF_Font*> m_fonts;
   ankerl::unordered_dense::map<string, TTF_Font*> m_fontMap;
   ankerl::unordered_dense::map<string, TTF_Font*> m_fontFilenameMap;
   vector<PUPPlaylist*> m_playlists;

   const uint32_t m_endpointId;
   const MsgPluginAPI* const m_msgApi;
   const VPXPluginAPI* m_vpxApi = nullptr;

   std::unique_ptr<PUPDMD::DMD> m_dmd;
   std::array<uint8_t, 128 * 32> m_idFrame;
   int ProcessDmdFrame(const DisplaySrcId& src, const uint8_t* frame);
   
   unsigned int m_getAuxRendererId = 0;
   unsigned int m_onAuxRendererChgId = 0;
   unsigned int m_getVpxApiId = 0;
   static int Render(VPXRenderContext2D* const renderCtx, void* context);
   static void OnGetRenderer(const unsigned int eventId, void* context, void* msgData);

   std::mutex m_eventMutex;
   std::unique_ptr<DOFEventStream> m_dofEventStream;
};

}
