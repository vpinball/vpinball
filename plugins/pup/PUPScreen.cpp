#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"
#include "PUPManager.h"
#include "PUPPlaylist.h"
#include "PUPLabel.h"
#include "PUPMediaManager.h"

namespace PUP {

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

PUPScreen::PUPScreen(PUPManager* manager, PUPScreen::Mode mode, int screenNum, const string& szScreenDes, bool transparent, float volume, std::unique_ptr<PUPCustomPos> pCustomPos, const std::vector<PUPPlaylist*>& playlists)
   : m_pManager(manager)
   , m_screenNum(screenNum)
   , m_mode(mode)
   , m_screenDes(szScreenDes)
   , m_transparent(transparent)
   , m_volume(volume)
   , m_pCustomPos(std::move(pCustomPos))
   , m_apiThread(std::this_thread::get_id())
{
   memset(&m_background, 0, sizeof(m_background));
   memset(&m_overlay, 0, sizeof(m_overlay));
   m_pMediaPlayerManager = std::make_unique<PUPMediaManager>(this);

   for (const PUPPlaylist* pPlaylist : playlists) {
      // make a copy of the playlist
      PUPPlaylist *pPlaylistCopy = new PUPPlaylist(*pPlaylist);
      AddPlaylist(pPlaylistCopy);
   }

   LoadTriggers();
}

PUPScreen::~PUPScreen()
{
   if (m_pageTimer)
      SDL_RemoveTimer(m_pageTimer);

   for (auto& [key, pPlaylist] : m_playlistMap)
      delete pPlaylist;

   for (auto& [key, pTriggers] : m_triggerMap) {
      for (PUPTrigger* pTrigger : pTriggers)
         delete pTrigger;
   }

   for (PUPLabel* pLabel : m_labels)
      delete pLabel;
}

std::unique_ptr<PUPScreen> PUPScreen::CreateFromCSV(PUPManager* manager, const string& line, const std::vector<PUPPlaylist*>& playlists)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8) {
      LOGE("Failed to parse screen line, expected 8 columns but got %d: %s", parts.size(), line.c_str());
      return nullptr;
   }

   PUPScreen::Mode mode;
   if (StrCompareNoCase(parts[5], "Show"s))
      mode = PUPScreen::Mode::Show;
   else if (StrCompareNoCase(parts[5], "ForceON"s))
      mode = PUPScreen::Mode::ForceOn;
   else if (StrCompareNoCase(parts[5], "ForcePoP"s))
      mode = PUPScreen::Mode::ForcePop;
   else if (StrCompareNoCase(parts[5], "ForceBack"s))
      mode = PUPScreen::Mode::ForceBack;
   else if (StrCompareNoCase(parts[5], "ForcePopBack"s))
      mode = PUPScreen::Mode::ForcePopBack;
   else if (StrCompareNoCase(parts[5], "MusicOnly"s))
      mode = PUPScreen::Mode::MusicOnly;
   else if (StrCompareNoCase(parts[5], "Off"s))
      mode = PUPScreen::Mode::Off;
   else {
      LOGE("Invalid screen mode: %s", parts[5].c_str());
      mode = PUPScreen::Mode::Off;
   }

   auto screen = std::make_unique<PUPScreen>(
      manager,
      mode,
      string_to_int(parts[0], 0), // screenNum
      parts[1], // screenDes
      parts[4] == "1", // transparent
      string_to_float(parts[6], 100.0f), // volume
      PUPCustomPos::CreateFromCSV(parts[7]), playlists);

   // Optional initial background playlist
   if (!parts[2].empty())
   {
      screen->Play(parts[2], parts[3], screen->GetVolume(), -1);
      screen->SetAsBackGround(true);
   }

   return screen;
}

