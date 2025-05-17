#pragma once

#include "PUPManager.h"

#include "PUPTrigger.h"

#include "PUPPlaylist.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

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
   PUP_PINDISPLAY_REQUEST_TYPE_STOP
} PUP_PINDISPLAY_REQUEST_TYPE;

const char* PUP_SCREEN_MODE_TO_STRING(PUP_SCREEN_MODE value);
const char* PUP_PINDISPLAY_REQUEST_TYPE_TO_STRING(PUP_PINDISPLAY_REQUEST_TYPE value);

struct PUPScreenRequest
{
   virtual ~PUPScreenRequest() = default;
};

struct PUPPinDisplayRequest : PUPScreenRequest
{
   PUP_PINDISPLAY_REQUEST_TYPE type = PUP_PINDISPLAY_REQUEST_TYPE_NORMAL;
   PUPPlaylist* pPlaylist = nullptr;
   string szPlayFile;
   float volume = 0.f;
   int priority = 0;
   int value = 0;

   PUPPinDisplayRequest()
   {
   }
};

struct PUPTriggerRequest : PUPScreenRequest
{
   PUPTrigger* pTrigger;
   int value;
};

struct PUPScreenRenderable
{
   SDL_Surface* pSurface = nullptr;
   VPXTexture pTexture = nullptr;
   bool dirty = true;
};

class PUPScreen final
{
public:
   ~PUPScreen();

   static PUPScreen* CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists);
   static PUPScreen* CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists);
   PUP_SCREEN_MODE GetMode() const { return m_mode; }
   void SetMode(PUP_SCREEN_MODE mode) { m_mode = mode; }
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   bool IsTransparent() const { return m_transparent; }
   float GetVolume() const { return m_volume; }
   void SetVolume(float volume) { m_volume = volume; }
   PUPCustomPos* GetCustomPos() const { return m_pCustomPos; }
   void AddChild(PUPScreen* pScreen);
   void SetParent(PUPScreen* pParent) { m_pParent = pParent; }
   PUPScreen* GetParent() const { return m_pParent; }
   bool HasParent() const { return m_pParent != nullptr;}
   void AddPlaylist(PUPPlaylist* pPlaylist);
   PUPPlaylist* GetPlaylist(const string& szFolder);
   void AddTrigger(PUPTrigger* pTrigger);
   vector<PUPTrigger*>* GetTriggers(const string& szTrigger);
   const ankerl::unordered_dense::map<string, vector<PUPTrigger*>>& GetTriggers() const { return m_triggerMap; }
   void SendToFront();
   void SetSize(int w, int h);
   void Init();
   void Start();
   bool IsLabelInit() const { return m_labelInit; }
   void SetLabelInit() { m_labelInit = true; }
   void AddLabel(PUPLabel* pLabel);
   PUPLabel* GetLabel(const string& labelName);
   void SendLabelToFront(PUPLabel* pLabel);
   void SendLabelToBack(PUPLabel* pLabel);
   void SetPage(int pagenum, int seconds);
   void Render(VPXRenderContext2D* const ctx);
   const SDL_Rect& GetRect() const { return m_rect; }
   void SetBackground(PUPPlaylist* pPlaylist, const std::string& szPlayFile);
   void SetCustomPos(const string& szCustomPos);
   void SetOverlay(PUPPlaylist* pPlaylist, const std::string& szPlayFile);
   void SetMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile, float volume, int priority, bool skipSamePriority, int length);
   void StopMedia();
   void StopMedia(int priority);
   void StopMedia(PUPPlaylist* pPlaylist, const std::string& szPlayFile);
   void SetLoop(int state);
   void SetBG(int mode);
   void QueuePlay(const string& szPlaylist, const string& szPlayFile, float volume, int priority);
   void QueueStop();
   void QueueLoop(int state);
   void QueueBG(int mode);
   void QueueTriggerRequest(PUPTriggerRequest* pRequest);
   string ToString(bool full = true) const;

   void SetActive(bool active) { m_active = active; }
   bool IsActive() const { return m_active; }

private:
   PUPScreen(PUPManager* manager, PUP_SCREEN_MODE mode, int screenNum, const string& screenDes, const string& backgroundPlaylist, const string& backgroundFilename, bool transparent, float volume, PUPCustomPos* pCustomPos, const std::vector<PUPPlaylist*>& playlists);

   void LoadTriggers();
   void ProcessQueue();
   void ProcessPinDisplayRequest(PUPPinDisplayRequest* pRequest);
   void ProcessTriggerRequest(PUPTriggerRequest* pRequest);
   void LoadRenderable(PUPScreenRenderable* pRenderable, const string& szFile);
   void Render(VPXRenderContext2D* const ctx, PUPScreenRenderable* pRenderable);
   void FreeRenderable(PUPScreenRenderable* pRenderable);
   static Uint32 PageTimerElapsed(void* param, SDL_TimerID timerID, Uint32 interval);

   PUPManager* const m_pManager = nullptr;
   bool m_active = false;
   PUP_SCREEN_MODE m_mode;
   const int m_screenNum;
   string m_screenDes;
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
   PUPScreenRenderable m_background;
   PUPScreenRenderable m_overlay;
   PUPMediaManager* m_pMediaPlayerManager;
   bool m_labelInit;
   int m_pagenum;
   int m_defaultPagenum;
   SDL_TimerID m_pageTimer = 0;
   PUPScreen* m_pParent;
   vector<PUPScreen*> m_topChildren;
   vector<PUPScreen*> m_backChildren;
   vector<PUPScreen*> m_defaultChildren;
   std::queue<PUPScreenRequest*> m_queue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning;
   std::thread m_thread;
   std::mutex m_renderMutex;
};
