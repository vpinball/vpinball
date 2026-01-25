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
      float volume, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists);
   ~PUPScreen();

   static std::unique_ptr<PUPScreen> CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists);
   static std::unique_ptr<PUPScreen> CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists);

   PUPManager* GetManager() const { return m_pManager; }
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   string ToString(bool full = true) const;

   Mode GetMode() const { return m_mode; }
   void SetMode(Mode mode) { m_mode = mode; }
   bool IsPop() const { return m_mode == PUPScreen::Mode::ForcePopBack || m_mode == PUPScreen::Mode::ForcePop; }

   bool IsTransparent() const { return m_transparent; }

   float GetVolume() const { return m_volume; }
   void SetMainVolume(float volume); // Set user defined global volume (allow to mute)
   void SetVolume(float volume); // Set default, and apply it to played media
   void SetVolumeCurrent(float volume); // Only modifiy volume of currently playing medias

   const std::unique_ptr<PUPCustomPos>& GetCustomPos() const { return m_pCustomPos; }
   void SetCustomPos(const string& szCustomPos);
   void SetBounds(int x, int y, int w, int h);

   void AddChild(std::shared_ptr<PUPScreen> pScreen);
   void ReplaceChild(std::shared_ptr<PUPScreen> pChild, std::shared_ptr<PUPScreen> pScreen);
   PUPScreen* GetParent() const { return m_pParent; }

   void AddTrigger(PUPTrigger* pTrigger);
   vector<PUPTrigger*>* GetTriggers(const string& szTrigger);
   const ankerl::unordered_dense::map<string, vector<PUPTrigger*>>& GetTriggers() const { return m_triggerMap; }

   bool IsLabelInit() const { return m_labelInit; }
   void SetLabelInit() { m_labelInit = true; }
   void AddLabel(PUPLabel* pLabel);
   PUPLabel* GetLabel(const string& labelName);
   void SendLabelToFront(PUPLabel* pLabel);
   void SendLabelToBack(PUPLabel* pLabel);
   void SetPage(int pagenum, int seconds);

   void AddPlaylist(PUPPlaylist* pPlaylist);
   PUPPlaylist* GetPlaylist(const string& szFolder);

   void SetMask(const std::filesystem::path& path);

   void SetGameTime(double gameTime);

   void Play(const string& szPlaylist, const std::filesystem::path& szPlayFile, float volume, int priority);
   void Play(PUPPlaylist* playlist, const std::filesystem::path& szPlayFile, float volume, int priority, bool skipSamePriority, int length, bool background);
   void Stop();
   void Stop(int priority);
   void Stop(PUPPlaylist* pPlaylist, const std::filesystem::path& szPlayFile);
   void Pause();
   void Resume();
   void SetLoop(int state);
   void SetLength(int length);
   void SetAsBackGround(int mode);

   bool HasUnderlay() const { return !m_background.GetFile().empty(); }
   bool IsBackgroundPlaying() const;
   bool IsMainPlaying() const;
   bool HasOverlay() const { return !m_overlay.GetFile().empty(); }

   const SDL_Rect& GetRect() const { return m_rect; }
   void Render(VPXRenderContext2D* const ctx, int pass);

   static const string& ToString(Mode mode);

private:
   void LoadTriggers();

   static uint32_t PageTimerElapsed(void* param, SDL_TimerID timerID, uint32_t interval);

   PUPManager* const m_pManager = nullptr;
   const int m_screenNum;
   const string m_screenDes;

   Mode m_mode;
   bool m_transparent;
   float m_mainVolume = 1.f;
   float m_volume;
   std::unique_ptr<PUPCustomPos> m_pCustomPos;
   SDL_Rect m_rect;
   vector<PUPLabel*> m_labels;
   ankerl::unordered_dense::map<string, PUPLabel*> m_labelMap;
   ankerl::unordered_dense::map<string, PUPPlaylist*> m_playlistMap;
   ankerl::unordered_dense::map<string, vector<PUPTrigger*>> m_triggerMap;
   PUPImage m_background;
   PUPImage m_overlay;
   std::unique_ptr<PUPMediaManager> m_pMediaPlayerManager;
   bool m_labelInit = false;
   int m_pagenum = 0;
   int m_defaultPagenum = 0;
   SDL_TimerID m_pageTimer = 0;
   PUPScreen* m_pParent = nullptr;
   vector<std::shared_ptr<PUPScreen>> m_children;
   const std::thread::id m_apiThread;
   std::mutex m_screenMutex;
};

}