std::unique_ptr<PUPScreen> PUPScreen::CreateDefault(PUPManager* manager, int screenNum, const std::vector<PUPPlaylist*>& playlists)
{
   switch(screenNum) {
   case PUP_SCREEN_TOPPER: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_TOPPER, "Topper"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_DMD: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_DMD, "DMD"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_BACKGLASS: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_BACKGLASS, "Backglass"s, false, 100.0f, nullptr, playlists);
   case PUP_SCREEN_PLAYFIELD: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, PUP_SCREEN_PLAYFIELD, "Playfield"s, false, 100.0f, nullptr, playlists);
   default: return std::make_unique<PUPScreen>(manager, PUPScreen::Mode::Show, screenNum, "Unknown"s, false, 100.0f, nullptr, playlists);
   }
}

void PUPScreen::LoadTriggers()
{
   assert(std::this_thread::get_id() == m_apiThread);
   string szPlaylistsPath = find_case_insensitive_file_path(m_pManager->GetPath() + "triggers.pup");
   std::ifstream triggersFile;
   triggersFile.open(szPlaylistsPath, std::ifstream::in);
   if (triggersFile.is_open())
   {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line))
      {
         if (++i == 1)
            continue;
         AddTrigger(PUPTrigger::CreateFromCSV(this, line));
      }
   }
}

void PUPScreen::SetVolume(float volume)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_volume = volume;
   m_pMediaPlayerManager->SetVolume(volume);
}

void PUPScreen::SetVolumeCurrent(float volume)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetVolume(volume);
}

void PUPScreen::AddChild(std::shared_ptr<PUPScreen> pScreen)
{
   assert(std::this_thread::get_id() == m_apiThread);
   switch (pScreen->GetMode()) {
      case PUPScreen::Mode::ForceOn:
      case PUPScreen::Mode::ForcePop:
         m_topChildren.push_back(pScreen);
         break;
      case PUPScreen::Mode::ForceBack:
      case PUPScreen::Mode::ForcePopBack:
         m_backChildren.push_back(pScreen);
         break;
      default:
          m_defaultChildren.push_back(pScreen);
   }
   pScreen->m_pParent = this;
}

void PUPScreen::SendToFront()
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_pParent) {
      if (m_mode == PUPScreen::Mode::ForceOn || m_mode == PUPScreen::Mode::ForcePop) {
         auto it = std::ranges::find_if(m_pParent->m_topChildren, [this](std::shared_ptr<PUPScreen> a) { return a.get() == this; });
         if (it != m_pParent->m_topChildren.end())
            std::rotate(it, it + 1, m_pParent->m_topChildren.end());
      }
      else if (m_mode == PUPScreen::Mode::ForceBack || m_mode == PUPScreen::Mode::ForcePopBack) {
         auto it = std::ranges::find_if(m_pParent->m_backChildren, [this](std::shared_ptr<PUPScreen> a) { return a.get() == this; });
         if (it != m_pParent->m_backChildren.end())
            std::rotate(it, it + 1, m_pParent->m_backChildren.end());
      }
   }
}

void PUPScreen::AddPlaylist(PUPPlaylist* pPlaylist)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!pPlaylist)
      return;

   m_playlistMap[lowerCase(pPlaylist->GetFolder())] = pPlaylist;
}

PUPPlaylist* PUPScreen::GetPlaylist(const string& szFolder)
{
   assert(std::this_thread::get_id() == m_apiThread);
   ankerl::unordered_dense::map<string, PUPPlaylist*>::const_iterator it = m_playlistMap.find(lowerCase(szFolder));
   return it != m_playlistMap.end() ? it->second : nullptr;
}

void PUPScreen::AddTrigger(PUPTrigger* pTrigger)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!pTrigger)
      return;

   m_triggerMap[pTrigger->GetTrigger()].push_back(pTrigger);
}

vector<PUPTrigger*>* PUPScreen::GetTriggers(const string& szTrigger)
{
   assert(std::this_thread::get_id() == m_apiThread);
   ankerl::unordered_dense::map<string, vector<PUPTrigger*>>::iterator it = m_triggerMap.find(szTrigger);
   return it != m_triggerMap.end() ? &it->second : nullptr;
}

