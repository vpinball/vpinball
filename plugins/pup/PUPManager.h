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

#include "PUPFont.h"

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

enum class PlayAction : int
{
   Normal,           // plays the file until it ends
   Loop,             // plays the file in a Loop
   SplashReset,      // meant to be used with a Looping file. If this is triggered while a Looping file is currently playing…then the SplashReset file will play to its end, and then the original Looping file will resume from its beginning (there may be a pause when the Looping file begins again). This can be handy, but you would be better using SetBG in most cases to do something similar.
   SplashReturn,     // meant to be used with a Looping file. If this is triggered while a Looping file is currently playing…then the SplashReturn file will play to its end, and then the original Looping file will resume from where it left off (there may be a pause when the Looping file begins again). This can be handy, but you would be better using SetBG in most cases to do something similar.
   StopPlayer,       // will stop whatever file is currently playing. Priority MUST be HIGHER than the file currently playing for this to work!
   StopFile,         // will stop ONLY the file specified in PlayFile (if it's playing). This has no effect on other files that are playing.
   SetBG,            // Set Background will set a new default looping "Background" file. When other files are done playing, then this new SetBG file will be played in a loop. Example: This can be handy for setting a new looping "mode" video, so that new other video events during the new mode will fall back to this SetBG video. Then you can change SetBG again to the main game mode video when the mode is completed.
   PlaySSF,          // used to play WAV files for Surround Sound Feedback. (You don't want these sounds playing from your front / backbox speakers). The settings for the 3D position of the sound files are set in COUNTER. The format is in X,Z,Y. Example: "-2,1,-8". X values range from -10 (left), 0 (center), 10 (right). Z values don't ever change and stay at 1. Y values range from 0 (top), -5 (center), -10 (bottom). NOTE: This currently will only work with the DEFAULT sound card in Windows. Additional sound card / devices are not yet supported!
   SkipSamePriority, // this will ignore the trigger if the file playing has the same Priority. This is nice for events such as Jackpot videos or others that will play very often, and you don't want to have them constantly interrupting each other. "Normal" PlayAction files with the same Priority will interrupt each other no matter the Rest Seconds. Using SkipSamePri will not play the new file (with the same Priority) if the current file is still playing and allows for smoother non-interruptive action for common events.
   CustomFunction    // Call a custom function
};

const string& PlayActionToString(PlayAction value);

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
   bool IsRunning() const { return m_dofEventStream != nullptr; }
   const std::filesystem::path& GetPath() const { return m_szPath; }
   bool AddScreen(std::shared_ptr<PUPScreen> pScreen);
   bool AddScreen(int screenNum);
   std::shared_ptr<PUPScreen> GetScreen(int screenNum, bool logMissing = false) const;
   void SendScreenToBack(const PUPScreen* screen);
   void SendScreenToFront(const PUPScreen* screen);
   bool AddFont(std::unique_ptr<PUPFont> pFont, const string& szFilename);
   PUPFont* GetFont(const string& szFont);

   void QueueDOFEvent(char c, int id, int value);

   void DuckAllExcept(int masterScreenNum, float duckLevel);
   void Unduck();

private:
   void UnloadFonts();
   void LoadFonts();
   void LoadPlaylists();

   void Start();
   void Stop();

   float m_mainVolume = 1.f;
   std::filesystem::path m_szRootPath;
   std::filesystem::path m_szPath;
   string m_szRomName;
   vector<std::shared_ptr<PUPScreen>> m_screenOrder;
   ankerl::unordered_dense::map<int, std::shared_ptr<PUPScreen>> m_screenMap;
   vector<std::unique_ptr<PUPFont>> m_fonts;
   ankerl::unordered_dense::map<string, PUPFont*> m_fontMap;
   ankerl::unordered_dense::map<string, PUPFont*> m_fontFilenameMap;
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

   int m_duckMasterScreen = -1;
   ankerl::unordered_dense::map<int, float> m_preDuckVolumes;
};

}
