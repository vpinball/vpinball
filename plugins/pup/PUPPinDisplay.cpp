// license:GPLv3+

#include "PUPPinDisplay.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPLabel.h"

#include <fstream>
#include <sstream>
#include <charconv>
#include <cassert>


namespace PUP {

static const string emptystring;

PUPPinDisplay::PUPPinDisplay(PUPManager& manager)
   : m_pupManager(manager)
{
}

PUPPinDisplay::~PUPPinDisplay()
{
   // This object is only a script proxy: tables may create and release several of
   // them for the same game, so do not unload the manager here. Teardown happens
   // on game end (see PUPPlugin OnGameEnd) or through an explicit CloseApp call.
}

void PUPPinDisplay::Init(int screenNum, const string& romName)
{
   m_pupManager.SetGameDir(romName);
   m_pupManager.AddScreen(screenNum);
}

void PUPPinDisplay::playlistadd(int screenNum, const string& folder, int sort, int restSeconds)
{
   //playlistadd  (<display#>  <folder name>,  <sort/shuffle>, <restSeconds>);
   // pretty self explaining...  the PinUpPlayer(PUP) will manage playlists and shuffle them.  RestSeconds is what I use to 'limit' the amount of videos being played.  So lets say you want a ramp video to play, but since I set it to 60... you can hit that ramp multiple times but no videos will play until the restSeconds time has passed since last video played.
   // <Sort 1=shuffle, 0 = alpha>  So if you want to progress game videos you could name the first one 001.mp4, 002.mp4...099.mp4  That way alpha sort will play in your order.
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;

   if (pScreen->GetPlaylist(folder)) {
      LOGE(std::format("Playlist already exists: screenNum={}, folder={}", screenNum, folder));
      return;
   }

   pScreen->AddPlaylist(new PUPPlaylist(&m_pupManager, PluginStrings::PathFromUTF8(folder), ""s, sort, restSeconds, 100, 1));
}

void PUPPinDisplay::playlistplay(int screenNum, const string& playlist)
{
   // Play next file from playlist (shuffled or in order) with default volume and no priority
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
   {
      PUPPlaylist* pPlaylist = pScreen->GetPlaylist(playlist);
      if (pPlaylist)
         pScreen->Play(pPlaylist, std::filesystem::path(), static_cast<float>(pScreen->GetDefaultVolume()), 0, PlayAction::Normal, 0);
      else
         LOGE(std::format("Playlist not found: screenNum={}, playlist={}", screenNum, playlist));
   }
}

void PUPPinDisplay::playlistplayex(int screenNum, const string& playlist, const string& playfilename, int volume, int priority)
{
   // priority(0=none, 1..9) will override the restSeconds (see playlistadd)
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->Play(playlist, PluginStrings::PathFromUTF8(playfilename), static_cast<float>(volume), priority);
}

void PUPPinDisplay::play(int screenNum, const string& playlist, const string& playfilename)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
   {
      PUPPlaylist* pPlaylist = pScreen->GetPlaylist(playlist);
      if (!pPlaylist)
      {
         LOGE(std::format("Playlist not found: screen={{{}}}, playlist={}", pScreen->ToString(false), playlist));
         return;
      }
      pScreen->Play(playlist, PluginStrings::PathFromUTF8(playfilename), static_cast<float>(pScreen->GetDefaultVolume()), 0);
   }
}

void PUPPinDisplay::setWidth(int screenNum, int width)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, width={}", screenNum, width));
}

void PUPPinDisplay::setHeight(int screenNum, int Height)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, height={}", screenNum, Height));
}

void PUPPinDisplay::setPosX(int screenNum, int Posx)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, Posx={}", screenNum, Posx));
}

void PUPPinDisplay::setPosY(int screenNum, int PosY)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, PosY={}", screenNum, PosY));
}

void PUPPinDisplay::setAspect(int screenNum, int aspectWide, int aspectHigh)
{
   //**** Set 0,0 to fittoscreen.
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, aspectWide={}, aspectHigh={}", screenNum, aspectWide, aspectHigh));
}

void PUPPinDisplay::setVolume(int screenNum, int vol)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetDefaultVolume(vol);
}

void PUPPinDisplay::setVolumeCurrent(int screenNum, int vol)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetVolumeCurrent(vol);
}

void PUPPinDisplay::playpause(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->Pause();
}

void PUPPinDisplay::playresume(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->Resume();
}

void PUPPinDisplay::playstop(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->Stop();
}

