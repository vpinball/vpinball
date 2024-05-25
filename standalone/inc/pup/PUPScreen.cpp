#include "core/stdafx.h"

#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"
#include "PUPManager.h"
#include "PUPPlaylist.h"
#include "PUPLabel.h"
#include "PUPMediaPlayer.h"

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

PUPScreen::PUPScreen()
{
   m_pManager = PUPManager::GetInstance();

   m_pCustomPos = nullptr;
   memset(&m_background, 0, sizeof(m_background));
   memset(&m_overlay, 0, sizeof(m_overlay));
   m_pMediaPlayer = new PUPMediaPlayer();
   m_labelInit = false;
   m_pagenum = 0;
   m_defaultPagenum = 0;
   m_pPageTimer = new VP::Timer();
   m_pPageTimer->SetElapsedListener(std::bind(&PUPScreen::PageTimerElapsed, this, std::placeholders::_1));

   m_isRunning = true;
   m_playThread = std::thread(&PUPScreen::ProcessQueue, this);
}

PUPScreen::~PUPScreen()
{
   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_isRunning = false;
   }
   m_queueCondVar.notify_all();
   m_playThread.join();

   delete m_pCustomPos;
   FreeRenderable(&m_background);
   FreeRenderable(&m_overlay);
   delete m_pMediaPlayer;
   delete m_pPageTimer;

   for (PUPLabel* pLabel : m_labels)
      delete pLabel;

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren })
      pChildren->clear();
}

PUPScreen* PUPScreen::CreateFromCSV(const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8)
      return nullptr;

   PUPScreen* pScreen = new PUPScreen();

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
   else if (string_compare_case_insensitive(mode, "MusicOnly"))
      pScreen->m_mode = PUP_SCREEN_MODE_MUSIC_ONLY;
   else
      pScreen->m_mode = PUP_SCREEN_MODE_OFF;

   pScreen->m_screenNum = string_to_int(parts[0], 0);
   pScreen->m_screenDes = parts[1];
   pScreen->m_backgroundPlaylist = parts[2];
   pScreen->m_backgroundFilename = parts[3];
   pScreen->m_transparent = (parts[4] == "1");
   pScreen->m_volume = string_to_int(parts[6], 0);
   pScreen->m_pCustomPos = PUPCustomPos::CreateFromCSV(parts[7]);

   return pScreen;
}

void PUPScreen::AddChild(PUPScreen* pScreen)
{
   switch (pScreen->GetMode()) {
      case PUP_SCREEN_MODE_FORCE_ON:
      case PUP_SCREEN_MODE_FORCE_POP:
         m_topChildren.push_back(pScreen);
         break;
      case PUP_SCREEN_MODE_FORCE_BACK:
      case PUP_SCREEN_MODE_FORCE_POP_BACK:
         m_backChildren.push_back(pScreen);
         break;
      default:
          m_defaultChildren.push_back(pScreen);
   }
   pScreen->SetParent(this);
}

void PUPScreen::SendToFront()
{
   if (m_pParent) {
      if (m_mode == PUP_SCREEN_MODE_FORCE_ON || m_mode == PUP_SCREEN_MODE_FORCE_POP) {
         auto it = std::find(m_pParent->m_topChildren.begin(), m_pParent->m_topChildren.end(), this);
         if (it != m_pParent->m_topChildren.end())
            std::rotate(it, it + 1, m_pParent->m_topChildren.end());
      }
      else if (m_mode == PUP_SCREEN_MODE_FORCE_BACK || m_mode == PUP_SCREEN_MODE_FORCE_POP_BACK) {
         auto it = std::find(m_pParent->m_backChildren.begin(), m_pParent->m_backChildren.end(), this);
         if (it != m_pParent->m_backChildren.end())
            std::rotate(it, it + 1, m_pParent->m_backChildren.end());
      }
   }
}

void PUPScreen::AddLabel(PUPLabel* pLabel)
{
   if (GetLabel(pLabel->GetName())) {
      PLOGW.printf("Duplicate label: screen={%s}, label=%s", ToString(false).c_str(), pLabel->ToString().c_str());
      delete pLabel;
      return;
   }

   pLabel->SetScreen(this);
   m_labelMap[string_to_lower(pLabel->GetName())] = pLabel;
   m_labels.push_back(pLabel);
}

