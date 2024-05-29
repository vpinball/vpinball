#pragma once

#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <SDL2/SDL_ttf.h>

#define PUP_SETTINGS_BACKGLASSX      320
#define PUP_SETTINGS_BACKGLASSY      30
#define PUP_SETTINGS_BACKGLASSWIDTH  290
#define PUP_SETTINGS_BACKGLASSHEIGHT 218
#define PUP_ZORDER_BACKGLASS         150

#define PUP_SETTINGS_TOPPERX         320
#define PUP_SETTINGS_TOPPERY         (PUP_SETTINGS_BACKGLASSY + PUP_SETTINGS_BACKGLASSHEIGHT + 5)
#define PUP_SETTINGS_TOPPERWIDTH     290
#define PUP_SETTINGS_TOPPERHEIGHT    75
#define PUP_ZORDER_TOPPER            300

typedef struct {
   char type;
   int number;
   int value;
} PUPTriggerData;

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;
class PUPWindow;

class PUPManager
{
public:
   ~PUPManager();

   static PUPManager* GetInstance();
   bool IsInit() const { return m_init; }
   bool LoadConfig(const string& szRomName);
   const string& GetPath() { return m_szPath; }
   bool AddScreen(PUPScreen* pScreen);
   PUPScreen* GetScreen(int screenNum);
   bool AddFont(TTF_Font* pFont);
   TTF_Font* GetFont(const string& szFamily);
   void QueueTriggerData(PUPTriggerData data);
   void Start();
   void Stop();

private:
   PUPManager();

   void ProcessQueue();

   static PUPManager* m_pInstance;
   bool m_init;
   string m_szPath;
   std::map<int, PUPScreen*> m_screenMap;
   std::map<string, TTF_Font*> m_fontMap;
   PUPWindow* m_pBackglassWindow;
   PUPWindow* m_pTopperWindow;
   std::queue<PUPTriggerData> m_triggerDataQueue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning;
   std::thread m_thread;
};