void PUPPinDisplay::CloseApp()
{
   m_pupManager.Unload();
}

bool PUPPinDisplay::GetisPlaying(int screenNum) const
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   return pScreen ? pScreen->IsMainPlaying() : false;
}

void PUPPinDisplay::SetisPlaying(int screenNum, bool value)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, value={}", screenNum, value));
}

void PUPPinDisplay::SetLength(int screenNum, int StopSecs)
{
   //after you play a file call setlength if you want it to stop.  so setlength(5) will stop video at 5 seconds mark.
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetLength(StopSecs);
}

void PUPPinDisplay::SetLoop(int screenNum, int LoopState)
{
   // if you set LoopState=1,  it will loop the currently playing file.  0=cancel loop
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetLoop(LoopState);
}

void PUPPinDisplay::SetBackGround(int screenNum, int mode)
{
   // if you set Mode=1, it will set current playing file as background (loop it always).  Mode=0 to cancel background.
   LOGD(std::format("SetBackGround called: screenNum={}, mode={}", screenNum, mode));
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetAsBackGround(mode);
}

void PUPPinDisplay::BlockPlay(int screenNum, int mode)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, mode={}", screenNum, mode));
}

void PUPPinDisplay::SetScreen(int screenNum)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}", screenNum));
}

void PUPPinDisplay::SetScreenEx(int screenNum, int xpos, int ypos, int swidth, int sheight, int popup) 
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;
   switch (popup) {
   case 0: pScreen->SetMode(PUPScreen::Mode::Show); break; // See Stranger Things
   case 1: pScreen->SetMode(PUPScreen::Mode::ForcePop); break;
   case 2: pScreen->SetMode(PUPScreen::Mode::MusicOnly); break; // See Stranger Things
   default: pScreen->SetMode(PUPScreen::Mode::MusicOnly); break;
   }
   if (swidth && sheight) {
      // If not 0, this is used to define a custom screen size from script. The only known use case is when using PUPDMDControl to render to a real DMD
      LOGE(std::format("Not fully implemented: screenNum={}, xpos={}, ypos={}, swidth={}, sheight={}, popup={}", screenNum, xpos, ypos, swidth, sheight, popup));
   }
}

int PUPPinDisplay::GetSN() const
{
   NOT_IMPLEMENTED("Not implemented"s);
   return 0;
}

void PUPPinDisplay::SetSN(int Value)
{
   NOT_IMPLEMENTED("Not implemented: value=" + std::to_string(Value));
}

void PUPPinDisplay::B2SData(const string& tIndex, int Value)
{
   const string tmp = tIndex.substr(1);
   int result;
   auto ec = std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), result).ec;
   assert(ec == std::errc{});

   m_pupManager.QueueDOFEvent(tIndex[0], result, Value);
}

const string& PUPPinDisplay::GetB2SFilter() const
{
   NOT_IMPLEMENTED("Not implemented"s);
   return emptystring;
}

void PUPPinDisplay::SetB2SFilter(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=" + value);
}

void PUPPinDisplay::Show(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetHidden(false);
}

void PUPPinDisplay::Hide(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetHidden(true);
}

void PUPPinDisplay::B2SInit(const string& tName, const string& romName)
{
   m_pupManager.LoadConfig(romName);
}

void PUPPinDisplay::CustomFunction(int sn, const string& szMsg)
{
   m_pupManager.CustomFunction(sn, szMsg);
}

void PUPPinDisplay::SendMSG(const string& szMsg)
{
   m_pupManager.SendMsg(szMsg);
}

/*
   screenNum - in standard we'd set this to pDMD ( or 1)
   LabelName - your name of the label. keep it short no spaces (like 8 chars) although you can call it
               anything really. When setting the label you will use this labelname to access the label
   FontName  - Windows font name, this must be exact match of OS front name. if you are using custom
               TTF fonts then double check the name of font names
   Size      - Height as a percent of display height. 20=20% of screen height
   Color     - integer value of windows color
   Angle     - not used yet (future)
   xAlign    - 0 = horizontal left align, 1 = center horizontal, 2 = right horizontal
   yAlign    - 0 = top, 1 = center, 2 = bottom vertical alignment
   xMargin   - This should be 0, but if you want to 'force' a position you can set this. it is a % of horizontal
               width. 20=20% of screen width
   yMargin   - same as xMargin
   PageNum   - IMPORTANT... this will assign this label to this 'page' or group
   Visible   - initial state of label. visible = 1 show, 0 = off
*/
void PUPPinDisplay::LabelNew(int screenNum, const string& LabelName, const string& FontName, int Size, int Color, int Angle, int xAlign, int yAlign, int xMargin, int yMargin, int PageNum, bool Visible)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;

   pScreen->LabelInit();

   pScreen->AddLabel(new PUPLabel(&m_pupManager, LabelName, FontName, static_cast<float>(Size), Color,
      static_cast<float>(Angle) / 10.0f, (PUP_LABEL_XALIGN)xAlign, (PUP_LABEL_YALIGN)yAlign,
      static_cast<float>(xMargin), static_cast<float>(yMargin), PageNum, Visible));

   return;
}