void PUPScreen::AddLabel(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (GetLabel(pLabel->GetName())) {
      LOGE("Duplicate label: screen={%s}, label=%s", ToString(false).c_str(), pLabel->ToString().c_str());
      delete pLabel;
      return;
   }

   pLabel->SetScreen(this);
   m_labelMap[lowerCase(pLabel->GetName())] = pLabel;
   m_labels.push_back(pLabel);
}

PUPLabel* PUPScreen::GetLabel(const string& szLabelName)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = m_labelMap.find(lowerCase(szLabelName));
   return it != m_labelMap.end() ? it->second : nullptr;
}

void PUPScreen::SendLabelToBack(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(m_labels.begin(), it, it + 1);
}

void PUPScreen::SendLabelToFront(PUPLabel* pLabel)
{
   assert(std::this_thread::get_id() == m_apiThread);
   auto it = std::find(m_labels.begin(), m_labels.end(), pLabel);
   if (it != m_labels.end())
      std::rotate(it, it + 1, m_labels.end());
}

void PUPScreen::SetPage(int pagenum, int seconds)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_pageTimer)
      SDL_RemoveTimer(m_pageTimer);
   m_pageTimer = 0;
   m_pagenum = pagenum;

   if (seconds == 0)
      m_defaultPagenum = pagenum;
   else
      m_pageTimer = SDL_AddTimer(seconds * 1000, PageTimerElapsed, this);
}

uint32_t PUPScreen::PageTimerElapsed(void* param, SDL_TimerID timerID, uint32_t interval)
{
   PUPScreen* me = static_cast<PUPScreen*>(param);
   assert(std::this_thread::get_id() == me->m_apiThread);
   SDL_RemoveTimer(me->m_pageTimer);
   me->m_pageTimer = 0;
   me->m_pagenum = me->m_defaultPagenum;
   return interval;
}

void PUPScreen::SetSize(int w, int h)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_rect = m_pCustomPos ? m_pCustomPos->ScaledRect(w, h) : SDL_Rect { 0, 0, w, h };
   m_pMediaPlayerManager->SetBounds(m_rect);

   for (auto pChildren : { &m_defaultChildren, &m_backChildren, &m_topChildren }) {
      for (auto pScreen : *pChildren)
          pScreen->SetSize(w, h);
   }
}

void PUPScreen::SetCustomPos(const string& szCustomPos)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pCustomPos = PUPCustomPos::CreateFromCSV(szCustomPos);
}

void PUPScreen::Play(const string& szPlaylist, const string& szPlayFile, float volume, int priority)
{
   assert(std::this_thread::get_id() == m_apiThread);
   PUPPlaylist* const pPlaylist = GetPlaylist(szPlaylist);
   if (!pPlaylist)
   {
      LOGE("Playlist not found: screen={%s}, playlist=%s", ToString(false).c_str(), szPlaylist.c_str());
      return;
   }
   Play(pPlaylist, szPlayFile, volume, priority, false, 0);
}

