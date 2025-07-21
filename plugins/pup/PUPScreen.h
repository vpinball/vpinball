#pragma once

#include "PUPManager.h"

#include "PUPTrigger.h"

#include "PUPPlaylist.h"

#include "PUPImage.h"

#include "PUPCustomPos.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace PUP {

class PUPMediaManager;
class PUPLabel;

class PUPScreenRequest
{
public:
   virtual ~PUPScreenRequest() = default;
};

class PUPPinDisplayRequest final : public PUPScreenRequest
{
public:
   enum class Type
   {
      Normal,
      Loop,
      SetBackground,
      Pause,
      Resume,
      Stop
   };

   PUPPinDisplayRequest(Type _type, int value = 0)
      : type(_type)
	  , value(value)
   {
   }

   static const string& ToString(Type value);

public:
   const PUPPinDisplayRequest::Type type;
   PUPPlaylist* pPlaylist = nullptr;
   string szPlayFile;
   float volume = 0.f;
   int priority = 0;
   int value = 0;
};

class PUPTriggerRequest final : public PUPScreenRequest
{
public:
   PUPTrigger* pTrigger;
   int value;
};


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

   PUPScreen(PUPManager* manager, PUPScreen::Mode mode, int screenNum, const string& screenDes, const string& backgroundPlaylist, const string& backgroundFilename, bool transparent,
      float volume, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists);
   ~PUPScreen();

   static std::unique_ptr<PUPScreen> CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists);
   static std::unique_ptr<PUPScreen> CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists);
   
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   string ToString(bool full = true) const;

   PUPScreen::Mode GetMode() const { return m_mode; }
   void SetMode(PUPScreen::Mode mode) { m_mode = mode; }

   bool IsTransparent() const { return m_transparent; }

   float GetVolume() const { return m_volume; }
   void SetVolume(float volume); // Set default, and apply it to played media
   void SetVolumeCurrent(float volume); // Only modifiy volume of currently playing medias

   const std::unique_ptr<PUPCustomPos>& GetCustomPos() const { return m_pCustomPos; }
   void SetCustomPos(const string& szCustomPos);
   void SetSize(int w, int h);

   void AddChild(std::shared_ptr<PUPScreen> pScreen);
   void SendToFront();

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
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   void SetLoop(int state);
   void SetBG(int mode);
   void QueuePlay(const string& szPlaylist, const string& szPlayFile, float volume, int priority);
   void QueuePause() { QueueRequest(new PUPPinDisplayRequest(PUPPinDisplayRequest::Type::Pause)); }
   void QueueResume() { QueueRequest(new PUPPinDisplayRequest(PUPPinDisplayRequest::Type::Resume)); }
   void QueueStop() { QueueRequest(new PUPPinDisplayRequest(PUPPinDisplayRequest::Type::Stop)); }
   void QueueLoop(int state) { QueueRequest(new PUPPinDisplayRequest(PUPPinDisplayRequest::Type::Loop, state)); }
   void QueueBG(int mode) { QueueRequest(new PUPPinDisplayRequest(PUPPinDisplayRequest::Type::SetBackground, mode)); }
   void QueueTriggerRequest(PUPTriggerRequest* pRequest);

   void SetActive(bool active) { m_active = active; }
   bool IsActive() const { return m_active; }

   void Start();
   const SDL_Rect& GetRect() const { return m_rect; }
   void Render(VPXRenderContext2D* const ctx);

   static const string& ToString(Mode mode);

private:
   void LoadTriggers();
   void QueueRequest(PUPPinDisplayRequest* request);
   void ProcessQueue();
   void ProcessPinDisplayRequest(PUPPinDisplayRequest* pRequest);
   void ProcessTriggerRequest(PUPTriggerRequest* pRequest);
   void ProcessPlaylistRequest(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, int length);

   void SetMask(const string& path);
   void SetMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void StopMedia();
   void StopMedia(int priority);
   void StopMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile);

   static uint32_t PageTimerElapsed(void* param, SDL_TimerID timerID, uint32_t interval);

   PUPManager* const m_pManager = nullptr;
   const int m_screenNum;
   const string m_screenDes;

   bool m_active = false;
   Mode m_mode;
   string m_backgroundPlaylist;
   string m_backgroundFilename;
   bool m_transparent;
   float m_volume;
   std::unique_ptr<PUPCustomPos> m_pCustomPos;
   SDL_Rect m_rect;
   std::shared_ptr<SDL_Surface> m_mask = nullptr;
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
   vector<std::shared_ptr<PUPScreen>> m_topChildren;
   vector<std::shared_ptr<PUPScreen>> m_backChildren;
   vector<std::shared_ptr<PUPScreen>> m_defaultChildren;
   std::queue<PUPScreenRequest*> m_queue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning = false;
   std::thread m_thread;
   std::mutex m_renderMutex;
};

}