/*
   screenNum - same as when adding new label.
   LabelName - same as labelname when you added new label.
   Caption -  value to assign label. Note you can add a ~ to insert a line break....see notes on that.
   Visible -  set visible state of label.
   Special -  this is a special crafter JSON string that can have unlimited values set. we'll have to see
              examples and things like animations need many different variables and settings. This allows use to
              future proof the text driver using a JSON string. Note: to make it easier to read in VBS its a json where
              you normally see "field", is 'field'.... we replace all ' with " before processing json.
              ----new in v1.3 Special can adjust many label properties AFTER created already.
              special allows lots of modification of existing label. you should not change this too often because of
              performance penalty like changing a fontname etc. for other items like position, there won't be a
              performance issue.
*/
void PUPPinDisplay::LabelSet(int screenNum, const string& LabelName, const string& Caption, bool Visible, const string& Special)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;

   if (pScreen->m_hideDuringPlay)
      return;

   PUPLabel* pLabel = pScreen->GetLabel(LabelName);
   if (!pLabel) {
      if (m_warnedLabels[screenNum].find(LabelName) == m_warnedLabels[screenNum].end())
      {
         LOGE(std::format("Invalid label: screen={{{}}}, labelName={}", pScreen->ToString(false), LabelName));
         m_warnedLabels[screenNum].insert(LabelName);
      }
      return;
   }

   pLabel->SetCaption(Caption);
   // PUP sets visibility for a normal caption only while that label's page is
   // current. The "`u`" caption is the exception: it can update visibility of any label.
   const bool updateVisibility = (Caption == "`u`"s) || (pLabel->GetPagenum() == pScreen->GetPage());
   if (updateVisibility && !pLabel->IsAnimating())
      pLabel->SetVisible(Visible);
   pLabel->SetSpecial(Special);

   // See pDMDLabelDispose - remove the label after applying the update
   if (pLabel->GetDoDispose())
      pScreen->RemoveLabel(pLabel);
}

void PUPPinDisplay::LabelSetEx()
{
   NOT_IMPLEMENTED("Not implemented"s);
}

/*
   screenNum
   PageNum - page to view
   Seconds - seconds to show page...this is useful for 'splash' pages. If your page is a splash page then
             you would set this to 3 for 3 second splash. It will auto-return to the default page. The
             default page is the one that was last set with seconds=0; So we set seconds to 0 to tell driver 
             that this is the current active page.
   Special - future.
*/

void PUPPinDisplay::LabelShowPage(int screenNum, int PageNum, int Seconds, const string& Special)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;
   // Special values "hidehudplay" and "returnplay" are PuPDMD framework behaviors exposed via
   // these helper subs (appear in Diehard/Matrix):
   //   Sub PDMDSplashPagePlaying    'will hide HUD and show labelpage while current media is
   //                                 playing and then autoreturn
   //   Sub PDMDSplashPagePlayingHUD 'will show labelpage and auto return to def after current
   //                                 video stopped
   pScreen->SetPage(PageNum, Seconds >= 501 ? Seconds : Seconds * 1000, Special);
}

void PUPPinDisplay::LabelInit(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->LabelInit();
}

const string& PUPPinDisplay::GetGetGame() const
{
   NOT_IMPLEMENTED("Not implemented"s);
   return emptystring;
}

void PUPPinDisplay::SetGetGame(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=" + value);
}

string PUPPinDisplay::GetGetRoot() const
{
   // The return path is either the default one when not playing or the one actually being played (which may be a per table or a global folder)
   if (m_pupManager.GetPath().empty())
      return PluginStrings::PathToUTF8(m_pupManager.GetRootPath());
   else
      return PluginStrings::PathToUTF8(m_pupManager.GetPath().parent_path() / ""sv);
}

