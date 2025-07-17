#pragma once

#include "PUPManager.h"

#include "PUPTrigger.h"

#include "PUPPlaylist.h"

#include "PUPImage.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace PUP {

class PUPCustomPos;
class PUPMediaManager;
class PUPLabel;

typedef enum
{
   PUP_SCREEN_MODE_OFF,
   PUP_SCREEN_MODE_SHOW,            // This will use whatever settings are set for this screen’s “default” settings.
   PUP_SCREEN_MODE_FORCE_ON,        // Forces the window to be the Top most window, and it stays on. A video is always going to be playing in this mode. When a new video starts or the current video restarts, the ForceON action happens and will force the window to be the Top window again.
   PUP_SCREEN_MODE_FORCE_POP,       // Similar to ForceOn, except the vlc window opens and closes with each video played
   PUP_SCREEN_MODE_FORCE_BACK,      // Keeps the video window always open, but under the other PuP screens. This can cause any PuP screen with this setting to get pushed behind your game window.
   PUP_SCREEN_MODE_FORCE_POP_BACK,  // Similar to ForceBack, except the vlc window opens and closes with each video played
   PUP_SCREEN_MODE_MUSIC_ONLY
} PUP_SCREEN_MODE;

typedef enum
{
   PUP_PINDISPLAY_REQUEST_TYPE_NORMAL,
   PUP_PINDISPLAY_REQUEST_TYPE_LOOP,
   PUP_PINDISPLAY_REQUEST_TYPE_SET_BG,
   PUP_PINDISPLAY_REQUEST_TYPE_PAUSE,
   PUP_PINDISPLAY_REQUEST_TYPE_RESUME,
   PUP_PINDISPLAY_REQUEST_TYPE_STOP
} PUP_PINDISPLAY_REQUEST_TYPE;

const char* PUP_SCREEN_MODE_TO_STRING(PUP_SCREEN_MODE value);
const char* PUP_PINDISPLAY_REQUEST_TYPE_TO_STRING(PUP_PINDISPLAY_REQUEST_TYPE value);

class PUPScreenRequest
{
public:
   virtual ~PUPScreenRequest() = default;
};

class PUPPinDisplayRequest final : public PUPScreenRequest
{
public:
   const PUP_PINDISPLAY_REQUEST_TYPE type;
   PUPPlaylist* pPlaylist = nullptr;
   string szPlayFile;
   float volume = 0.f;
   int priority = 0;
   int value = 0;
   
public:
   PUPPinDisplayRequest(PUP_PINDISPLAY_REQUEST_TYPE _type)
      : type(_type)
   {
   }
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
   ~PUPScreen();

   static PUPScreen* CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists);
   static PUPScreen* CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists);
   
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   string ToString(bool full = true) const;

   PUP_SCREEN_MODE GetMode() const { return m_mode; }
   void SetMode(PUP_SCREEN_MODE mode) { m_mode = mode; }

   bool IsTransparent() const { return m_transparent; }

   float GetVolume() const { return m_volume; }
   void SetVolume(float volume); // Set default, and apply it to played media
   void SetVolumeCurrent(float volume); // Only modifiy volume of currently playing medias

   PUPCustomPos* GetCustomPos() const { return m_pCustomPos; }
   void SetCustomPos(const string& szCustomPos);
   void SetSize(int w, int h);

   void AddChild(PUPScreen* pScreen);
   void SetParent(PUPScreen* pParent) { m_pParent = pParent; }
   PUPScreen* GetParent() const { return m_pParent; }
   bool HasParent() const { return m_pParent != nullptr;}
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

   // Background and overlay images
   void SetBackground(PUPPlaylist* pPlaylist, const std::string& szPlayFile);
   void SetOverlay(PUPPlaylist* pPlaylist, const std::string& szPlayFile);
   
   void AddPlaylist(PUPPlaylist* pPlaylist);
   PUPPlaylist* GetPlaylist(const string& szFolder);
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   void SetLoop(int state);
   void SetBG(int mode);
   void QueuePlay(const string& szPlaylist, const string& szPlayFile, float volume, int priority);
   void QueuePause() { QueueRequest(new PUPPinDisplayRequest(PUP_PINDISPLAY_REQUEST_TYPE_PAUSE)); }
   void QueueResume() { QueueRequest(new PUPPinDisplayRequest(PUP_PINDISPLAY_REQUEST_TYPE_RESUME)); }
   void QueueStop() { QueueRequest(new PUPPinDisplayRequest(PUP_PINDISPLAY_REQUEST_TYPE_STOP)); }
   void QueueLoop(int state);
   void QueueBG(int mode);
   void QueueTriggerRequest(PUPTriggerRequest* pRequest);

   void SetActive(bool active) { m_active = active; }
   bool IsActive() const { return m_active; }

   void Start();
   const SDL_Rect& GetRect() const { return m_rect; }
   void Render(VPXRenderContext2D* const ctx);

private:
   PUPScreen(PUPManager* manager, PUP_SCREEN_MODE mode, int screenNum, const string& screenDes, const string& backgroundPlaylist, const string& backgroundFilename, bool transparent, float volume, PUPCustomPos* pCustomPos, const std::vector<PUPPlaylist*>& playlists);

   void LoadTriggers();
   void QueueRequest(PUPPinDisplayRequest* request);
   void ProcessQueue();
   void ProcessPinDisplayRequest(PUPPinDisplayRequest* pRequest);
   void ProcessTriggerRequest(PUPTriggerRequest* pRequest);

   void SetMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void StopMedia();
   void StopMedia(int priority);
   void StopMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile);

   static uint32_t PageTimerElapsed(void* param, SDL_TimerID timerID, uint32_t interval);

   PUPManager* const m_pManager = nullptr;
   const int m_screenNum;
   const string m_screenDes;

   bool m_active = false;
   PUP_SCREEN_MODE m_mode;
   string m_backgroundPlaylist;
   string m_backgroundFilename;
   bool m_transparent;
   float m_volume;
   PUPCustomPos* m_pCustomPos;
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
   vector<PUPScreen*> m_topChildren;
   vector<PUPScreen*> m_backChildren;
   vector<PUPScreen*> m_defaultChildren;
   std::queue<PUPScreenRequest*> m_queue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning = false;
   std::thread m_thread;
   std::mutex m_renderMutex;
};

}