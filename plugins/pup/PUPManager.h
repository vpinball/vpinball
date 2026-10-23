// license:GPLv3+

#pragma once

#include "common.h"

#include "plugins/B2SPluginEventStream.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/PagedUIPlugin.h"
#include "plugins/ResURIResolver.h"
#include "plugins/VPXPlugin.h"

#include "DMDOverlay/DMDOverlay.h"
#include "PUPSound.h"

#pragma warning(push)
#pragma warning(disable : 4251) // In PupDMD: std::map<uint16_t,PUPDMD::Hash,std::less<uint16_t>,std::allocator<std::pair<const uint16_t,PUPDMD::Hash>>> needs dll-interface
#include "pupdmd.h"
#pragma warning(pop)

#include <unordered_dense.h>
#include <vector>
#include <mutex>
#include <thread>

#include <SDL3_ttf/SDL_ttf.h>

#include "PUPFont.h"

#define PUP_SCREEN_TOPPER             0

#define PUP_SCREEN_BACKGLASS          2

#define PUP_SCREEN_DMD                1

#define PUP_SCREEN_PLAYFIELD          3

#define PUP_SCREEN_FULLDMD            5

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
   CustomFunction,   // Call a custom function
   QueueSameP = 11,  // Queue after the current file when it passes the priority rules
   QueueAlways = 12  // Queue after the current file whatever its priority
};

const string& PlayActionToString(PlayAction value);

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;

// The authored overlay document is the shared DMDOverlay model: a user-authored record of the
// pack's 'displays.pup' file is a DMD rendered on top of the content of one of the ancillary
// windows (target 0 = backglass, 1 = score view, 2 = topper), in window pixels with the origin
// at the top left. The glass image is referenced by a path relative to the pack root, inside
// its 'PUPDisplays' folder (glassPath), with a picked file staged in glassPending until commit.
// The plugin binds the model hooks in PUPManager.cpp.
using PUPDMDDisplay = DMDOverlay::Display;
using PUPOverlayDoc = DMDOverlay::Doc;

class PUPManager final
{
public:
   PUPManager(const MsgPluginAPI* msgApi, uint32_t endpointId, const PagedUIPluginAPI* pagedUIApi, const std::filesystem::path& rootPath);
   ~PUPManager();

   const std::filesystem::path& GetRootPath() const { return m_szRootPath; }

   // Locate the pupvideos folder for a game id (format: ns::rom), searching
   // each base under an optional intermediate namespace folder first --
   // base/ns/rom -- then directly -- base/rom.
   std::filesystem::path FindGameDir(const std::string_view& gameNs, const std::string_view& gameId) const;
   void SetGameDir(const ControllerDef& controller);
   void SetGameDir(const string& szRomName);
   void LoadConfig(const ControllerDef& controller);
   void LoadConfig(const string& szRomName);
   void Unload();
   bool IsRunning() const { return m_B2SPluginEventStream != nullptr; }
   const std::filesystem::path& GetPath() const { return m_szPath; }
   bool AddScreen(std::shared_ptr<PUPScreen> pScreen);
   bool AddScreen(int screenNum);
   std::shared_ptr<PUPScreen> GetScreen(int screenNum, bool logMissing = false) const;
   void SendScreenToBack(const PUPScreen* screen);
   void SendScreenToFront(const PUPScreen* screen);
   bool AddFont(std::unique_ptr<PUPFont> pFont, const string& szFilename);
   PUPFont* GetFont(const string& szFont);

   void QueueDOFEvent(char c, int id, int value);
   void RunLater(double seconds, std::function<void()> fn);
   void PlaySound(const string& name, int x, int y, int z, int vol, const string& special) { m_sounds.Play(name, x, y, z, vol, special); }
   void CustomFunction(int screenNum, const string& szMsg);
   void SendMsg(const string& szMsg);

   void DuckAllExcept(int percent, int masterScreenNum, bool all);
   void Unduck();
   void SetDefaultVolumeAll(int volume);

private:
   void ApplyGameDir(const std::filesystem::path& path, const std::string_view& gameId, const ControllerDef& controller);
   ControllerDef SelectControllerForGame(const std::string_view& gameKey);
   void UnloadFonts();
   void LoadFonts();
   void LoadPlaylists();
   void LoadDisplays();
   void RegisterOverlaySheet();
   int RenderDisplays(VPXRenderContext2D* ctx, int target);

   void Start();
   void Tick();
   void Stop();

   float m_mainVolume = 1.f;
   std::filesystem::path m_szRootPath;
   std::filesystem::path m_szPath;
   string m_szRomName;
   // Controller the event stream is bound to, m_controllerGameId backing its gameId pointer
   ControllerDef m_controller {};
   string m_controllerGameId;
   vector<std::shared_ptr<PUPScreen>> m_screenOrder;
   ankerl::unordered_dense::map<int, std::shared_ptr<PUPScreen>> m_screenMap;
   PUPSoundBank m_sounds { this };
   vector<std::unique_ptr<PUPFont>> m_fonts;
   ankerl::unordered_dense::map<string, PUPFont*> m_fontMap;
   ankerl::unordered_dense::map<string, PUPFont*> m_fontFilenameMap;
   vector<PUPPlaylist*> m_playlists;

   const uint32_t m_endpointId;
   const MsgPluginAPI* const m_msgApi;
   const PagedUIPluginAPI* const m_pagedUIApi;
   const VPXPluginAPI* m_vpxApi = nullptr;

   // Document being authored through the in game UI (see DMDOverlay/DMDOverlay.h InitEditSheetDef)
   PUPOverlayDoc m_overlayDoc;
   bool m_overlaySheetRegistered = false;
   PinballPlugin::ResURIResolver m_resURIResolver;
   vector<std::unique_ptr<DMDOverlay::Renderer>> m_dmdOverlays; // One per display of the edited document, by document index

   std::unique_ptr<PUPDMD::DMD> m_dmd;
   // Triggers in the loaded pack that only a DMD frame match can fire, and what
   // is needed to report once that nothing can fire them.
   unsigned int m_dmdTriggerCount = 0;
   bool m_dmdTriggerDataLoaded = false;
   bool m_reportedMissingIdentification = false;
   std::array<uint8_t, 128 * 32> m_idFrame;
   int ProcessDmdFrame(const DisplaySrcId& src, const uint8_t* frame);
   
   const unsigned int m_getVpxApiId;

   const unsigned int m_getAuxRendererId;
   const unsigned int m_onAuxRendererChgId;
   static int Render(VPXRenderContext2D* const renderCtx, void* context);
   static void OnGetRenderer(const unsigned int eventId, void* context, void* msgData);

   const unsigned int m_getAudioSrcId;
   const unsigned int m_onAudioSrcChangedId;
   const AudioSrcId m_audioSrcDef;
   static void OnGetAudioSrc(const unsigned int msgId, void* userData, void* msgData);

   std::mutex m_eventMutex;
   std::unique_ptr<B2SPluginEventStream> m_B2SPluginEventStream;
   std::shared_ptr<int> m_runToken = std::make_shared<int>(0);
   bool m_ticking = false;
};

}
