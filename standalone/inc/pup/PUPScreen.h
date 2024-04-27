#pragma once

#include "PUPManager.h"

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#define VIDEO_WINDOW_HAS_FFMPEG_LIBS 1
#endif

class PUPCustomPos;
class PUPTrigger;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
class PUPMediaPlayer;
#endif

class PUPScreen {
public:
   ~PUPScreen();

   static PUPScreen* CreateFromCSVLine(PUPManager* pManager, string line);

   PUP_SCREEN_MODE GetMode() const { return m_mode; }
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   bool IsTransparent() const { return m_transparent; }
   int GetVolume() const { return m_volume; }
   PUPCustomPos* GetCustomPos() { return m_pCustomPos; }
   void AddChild(PUPScreen* pScreen) { m_children.push_back(pScreen); }
   const vector<PUPScreen*>& GetChildren() const { return m_children; }
   void AddTrigger(PUPTrigger* pTrigger);
   void Init(SDL_Renderer* pRenderer, int w, int h);
   void Render();
   const SDL_Rect& GetRect() const { return m_rect; }
   string ToString() const;
   void Trigger(const string& szTrigger);

private:
   PUPScreen(PUPManager* m_pManager);

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
   vector<PUPScreen*> m_children;
   std::map<string, PUPTrigger*> m_triggerMap;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   PUPMediaPlayer* m_pMediaPlayer;
#endif
   SDL_Renderer* m_pRenderer;
   SDL_Surface* m_pOverlay;
   SDL_Texture* m_pOverlayTexture;
};