void PUPScreen::Play(PUPPlaylist* pPlaylist, const string& szPlayFile, float volume, int priority, bool skipSamePriority, int length)
{
   assert(std::this_thread::get_id() == m_apiThread);
   LOGD("play, screen={%s}, playlist={%s}, playFile=%s, volume=%.f, priority=%d", ToString(false).c_str(), pPlaylist->ToString().c_str(), szPlayFile.c_str(), volume, priority);
   //StopMedia(); // Does it stop the played media on all request like overlays or alphas ? I don't think so but unsure
   switch (pPlaylist->GetFunction())
   {
   case PUPPlaylist::Function::Default:
      // In original PupPlayer, Pop screens are recreated when play is call and therefore placed at the top of there z order stack (normal or topmost)
      if (m_pParent && IsPop())
      {
         vector<std::shared_ptr<PUPScreen>>& childrens = m_mode == PUPScreen::Mode::ForcePop ? m_pParent->m_topChildren : m_pParent->m_backChildren;
         auto it = std::ranges::find_if(childrens, [this](std::shared_ptr<PUPScreen> s) { return s.get() == this; });
         if (it != childrens.end())
         {
            auto item = std::move(*it);
            childrens.erase(it);
            childrens.push_back(item);
         }
      }
      m_pMediaPlayerManager->Play(pPlaylist, szPlayFile, m_pParent ? (volume / 100.0f) * m_pParent->GetVolume() : volume, priority, skipSamePriority, length);
      break;

   case PUPPlaylist::Function::Frames:
      m_background.Load(pPlaylist->GetPlayFilePath(szPlayFile));
      break;

   case PUPPlaylist::Function::Overlays:
   case PUPPlaylist::Function::Alphas:
      m_overlay.Load(pPlaylist->GetPlayFilePath(szPlayFile));
      break;

   case PUPPlaylist::Function::Shapes:
      SetMask(pPlaylist->GetPlayFilePath(szPlayFile));
      break;

   default:
      LOGE("Invalid playlist function: %s", PUPPlaylist::ToString(pPlaylist->GetFunction()).c_str());
      break;
   }
}

void PUPScreen::SetMask(const string& path)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetMask(path);
}

void PUPScreen::Stop()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop();
}

void PUPScreen::Stop(int priority)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop(priority);
}

void PUPScreen::Stop(PUPPlaylist* pPlaylist, const std::string& szPlayFile)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Stop(pPlaylist, szPlayFile);
}

void PUPScreen::Pause()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Pause();
}

void PUPScreen::Resume()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->Resume();
}

void PUPScreen::SetLoop(int state)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetLoop(state != 0);
}

void PUPScreen::SetLength(int length)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetMaxLength(length);
}

void PUPScreen::SetAsBackGround(int mode)
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_pMediaPlayerManager->SetAsBackGround(mode != 0);
}

bool PUPScreen::IsPlaying() {
   assert(std::this_thread::get_id() == m_apiThread);
   return m_pMediaPlayerManager->IsPlaying();
}

void PUPScreen::Render(VPXRenderContext2D* const ctx) {
   assert(std::this_thread::get_id() == m_apiThread);
   for (auto pScreen : m_backChildren)
      pScreen->Render(ctx);

   for (auto pScreen : m_defaultChildren)
      pScreen->Render(ctx);

   m_background.Render(ctx, m_rect);
   m_pMediaPlayerManager->Render(ctx);
   // FIXME port SDL_SetRenderClipRect(m_pRenderer, &m_rect);
   for (PUPLabel* pLabel : m_labels)
      pLabel->Render(ctx, m_rect, m_pagenum);
   // FIXME port SDL_SetRenderClipRect(m_pRenderer, NULL);
   m_overlay.Render(ctx, m_rect);

   for (auto pScreen : m_topChildren)
      pScreen->Render(ctx);
}

string PUPScreen::ToString(bool full) const
{
   if (full) {
      return "mode=" + ToString(m_mode) +
         ", screenNum=" + std::to_string(m_screenNum) +
         ", screenDes=" + m_screenDes +
         ", transparent=" + (m_transparent ? "true" : "false") +
         ", volume=" + std::to_string(m_volume) +
         ", m_customPos={" + (m_pCustomPos ? m_pCustomPos->ToString() : ""s) + '}';
   }

   return "screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", mode=" + ToString(m_mode);
}

const string& PUPScreen::ToString(Mode value)
{
   static const string modeStrings[] = { "Off"s, "Show"s, "ForceOn"s, "ForcePop"s, "ForceBack"s, "ForcePopBack"s, "MusicOnly"s };
   static const string error = "Unknown"s;
   if ((int)value < 0 || (size_t)value >= std::size(modeStrings))
      return error;
   return modeStrings[(int)value];
}

}
