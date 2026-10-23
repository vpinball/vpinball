// license:GPLv3+

#pragma once

#include "PUPManager.h"

#include "PUPTrigger.h"

#include "PUPPlaylist.h"

#include "PUPImage.h"

#include "PUPCustomPos.h"

namespace PUP {

class PUPMediaManager;
class PUPLabel;

class PUPScreen final
{
public:
   enum class Mode
   {
      Off,
      Show, // This will use whatever settings are set for this screen's "default" settings.
      ForceOn, // Forces the window to be the Top most window, and it stays on. A video is always going to be playing in this mode. When a new video starts or the current video restarts, the ForceON action happens and will force the window to be the Top window again.
      ForcePop, // Similar to ForceOn, except the vlc window opens and closes with each video played
      ForceBack, // Keeps the video window always open, but under the other PuP screens. This can cause any PuP screen with this setting to get pushed behind your game window.
      ForcePopBack, // Similar to ForceBack, except the vlc window opens and closes with each video played
      MusicOnly
   };

   PUPScreen(PUPManager* manager, PUPScreen::Mode mode, int screenNum, const string& screenDes, bool transparent,
      int volumeMultiplier, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists);
   ~PUPScreen();

   static std::unique_ptr<PUPScreen> CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists);
   static std::unique_ptr<PUPScreen> CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists);

   PUPManager* GetManager() const { return m_pManager; }
   int GetScreenNum() const { return m_screenNum; }
   string ToString(bool full = true) const;

   Mode GetMode() const { return m_mode; }
   void SetMode(Mode mode);

   bool IsPop() const { return m_mode == PUPScreen::Mode::ForcePopBack || m_mode == PUPScreen::Mode::ForcePop; }

   bool IsTopmost() const { return m_topmost; }
   void SetTopmost(bool topmost) { m_topmost = topmost; }

   bool IsHidden() const { return m_hidden; }
   void SetHidden(bool hidden) { m_hidden = hidden; }

   float m_screenAlpha = 1.0f;
   float m_overlayAlpha = 1.0f;
   bool m_hudVisible = true; // FN 3: the whole label overlay shown or hidden
   bool m_hideDuringPlay = false; // FN 34: labels hidden and label commands ignored until the current media ends
   bool m_padTextAlways = false;
   // Action queued by LabelShowPage, applied when the main media ends.
   enum class HudReturn
   {
      None,
      RestoreHud,  // "hidehudplay": re-show the HUD overlay
      RestorePage, // "returnplay":  return to the default page
   };
   HudReturn m_hudReturn = HudReturn::None;
   void OnMediaEnd();

   int GetDefaultVolume() const { return m_defaultVolume; }
   void SetMainVolume(float volume);
   void SetVolumeMultiplier(int multiplier);
   void SetDefaultVolume(int volume);
   void SetVolumeCurrent(int volume);
   void UpdateVolumeFade();
   void Duck(int percent);
   void Unduck();
   bool IsDuckMaster() const { return m_duckMaster; }
   void SetDuckMaster(bool master) { m_duckMaster = master; }
   int GetDefaultDuck() const { return m_defaultDuck; }
   void SetDefaultDuck(int percent) { m_defaultDuck = percent; }

   const std::unique_ptr<PUPCustomPos>& GetCustomPos() const { return m_pCustomPos; }
   void SetCustomPos(const string& szCustomPos);
   void SetBounds(int x, int y, int w, int h);

   void AddChild(std::shared_ptr<PUPScreen> pScreen);
   void ReplaceChild(std::shared_ptr<PUPScreen> pChild, std::shared_ptr<PUPScreen> pScreen);
   PUPScreen* GetParent() const { return m_pParent; }
   bool HasChildren() const { return !m_children.empty(); }

   void AddTrigger(PUPTrigger* pTrigger);
   const ankerl::unordered_dense::map<string, vector<PUPTrigger*>>& GetTriggers() const { return m_triggerMap; }

   void LabelInit();
   void AddLabel(PUPLabel* pLabel);
   PUPLabel* GetLabel(const string& labelName);
   void SendLabelToFront(PUPLabel* pLabel);
   void SendLabelToBack(PUPLabel* pLabel);
   void SendLabelToBackground(PUPLabel* pLabel);
   void RemoveLabel(PUPLabel* pLabel);
   int GetPage() const { return m_pagenum; }
   void SetPage(int pagenum, int durationMs, const string& special = ""s);

   void AddPlaylist(PUPPlaylist* pPlaylist);
   PUPPlaylist* GetPlaylist(const string& szFolder);

   void SetMask(const std::filesystem::path& path);

   void SetGameTime(double gameTime);
   void Update(double gameTime);

   void Play(const string& szPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action = PlayAction::Normal, int length = 0);
   void Play(PUPPlaylist* playlist, const std::filesystem::path& szPlayFile, float volume, int priority, PlayAction action, int length);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile);
   void Pause();
   void Resume();
   void SetLoop(int state);
   void SetLength(int seconds);
   void SetAsBackGround(int mode);
   void SetFadeStep(int step);

   bool IsBackgroundPlaying() const;
   bool IsMainPlaying() const;

   void Render(VPXRenderContext2D* const ctx, int pass);

   static const string& ToString(Mode mode);

private:
   void LoadTriggers();
   void UpdateTimers();
   int ResolveVolume(float volume, const PUPPlaylist* pPlaylist) const;
   float EffectiveVolume(int volume) const;
   void ApplyVolume(int volume);
   void StartVolumeFade(int from, int to, int step);

   PUPManager* const m_pManager = nullptr;
   const int m_screenNum;
   const string m_screenDes;

   Mode m_mode;
   bool m_topmost = false;
   bool m_hidden = false;
   bool m_transparent;
   float m_mainVolume = 1.f;
   int m_volumeMultiplier;
   int m_defaultVolume = 100;
   int m_currentVolume = 100;
   bool m_duckMaster = false;
   int m_duckOrigVolume = -1;
   int m_defaultDuck = -1;
   int m_fadeVolume = -1;
   int m_fadeTarget = 0;
   int m_volumeFadeStep = 0;
   uint64_t m_fadeNextTick = 0;
   std::unique_ptr<PUPCustomPos> m_pCustomPos;
   SDL_Rect m_rect {};
   vector<PUPLabel*> m_labels;
   ankerl::unordered_dense::map<string, PUPLabel*> m_labelMap;
   ankerl::unordered_dense::map<string, PUPPlaylist*> m_playlistMap;
   ankerl::unordered_dense::map<string, vector<PUPTrigger*>> m_triggerMap;
   PUPImage m_background;    // PuPFrames playlist - underlay behind video
   PUPImage m_staticImage;   // Static PNG/JPG from Default function - renders on video layer, bypasses FFmpeg
   std::unique_ptr<PUPMediaManager> m_pMediaPlayerManager;
   int m_pagenum = 0;
   int m_defaultPagenum = 0;
   uint64_t m_pageExpiry = 0;
   uint64_t m_imageExpiry = 0;
   PUPScreen* m_pParent = nullptr;
   vector<std::shared_ptr<PUPScreen>> m_children;
   const std::thread::id m_apiThread;
};

}