PUPLabel* PUPScreen::GetLabel(const string& szLabelName)
{
   auto it = m_labelMap.find(string_to_lower(szLabelName));
   return it != m_labelMap.end() ? it->second : nullptr;
}

void PUPScreen::SendLabelToBack(PUPLabel* pLabel)
{
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(m_labels.begin(), it, it + 1);
}

void PUPScreen::SendLabelToFront(PUPLabel* pLabel)
{
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(it, it + 1, m_labels.end());
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

void PUPScreen::PageTimerElapsed(VP::Timer* pTimer)
{
   m_pPageTimer->Stop();
   m_pagenum = m_defaultPagenum;
}

void PUPScreen::SetSize(int w, int h)
{
   if (m_pCustomPos)
      m_rect = m_pCustomPos->ScaledRect(w, h);
   else
      m_rect = { 0, 0, w, h };

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (PUPScreen* pScreen : *pChildren)
          pScreen->SetSize(w, h);
   }
}

void PUPScreen::SetBackground(const string& file)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);
   LoadRenderable(&m_background, file);
}

void PUPScreen::SetMedia(const std::string& file, int volume)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);

   string ext = extension_from_path(file);
   if (ext == "png" || ext == "jpg" || ext == "bmp") {
       LoadRenderable(&m_background, file);
   }
   else {
      if (m_background.pSurface) {
         SDL_FreeSurface(m_background.pSurface);
         m_background.pSurface = NULL;
         m_background.dirty = true;
      }
      m_pMediaPlayer->Play(file, volume);
   }
}

void PUPScreen::SetOverlay(const string& file)
{
   std::lock_guard<std::mutex> lock(m_renderMutex);
   LoadRenderable(&m_overlay, file);
}

void PUPScreen::PlayEx(const string& szPlaylist, const string& szPlayfilename, int volume, int priority)
{
   PUPPlaylist* pPlaylist = m_pManager->GetPlaylist(szPlaylist);
   if (pPlaylist) {
      {
         std::lock_guard<std::mutex> lock(m_queueMutex);
         m_requestQueue.push({ PUP_TRIGGER_PLAY_ACTION_NORMAL, pPlaylist, szPlayfilename, volume, priority});
      }
      m_queueCondVar.notify_one();
   }
}

void PUPScreen::Stop()
{
   PLOGI.printf("Stop: screen={%s}", ToString(false).c_str());

   //m_pMediaPlayer->Stop();
}

void PUPScreen::SetLoop(int state)
{
   PLOGI.printf("SetLoop: screen={%s}, state=%d", ToString(false).c_str(), state);

   //m_pMediaPlayer->SetLoop(state);
}

void PUPScreen::SetBackGround(int mode)
{
   PLOGW.printf("Not implemented: screen={%s}, mode=%d", ToString(false).c_str(), mode);
}

void PUPScreen::ProcessTrigger(PUPTrigger* pTrigger)
{
   PLOGW.printf("Trigger: {%s}", pTrigger->ToString().c_str());

   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_requestQueue.push({ pTrigger->GetPlayAction(), pTrigger->GetPlaylist(), pTrigger->GetPlayFile(), pTrigger->GetVolume(), pTrigger->GetPriority() });
   }
   m_queueCondVar.notify_one();
}

void PUPScreen::Init(SDL_Renderer* pRenderer)
{
   m_pRenderer = pRenderer;

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (PUPScreen* pScreen : *pChildren)
         pScreen->Init(pRenderer);
   }

   PlayEx(m_backgroundPlaylist, m_backgroundFilename, m_volume, 0);
}

