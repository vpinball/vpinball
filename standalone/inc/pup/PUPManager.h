#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <SDL3_ttf/SDL_ttf.h>

#define PUP_SCREEN_TOPPER             0
#define PUP_SCREEN_BACKGLASS          2
#define PUP_SCREEN_DMD                1
#define PUP_SCREEN_PLAYFIELD          3
#define PUP_SCREEN_FULLDMD            5

typedef struct {
   char type;
   int number;
   int value;
} PUPTriggerData;

class PUPScreen;
class PUPPlaylist;
class PUPTrigger;
class PUPWindow;

class PUPManager final
{
public:
   PUPManager();
   ~PUPManager();

   const string& GetRootPath() const { return m_szRootPath; }

   bool IsInit() const { return m_init; }
   void LoadConfig(const string& szRomName);
   void Unload();
   const string& GetPath() const { return m_szPath; }
   bool AddScreen(PUPScreen* pScreen);
   bool AddScreen(int screenNum);
   bool HasScreen(int screenNum);
   PUPScreen* GetScreen(int screenNum) const;
   bool AddFont(TTF_Font* pFont, const string& szFilename);
   TTF_Font* GetFont(const string& szFamily);
   void QueueTriggerData(const PUPTriggerData& data);
   int GetTriggerValue(const string& triggerId);
   void Start();
   void Stop();

private:
   void LoadPlaylists();
   void ProcessQueue();
   void AddWindow(const string& szWindowName, int defaultScreen, VPXAnciliaryWindow anciliaryWindow);

   bool m_init;
   string m_szRootPath;
   string m_szPath;
   ankerl::unordered_dense::map<int, PUPScreen*> m_screenMap;
   vector<TTF_Font*> m_fonts;
   ankerl::unordered_dense::map<string, TTF_Font*> m_fontMap;
   ankerl::unordered_dense::map<string, TTF_Font*> m_fontFilenameMap;
   vector<PUPWindow*> m_windows;
   std::queue<PUPTriggerData> m_triggerDataQueue;
   std::mutex m_queueMutex;
   std::condition_variable m_queueCondVar;
   bool m_isRunning;
   std::thread m_thread;
   vector<PUPPlaylist*> m_playlists;
   ankerl::unordered_dense::map<string, int> m_triggerMap;
};
