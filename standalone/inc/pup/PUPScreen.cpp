#include "stdafx.h"

#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"
#include "PUPManager.h"
#include "PUPPlaylist.h"
#include "PUPLabel.h"
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
#include "PUPMediaPlayer.h"
#endif

#include <SDL2/SDL_image.h>

/*
   screens.pup: ScreenNum,ScreenDes,PlayList,PlayFile,Loopit,Active,Priority,CustomPos
   PuP Pack Editor: Mode,ScreenNum,ScreenDes,Background Playlist,Background Filename,Transparent,CustomPos,Volume %

   mappings:

     ScreenNum = ScreenNum
     ScreenDes = ScreenDes
     PlayList = Background Playlist
     PlayFile = Background Filename
     Loopit = Transparent
     Active = Mode
     Priority = Volume %
     CustomPos = CustomPos
*/

PUPScreen::PUPScreen(PUPManager* pManager)
{
   m_pManager = pManager;

   m_pCustomPos = nullptr;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   m_pMediaPlayer = nullptr;
#endif
   m_pOverlay = NULL;
   m_pOverlayTexture = NULL;

   m_labelInit = false;
   m_pagenum = 0;
   m_defaultPagenum = 0;
   m_pPageTimer = new VP::Timer();
   m_pPageTimer->SetElapsedListener(std::bind(&PUPScreen::PageTimerElapsed, this, std::placeholders::_1));
}

PUPScreen::~PUPScreen()
{
   delete m_pPageTimer;

   delete m_pCustomPos;
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   delete m_pMediaPlayer;
#endif

   for (auto& [key, pTrigger] : m_triggerMap)
      delete pTrigger;

   for (auto& [key, pLabel] : m_labelMap)
      delete pLabel;

   m_children.clear();

   if (m_pOverlay)
      SDL_FreeSurface(m_pOverlay);

   if (m_pOverlayTexture)
      SDL_DestroyTexture(m_pOverlayTexture);
}

