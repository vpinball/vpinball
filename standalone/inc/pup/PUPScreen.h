#pragma once

#include "PUPManager.h"

#include "../common/Timer.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class PUPCustomPos;
class PUPTrigger;
class PUPMediaPlayer;
class PUPLabel;


struct PUPScreenRequest
{
   PUP_TRIGGER_PLAY_ACTION action;
   PUPPlaylist* pPlaylist;
   string szPlayFile;
   int volume;
   int priority;
};

struct PUPScreenRenderable
{
   SDL_Surface* pSurface;
   SDL_Texture* pTexture;
   bool dirty;
};

class PUPScreen {
public:
   ~PUPScreen();

   static PUPScreen* CreateFromCSV(const string& line);
   PUP_SCREEN_MODE GetMode() const { return m_mode; }
   void SetMode(PUP_SCREEN_MODE mode) { m_mode = mode; }
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   bool IsTransparent() const { return m_transparent; }
   int GetVolume() const { return m_volume; }
   PUPCustomPos* GetCustomPos() const { return m_pCustomPos; }
   void AddChild(PUPScreen* pScreen);
   void SetParent(PUPScreen* pParent) { m_pParent = pParent; }
   PUPScreen* GetParent() const { return m_pParent; }
   void SendToFront();
   void SetSize(int w, int h);
   void Init(SDL_Renderer* pRenderer);
   bool IsLabelInit() const { return m_labelInit; }
   void SetLabelInit() { m_labelInit = true; }
   void AddLabel(PUPLabel* pLabel);
   PUPLabel* GetLabel(const string& labelName);
   void SendLabelToFront(PUPLabel* pLabel);
   void SendLabelToBack(PUPLabel* pLabel);
   void SetPage(int pagenum, int seconds);
   void Render();
   const SDL_Rect& GetRect() const { return m_rect; }
   void ProcessTrigger(PUPTrigger* pTrigger);
   void PlayEx(const string& szPlaylist, const string& szPlayfilename, int volume, int priority);
   void Stop();
   void SetLoop(int state);
   void SetBackGround(int mode);
   void SetBackground(const std::string& file);
   void SetMedia(const std::string& file, int volume);
   void SetOverlay(const std::string& file);
   string ToString(bool full = true) const;

private:
   PUPScreen();

   void ProcessQueue();
   void LoadRenderable(PUPScreenRenderable* pRenderable, const string& szFile);
   void Render(PUPScreenRenderable* pRenderable);
   void FreeRenderable(PUPScreenRenderable* pRenderable);
   void PageTimerElapsed(VP::Timer* pTimer);

   PUPManager* m_pManager;
   PUP_SCREEN_MODE m_mode;
   int m_screenNum;
   string m_screenDes;
   string m_backgroundPlaylist;
   string m_backgroundFilename;
   bool m_transparent;
   int m_volume;
   PUPCustomPos* m_pCustomPos;
   SDL_Rect m_rect;
   PUPScreen* m_pParent;
   vector<PUPLabel*> m_labels;
   std::map<string, PUPLabel*> m_labelMap;
   SDL_Renderer* m_pRenderer;
   PUPScreenRenderable m_background;
   PUPScreenRenderable m_overlay;
   PUPMediaPlayer* m_pMediaPlayer;
   bool m_labelInit;
   int m_pagenum;
   int m_defaultPagenum;
   VP::Timer* m_pPageTimer;
   vector<PUPScreen*> m_topChildren;
   vector<PUPScreen*> m_backChildren;
   vector<PUPScreen*> m_defaultChildren;
   std::queue<PUPScreenRequest> m_requestQueue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning;
   std::thread m_playThread;
   std::mutex m_renderMutex;
};