void PUPPinDisplay::SetGetRoot(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=" + value);
}

void PUPPinDisplay::SoundAdd(const string& sname, const string& fname, int svol, double sX, double sy, const string& SP)
{
   // Ignored by PinUP Player: sounds come from PUPSOUNDS\*.wav through PuPSound
   LOGD("SoundAdd ignored: sname=" + sname);
}

void PUPPinDisplay::SoundPlay(const string& sname)
{
   // Ignored by PinUP Player: sounds come from PUPSOUNDS\*.wav through PuPSound
   LOGD("SoundPlay ignored: sname=" + sname);
}

void PUPPinDisplay::PuPSound(const string& sname, int sX, int sy, int sz, int vol, const string& SP)
{
   m_pupManager.PlaySound(sname, sX, sy, sz, vol, SP);
}

void PUPPinDisplay::InitPuPMenu(int Param1)
{
   NOT_IMPLEMENTED("Not implemented: param1=" + std::to_string(Param1));
}

const string& PUPPinDisplay::GetB2SDisplays() const
{
   NOT_IMPLEMENTED("Not implemented"s);
   return emptystring;
}

int PUPPinDisplay::GameUpdate(const string& GameTitle, int Func, int FuncData, const string& Extra) const
{
   // Used in PizzaTime to communicate with the mini game app:
   // - forward key up / key down
   // - check for mini game state
   // - readback the value of the score
   NOT_IMPLEMENTED(std::format("Not implemented: gameTitle={}, func={}, funcData={}, extra={}", GameTitle, Func, FuncData, Extra));
   return 0;
}

/* STDMETHODIMP PUPPinDisplay::GrabDC(int pWidth, int pHeight, BSTR wintitle, VARIANT* pixels)
{
   LOGE("Not implemented: pWidth=%d, pHeight=%d, wintitle=%s", pWidth, pHeight, MakeString(wintitle).c_str());

   return;
}*/

string PUPPinDisplay::GetVersion() const
{
   constexpr int nVersionNo0 = 2;
   constexpr int nVersionNo1 = 0;
   constexpr int nVersionNo2 = 99;
   static const string version = std::to_string(nVersionNo0) + '.' + std::to_string(nVersionNo1) + '.' + std::to_string(nVersionNo2);
   return version;
}

/* STDMETHODIMP PUPPinDisplay::GrabDC2(int pWidth, int pHeight, BSTR wintitle, SAFEARRAY** pixels)
{
   LOGE("Not implemented: pWidth=%d, pHeight=%d, wintitle=%s", pWidth, pHeight, MakeString(wintitle).c_str());

   return;
}*/

void PUPPinDisplay::playevent(int screenNum, const string& playlist, const string& playfilename, int volume, int priority, int playtype, int Seconds, const string& Special)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;

   if (playtype < static_cast<int>(PlayAction::Normal) || playtype == 10 || playtype > static_cast<int>(PlayAction::QueueAlways)) {
      NOT_IMPLEMENTED("Not implemented: playevent playtype=" + std::to_string(playtype));
      return;
   }
   const PlayAction action = static_cast<PlayAction>(playtype);

   switch (action) {
   case PlayAction::StopPlayer:
      pScreen->Stop(priority);
      return;
   case PlayAction::StopFile:
   {
      PUPPlaylist* pPlaylist = pScreen->GetPlaylist(playlist);
      if (pPlaylist)
         pScreen->Stop(pPlaylist, PluginStrings::PathFromUTF8(playfilename));
      return;
   }
   case PlayAction::PlaySSF:
      m_pupManager.PlaySound(playfilename, 0, 0, 0, volume < 0 ? 100 : volume, ""s);
      return;
   case PlayAction::CustomFunction:
      NOT_IMPLEMENTED("Not implemented: playevent playtype=" + std::to_string(playtype));
      return;
   default:
      break;
   }

   pScreen->Play(playlist, PluginStrings::PathFromUTF8(playfilename), static_cast<float>(volume), priority, action, Seconds);
}

void PUPPinDisplay::SetPosVideo(int screenNum, int StartPos, int EndPos, int mode, const string& Special)
{
   NOT_IMPLEMENTED(std::format("Not implemented: screenNum={}, startPos={}, endPos={}, mode={}, special={}", screenNum, StartPos, EndPos, mode, Special));
}

void PUPPinDisplay::PuPClose()
{
   m_pupManager.Unload();
}

}