PUPScreen* PUPScreen::CreateFromCSVLine(PUPManager* pManager, string line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8)
      return nullptr;

   PUPScreen* pScreen = new PUPScreen(pManager);

   string mode = parts[5];
   if (string_compare_case_insensitive(mode, "Show"))
      pScreen->m_mode = PUP_SCREEN_MODE_SHOW;
   else if (string_compare_case_insensitive(mode, "ForceON"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_ON;
   else if (string_compare_case_insensitive(mode, "ForcePoP"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_POP;
   else if (string_compare_case_insensitive(mode, "ForceBack"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_BACK;
   else if (string_compare_case_insensitive(mode, "ForcePopBack"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_POP_BACK;
   else
      pScreen->m_mode = PUP_SCREEN_MODE_OFF;

   pScreen->m_screenNum = string_to_int(parts[0], 0);
   pScreen->m_screenDes = parts[1];
   pScreen->m_backgroundPlaylist = parts[2];
   pScreen->m_backgroundFilename = parts[3];
   pScreen->m_transparent = (parts[4] == "1");
   pScreen->m_volume = string_to_int(parts[6], 0);
   pScreen->m_pCustomPos = PUPCustomPos::CreateFromCSVLine(parts[7]);

   return pScreen;
}

void PUPScreen::PageTimerElapsed(VP::Timer* pTimer)
{
   m_pPageTimer->Stop();
   m_pagenum = m_defaultPagenum;
}

void PUPScreen::UpdateSize(int w, int h)
{
   if (!m_pCustomPos)
      m_rect = { 0, 0, w, h };
   else {
      m_rect = {
         (int)((m_pCustomPos->GetXPos() / 100.) * w),
         (int)((m_pCustomPos->GetYPos() / 100.) * h),
         (int)((m_pCustomPos->GetWidth() / 100.) * w),
         (int)((m_pCustomPos->GetHeight() / 100.) * h)
      };
   }

   for (PUPScreen* pScreen : GetChildren())
      pScreen->UpdateSize(w, h);
}

void PUPScreen::AddTrigger(PUPTrigger* pTrigger)
{
   std::map<string, PUPTrigger*>::iterator it = m_triggerMap.find(pTrigger->GetTrigger());
   if (it != m_triggerMap.end()) {
      PLOGW.printf("Duplicate trigger: screenNum=%d, new={%s}, existing={%s}", m_screenNum,
         pTrigger->ToString().c_str(), it->second->ToString().c_str());
   }
   else
      m_triggerMap[pTrigger->GetTrigger()] = pTrigger;
}

void PUPScreen::Init(SDL_Renderer* pRenderer)
{
   m_pRenderer = pRenderer;

#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   m_pMediaPlayer = new PUPMediaPlayer(pRenderer, &m_rect);

   PUPPlaylist* pPlaylist = m_pManager->GetPlaylist(m_backgroundPlaylist);
   if (pPlaylist) {
      string szPath = m_pManager->GetPath(pPlaylist, m_backgroundFilename);
      if (!szPath.empty())
         m_pMediaPlayer->SetBG(szPath, GetVolume(), 0);
   }
#endif

   for (PUPScreen* pScreen : GetChildren())
      pScreen->Init(pRenderer);
}

void PUPScreen::AddLabel(const string& szLabelName, PUPLabel* pLabel)
{
   m_labelMap[string_to_lower(szLabelName)] = pLabel;
}

PUPLabel* PUPScreen::GetLabel(const string& szLabelName)
{
   std::map<string, PUPLabel*>::iterator it = m_labelMap.find(string_to_lower(szLabelName));
   return it != m_labelMap.end() ? it->second : nullptr;
}

void PUPScreen::SetPage(int pagenum, int seconds)
{
   m_pPageTimer->Stop();
   m_pagenum = pagenum;

   if (seconds == 0)
      m_defaultPagenum = pagenum;
   else {
      m_pPageTimer->SetInterval(seconds * 1000);
      m_pPageTimer->Start();
   }
}

void PUPScreen::Render()
{
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
   if (m_pMediaPlayer)
      m_pMediaPlayer->Render();
#endif

   for (auto& [key, pLabel] : m_labelMap)
      pLabel->Render(m_pRenderer, m_rect, m_pagenum);

   for (PUPScreen* pScreen : GetChildren())
      pScreen->Render();

   if (m_pOverlay) {
      if (!m_pOverlayTexture)
         m_pOverlayTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pOverlay);

      if (m_pOverlayTexture)
         SDL_RenderCopy(m_pRenderer, m_pOverlayTexture, NULL, &m_rect);
   }
}

void PUPScreen::Trigger(const string& szTrigger)
{
   std::map<string, PUPTrigger*>::iterator it = m_triggerMap.find(szTrigger);
   if (it != m_triggerMap.end()) {
      PUPTrigger* pTrigger = it->second;
      PLOGD.printf("Found pup trigger: trigger={%s}", pTrigger->ToString().c_str());
      PUPPlaylist* pPlaylist = m_pManager->GetPlaylist(pTrigger->GetPlaylist());
      if (pPlaylist) {
         string szPlayfile = pTrigger->GetPlayFile();
         string szPath = m_pManager->GetPath(pPlaylist, szPlayfile);
         if (szPath.empty())
            return;
         if (pPlaylist->GetFunction() == PUP_PLAYLIST_FUNCTION_OVERLAYS) {
            if (m_pOverlay)
               SDL_FreeSurface(m_pOverlay);
            m_pOverlay = IMG_Load(szPath.c_str());
         }
         else {
#ifdef VIDEO_WINDOW_HAS_FFMPEG_LIBS
            if (m_pMediaPlayer)
               m_pMediaPlayer->Play(szPath, (!szPlayfile.empty() ? pTrigger->GetVolume() : pPlaylist->GetVolume()),
                  pTrigger->GetPlayAction(), pTrigger->GetPriority());
#endif
         }
      }
   }
   for (PUPScreen* pScreen : GetChildren())
      pScreen->Trigger(szTrigger);
}

string PUPScreen::ToString() const
{
   return "mode=" + std::to_string(m_mode) +
      ", screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", backgroundPlaylist=" + m_backgroundPlaylist +
      ", backgroundFilename=" + m_backgroundFilename +
      ", transparent=" + (m_transparent ? "true" : "false") +
      ", volume=" + std::to_string(m_volume) +
      ", m_customPos={" + (m_pCustomPos ? m_pCustomPos->ToString() : "") + "}";
}