void PUPScreen::ProcessQueue()
{
   while (true) {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_queueCondVar.wait(lock, [this] { return !m_requestQueue.empty() || !m_isRunning; });

      if (!m_isRunning) {
         while (!m_requestQueue.empty())
            m_requestQueue.pop();
         break;
      }

      PUPScreenRequest request = m_requestQueue.front();
      m_requestQueue.pop();
      lock.unlock();

      switch (request.action) {
         case PUP_TRIGGER_PLAY_ACTION_NORMAL:
         case PUP_TRIGGER_PLAY_ACTION_SET_BG:
         case PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY:
         case PUP_TRIGGER_PLAY_ACTION_LOOP:
         {
            if (!request.pPlaylist)
               continue;

            int volume;
            string szPath;

            if (!request.szPlayFile.empty()) {
                szPath = request.pPlaylist->GetPlayFile(request.szPlayFile);
                volume = request.volume;
            }
            else {
               szPath = request.pPlaylist->GetPlayFile("");
               volume = request.pPlaylist->GetVolume();
            }

            if (szPath.empty()) {
               PLOGW.printf("Unable to find path: screen={%s}, action={%s}, playlist={%s}, playFile=%s, volume=%d, priority=%d", ToString(false).c_str(), 
                  PUP_TRIGGER_PLAY_ACTION_TO_STRING(request.action), request.pPlaylist->ToString().c_str(), request.szPlayFile.c_str(), request.volume, request.priority);
               continue;
            }

            switch (request.pPlaylist->GetFunction()) {
               case PUP_PLAYLIST_FUNCTION_FRAMES:
                  SetBackground(szPath);
                  break;
               case PUP_PLAYLIST_FUNCTION_OVERLAYS:
                  SetOverlay(szPath);
                  break;
               default:
                  SetMedia(szPath, volume);
                  break;
            }
         }
         break;

         default:
            PLOGE.printf("Unhandled action: %s", PUP_TRIGGER_PLAY_ACTION_TO_STRING(request.action));
            break;
      }
   }
}

void PUPScreen::Render()
{
   std::lock_guard<std::mutex> lock(m_renderMutex);

   Render(&m_background);

   if (m_mode == PUP_SCREEN_MODE_FORCE_POP || m_mode == PUP_SCREEN_MODE_FORCE_POP_BACK) {
      if (m_pMediaPlayer->IsPlaying())
         m_pMediaPlayer->Render(m_pRenderer, &m_rect);
   }
   else
      m_pMediaPlayer->Render(m_pRenderer, &m_rect);

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (PUPScreen* pScreen : *pChildren)
         pScreen->Render();
   }

   Render(&m_overlay);

   SDL_RenderSetClipRect(m_pRenderer, &m_rect);

   for (PUPLabel* pLabel : m_labels)
      pLabel->Render(m_pRenderer, m_rect, m_pagenum);

   SDL_RenderSetClipRect(m_pRenderer, NULL);
}

void PUPScreen::LoadRenderable(PUPScreenRenderable* pRenderable, const string& szFile)
{
   if (pRenderable->pSurface)
      SDL_FreeSurface(pRenderable->pSurface);

   pRenderable->pSurface = IMG_Load(szFile.c_str());
   pRenderable->dirty = true;
}

void PUPScreen::Render(PUPScreenRenderable* pRenderable)
{
   if (pRenderable->dirty) {
      if (pRenderable->pTexture) {
         SDL_DestroyTexture(pRenderable->pTexture);
         pRenderable->pTexture = NULL;
      }
      if (pRenderable->pSurface) {
         pRenderable->pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pRenderable->pSurface);
         SDL_FreeSurface(pRenderable->pSurface);
         pRenderable->pSurface = NULL;
      }
      pRenderable->dirty = false;
   }

   if (pRenderable->pTexture)
      SDL_RenderCopy(m_pRenderer, pRenderable->pTexture, NULL, &m_rect);
}

void PUPScreen::FreeRenderable(PUPScreenRenderable* pRenderable)
{
   if (pRenderable->pSurface)
      SDL_FreeSurface(pRenderable->pSurface);

   if (pRenderable->pTexture)
      SDL_DestroyTexture(pRenderable->pTexture);
}

string PUPScreen::ToString(bool full) const
{
   if (full) {
      return "mode=" + string(PUP_SCREEN_MODE_TO_STRING(m_mode)) +
         ", screenNum=" + std::to_string(m_screenNum) +
         ", screenDes=" + m_screenDes +
         ", backgroundPlaylist=" + m_backgroundPlaylist +
         ", backgroundFilename=" + m_backgroundFilename +
         ", transparent=" + (m_transparent ? "true" : "false") +
         ", volume=" + std::to_string(m_volume) +
         ", m_customPos={" + (m_pCustomPos ? m_pCustomPos->ToString() : "") + '}';
   }

   return "screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", mode=" + string(PUP_SCREEN_MODE_TO_STRING(m_mode));
}
