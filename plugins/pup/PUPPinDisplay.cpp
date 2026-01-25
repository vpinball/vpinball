// license:GPLv3+

#include "PUPPinDisplay.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPLabel.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include <cassert>

#pragma warning(push)
#pragma warning(disable : 4267)
#include "RSJparser/RSJparser.tcc"
#pragma warning(pop)

namespace PUP {

static const string emptystring;

PUPPinDisplay::PUPPinDisplay(PUPManager& manager)
   : m_pupManager(manager)
{
}

PUPPinDisplay::~PUPPinDisplay()
{
   m_pupManager.Unload();
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
      LOGE("Playlist already exists: screenNum=%d, folder=%s", screenNum, folder.c_str());
      return;
   }

   pScreen->AddPlaylist(new PUPPlaylist(&m_pupManager, folder, ""s, sort, restSeconds, 100, 1));
}

void PUPPinDisplay::playlistplay(int screenNum, const string& playlist)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, playlist=%s", screenNum, playlist.c_str());
}

void PUPPinDisplay::playlistplayex(int screenNum, const string& playlist, const string& playfilename, int volume, int priority)
{
   // priority(0=none, 1..9) will override the restSeconds (see playlistadd)
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->Play(playlist, playfilename, static_cast<float>(volume), priority);
}

void PUPPinDisplay::play(int screenNum, const string& playlist, const string& playfilename)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
   {
      PUPPlaylist* pPlaylist = pScreen->GetPlaylist(playlist);
      if (!pPlaylist)
      {
         LOGE("Playlist not found: screen={%s}, playlist=%s", pScreen->ToString(false).c_str(), playlist.c_str());
         return;
      }
      pScreen->Play(playlist, playfilename, pPlaylist->GetVolume(), 0);
   }
}

void PUPPinDisplay::setWidth(int screenNum, int width)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, width=%d", screenNum, width);
}

void PUPPinDisplay::setHeight(int screenNum, int Height)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, height=%d", screenNum, Height);
}

void PUPPinDisplay::setPosX(int screenNum, int Posx)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, Posx=%d", screenNum, Posx);
}

void PUPPinDisplay::setPosY(int screenNum, int PosY)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, PosY=%d", screenNum, PosY);
}

void PUPPinDisplay::setAspect(int screenNum, int aspectWide, int aspectHigh)
{
   //**** Set 0,0 to fittoscreen.
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, aspectWide=%d, aspectHigh=%d", screenNum, aspectWide, aspectHigh);
}

void PUPPinDisplay::setVolume(int screenNum, int vol)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetVolume(static_cast<float>(vol));
}

void PUPPinDisplay::setVolumeCurrent(int screenNum, int vol)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetVolumeCurrent(static_cast<float>(vol));
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
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, value=%d", screenNum, value);
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
   // if you set Mode=1, it will set current playing file as background (loop it always).  Mode=0 to cancel background.  Note if user has 'POP-UP' mode this will be disabled automagically (you don't need to worry about it).
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetAsBackGround(mode);
}

void PUPPinDisplay::BlockPlay(int screenNum, int mode)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, mode=%d", screenNum, mode);
}

void PUPPinDisplay::SetScreen(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d", screenNum);
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
      LOGE("Not fully implemented: screenNum=%d, xpos=%d, ypos=%d, swidth=%d, sheight=%d, popup=%d", screenNum, xpos, ypos, swidth, sheight, popup);
   }
}

int PUPPinDisplay::GetSN() const
{
   NOT_IMPLEMENTED("Not implemented");
   return 0;
}

void PUPPinDisplay::SetSN(int Value)
{
   NOT_IMPLEMENTED("Not implemented: value=%d", Value);
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
   NOT_IMPLEMENTED("Not implemented");
   return emptystring;
}

void PUPPinDisplay::SetB2SFilter(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=%s", value.c_str());
}

void PUPPinDisplay::Show(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d", screenNum);
}

void PUPPinDisplay::Hide(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (!pScreen)
      return;

   // Is off the same as Hide?
   // Seems to be called for the music screens
   pScreen->SetMode(PUPScreen::Mode::MusicOnly);
}

void PUPPinDisplay::B2SInit(const string& tName, const string& romName)
{
   m_pupManager.LoadConfig(romName);
}

void PUPPinDisplay::SendMSG(const string& szMsg)
{
   RSJresource json(szMsg);
   if (json["mt"s].exists()) {
      int mt = json["mt"s].as<int>();
      switch(mt) {
         case 301:
            if (json["SN"s].exists() && json["FN"s].exists()) {
               int sn = json["SN"s].as<int>();
               std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(sn);
               if (pScreen) {
                  int fn = json["FN"s].as<int>();
                  switch (fn) {
                     case 3:
                        // hide/show overlay text - { "mt":301, "SN": XX, "FN":3, "OT": 0 } - OT 0/1 overlay text on off bool
                        NOT_IMPLEMENTED("Show/Hide screen not implemented. szMsg=%s", szMsg.c_str());
                        break;
                     case 4:
                        // set StayOnTop { "mt":301, "SN": XX, "FN":4, "FS":1/0 }
                        LOGD("Stay on top requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetMode((json["FS"s].exists() && json["FS"s].as<int>() == 1) ? PUPScreen::Mode::ForceOn : PUPScreen::Mode::ForceBack);
                        break;
                     case 6:
                        // Bring screen to the front
                        LOGD("Bring screen to front requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        m_pupManager.SendScreenToFront(pScreen.get());
                        break;
                     case 10:
                        // set all displays all volume { "mt":301, "SN": XX, "FN":10, "VL":9}  VL=volume level
                        NOT_IMPLEMENTED("Set all displays all volume not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 11:
                        // set all volume { "mt":301, "SN": XX, "FN":11, "VL":9}  VL=volume level
                        LOGD("Set all volume requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetVolume(static_cast<float>(json["VL"s].as<double>()));
                        break;
                     case 12:
                        // "{ ""mt"":301, ""SN"": 18, ""FN"":12 }" 'STOPSCREEN
                        NOT_IMPLEMENTED("StopScreen? not implemented. szMsg=%s", szMsg.c_str());
                        break;
                     case 15:
                        // set screen custompos { 'mt':301, 'SN':15,'FN':15,'CP':'parent_screen,x,y,w,h'} CP = CustomPos String, coordinates relative in %
                        LOGD("Set screen custompos requested: screen={%s}, fn=%d, szMsg=%s",pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetCustomPos(json["CP"s].as_str());
                        break;
                     case 16:
                        // start executable
                        //   { ""mt"":301, ""SN"": 2, ""FN"":16, ""EX"": """&PuPMiniGameExe  &""", ""WT"": """&PuPMiniGameTitle&""", ""RS"":1 , ""TO"":15 , ""WZ"":0 , ""SH"": 1 , ""FT"":""Visual Pinball Player"" }
                        //   { ""mt"":301, ""SN"": 2, ""FN"":16, ""EX"": ""Pupinit.bat"", ""WT"": """", ""RS"":1 , ""TO"":15 , ""WZ"":0 , ""SH"": 1 , ""FT"":""Visual Pinball Player"" }
                        // EX: Windows executable / WT: Window Title / WZ: Window Z order / RS: ? / TO: ? / SH: ? / FT: ?
                        NOT_IMPLEMENTED("Executing OS executable is not implemented. szMsg=%s", szMsg.c_str());
                        break;
                     case 17:
                        // set window z order { ""mt"":301, ""SN"": ""2"", ""FN"":17, ""WT"":""Visual Pinball Player"", ""WZ"": 1, ""WP"": 1 }
                        // See https://learn.microsoft.com/fr-fr/windows/win32/api/winuser/nf-winuser-setwindowpos
                        // WT: Window Title / WZ: hWndInsertAfter / WP: uFlags
                        NOT_IMPLEMENTED("Set OS window z ordering is not implemented. szMsg=%s", szMsg.c_str());
                        break;
                     case 22:
                        // set screen transparency { "mt":301, "SN": 16, "FN":22, "AM":1, "AV":255 } AV: Alpha Value (0-255), AM: Alpha mode enabled 0/1?
                        NOT_IMPLEMENTED("Set screen transparency not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 30:
                        // {'mt':301, 'SN': XX, 'FN':30, 'PM':1 } set (play ?) jukebox mode: jukebox mode will auto advance to next media in playlist and you can use next/prior sub to manuall advance
                        NOT_IMPLEMENTED("Jukebox mode not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 31:
                        // pup jukebox control - {'mt':301, 'SN': XX, 'FN':31, 'PM':1 } - PM 1 = next, PM 2 = previous
                        NOT_IMPLEMENTED("Jukebox mode not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 32:
                        // "{ ""mt"":301, ""SN"": 1, ""FN"":32, ""FQ"":3 }"   'set no antialias on font render if real
                        NOT_IMPLEMENTED("Font quality is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 33:
                        // "{ ""mt"":301, ""SN"": 1, ""FN"":33 }"             'set pupdmd for mirror and hide behind other pups
                        // TODO expose the DMD screen to other pluginsthrough the generic controllar plugin API
                        NOT_IMPLEMENTED("DMD mirroring is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 34:
                        // "{ ""mt"":301, ""SN"": "& pDisp &", ""FN"": 34 }"             'hideoverlay text during next videoplay on DMD auto return
                        NOT_IMPLEMENTED("Uknown function not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 41:
                        // 'set safeloop mode on current playing media.  Good for background videos that refresh often?  { "mt":301, "SN": XX, "FN":41 }
                        NOT_IMPLEMENTED("Safe loop mode not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 42:
                        // will temporary volume duck all pups (not masterid) till masterid currently playing video ends.  will auto-return all pups to normal.
                        // VolLevel is number,  0 to mute 99 for 99%
                        // ALL may be omitted, not sure how it affects
                        // "{ ""mt"":301, ""SN"": "& MasterPuPID& ", ""FN"": 42, ""DV"": "&VolLevel&" , ""ALL"":1 }"                 
                        NOT_IMPLEMENTED("Temporary volume ducking is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 45:
                        // slow pc mode { "mt":301, "SN":XX, "FN":45, "SP":1 } - SP 0/1 = slow pc mode bool
                        NOT_IMPLEMENTED("Slow PC mode is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 46:
                        // pad all text { "mt":301, "SN": XX, "FN":46, "PA":1 } - PA 0/1 = padd text bool
                        NOT_IMPLEMENTED("Pas all text is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 50:
                        // pSetAspectRatio(PuPID, arWidth, arHeight) "{ ""mt"":301, ""SN"": "&PuPID& ", ""FN"": 50, ""WIDTH"": "&arWidth&", ""HEIGHT"": "&arHeight&" }"   
                        NOT_IMPLEMENTED("Set aspect ratio is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 51:
                        // set media play position in ms { "mt":301, "SN": XX, "FN":51, "SP": 3431} - SP position in ms
                        NOT_IMPLEMENTED("Set precise media position is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 52:
                        // pDMDSetTextQuality(AALevel)  '0 to 4 aa.  4 is sloooooower.  default 1,  perhaps use 2-3 if small desktop view.  only affect text quality.  can set per label too with 'qual' settings.
                        // "{ ""mt"":301, ""SN"": 5, ""FN"":52, ""SC"": "& AALevel &" }"    'slow pc mode
                        NOT_IMPLEMENTED("Font antialiasing level is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 53:
                        // Experimental frame rescale, FORCE higher frame size to autosize and rescale nicer,  like AA and auto-fit.
                        // "{ ""mt"":301, ""SN"": "&PuPID& ", ""FN"": 53, ""XW"": "&fWidth&", ""YH"": "&fHeight&", ""FR"":1 }"
                        NOT_IMPLEMENTED("Experimental frame rescale is not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     default:
                        NOT_IMPLEMENTED("Uknown function not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                  }
               }
               else {
                  LOGE("Screen not found: screenNum=%d, szMsg=%s", sn, szMsg.c_str());
               }
            }
            else {
               NOT_IMPLEMENTED("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
            }
            break;
         default:
            NOT_IMPLEMENTED("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
            break;
      }
   }
   else {
      LOGE("No message type found: szMsg=%s", szMsg.c_str());
   }

   return;
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

   if (!pScreen->IsLabelInit()) {
      LOGE("LabelInit has not been called: screenNum=%d", screenNum);
      return;
   }

   pScreen->AddLabel(new PUPLabel(&m_pupManager, LabelName, FontName, static_cast<float>(Size), Color, 
      static_cast<float>(Angle), (PUP_LABEL_XALIGN)xAlign, (PUP_LABEL_YALIGN)yAlign, 
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

   PUPLabel* pLabel = pScreen->GetLabel(LabelName);
   if (!pLabel) {
      if (m_warnedLabels[screenNum].find(LabelName) == m_warnedLabels[screenNum].end())
      {
         LOGE("Invalid label: screen={%s}, labelName=%s", pScreen->ToString(false).c_str(), LabelName.c_str());
         m_warnedLabels[screenNum].insert(LabelName);
      }
      return;
   }

   pLabel->SetCaption(Caption);
   pLabel->SetVisible(Visible);
   pLabel->SetSpecial(Special);
}

void PUPPinDisplay::LabelSetEx()
{
   NOT_IMPLEMENTED("Not implemented");
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
   if (pScreen)
      pScreen->SetPage(PageNum, Seconds);
}

void PUPPinDisplay::LabelInit(int screenNum)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   if (pScreen)
      pScreen->SetLabelInit();
}

const string& PUPPinDisplay::GetGetGame() const
{
   NOT_IMPLEMENTED("Not implemented");
   return emptystring;
}

void PUPPinDisplay::SetGetGame(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=%s", value.c_str());
}

string PUPPinDisplay::GetGetRoot() const
{
   // The return path is either the default one when not playing or the one actually being played (which may be a per table or a global folder)
   if (m_pupManager.GetPath().empty())
      return m_pupManager.GetRootPath().string();
   else
      return (m_pupManager.GetPath().parent_path() / "").string();
}

void PUPPinDisplay::SetGetRoot(const string& value)
{
   NOT_IMPLEMENTED("Not implemented: value=%s", value.c_str());
}

void PUPPinDisplay::SoundAdd(const string& sname, const string& fname, int svol, double sX, double sy, const string& SP)
{
   NOT_IMPLEMENTED("Not implemented: sname=%s, fname=%s, svol=%d, sX=%f, sy=%f, SP=%s", sname.c_str(), fname.c_str(), svol, sX, sy, SP.c_str());
}

void PUPPinDisplay::SoundPlay(const string& sname)
{
   NOT_IMPLEMENTED("Not implemented: sname=%s", sname.c_str());
}

void PUPPinDisplay::PuPSound(const string& sname, int sX, int sy, int sz, int vol, const string& SP)
{
   NOT_IMPLEMENTED("Not implemented: sname=%s, sX=%d, sy=%d, sz=%d, vol=%d, SP=%s", sname.c_str(), sX, sy, sz, vol, SP.c_str());
}

void PUPPinDisplay::InitPuPMenu(int Param1)
{
   NOT_IMPLEMENTED("Not implemented: param1=%d", Param1);
}

const string& PUPPinDisplay::GetB2SDisplays() const
{
   NOT_IMPLEMENTED("Not implemented");
   return emptystring;
}

int PUPPinDisplay::GetGameUpdate(const string& GameTitle, int Func, int FuncData, const string& Extra) const
{
   // Used in PizzaTime to communicate with the mini game app:
   // - forward key up / key down
   // - check for mini game state
   // - readback the value of the score
   NOT_IMPLEMENTED("Not implemented: gameTitle=%s, func=%d, funcData=%d, extra=%s", GameTitle.c_str(), Func, FuncData, Extra.c_str());
   return 0;
}

/* STDMETHODIMP PUPPinDisplay::GrabDC(int pWidth, int pHeight, BSTR wintitle, VARIANT* pixels)
{
   LOGE("Not implemented: pWidth=%d, pHeight=%d, wintitle=%s", pWidth, pHeight, MakeString(wintitle).c_str());

   return;
}*/

string PUPPinDisplay::GetVersion() const
{
   constexpr int nVersionNo0 = 1;
   constexpr int nVersionNo1 = 5;
   constexpr int nVersionNo2 = 99;
   constexpr int nVersionNo3 = 99;
   return std::to_string(nVersionNo0) + '.' + std::to_string(nVersionNo1) + '.' + std::to_string(nVersionNo2) + '.' + std::to_string(nVersionNo3);
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
   // TODO handle seconds and Special
   pScreen->Play(playlist, playfilename, static_cast<float>(volume), priority);

   //  'playtype for triggers
   //  'ptNormal=0;
   //  'ptLoop=1;
   //  'ptSplashReset=2;
   //  'ptSplashResume=3;
   //  'ptStopScreen=4;
   //  'ptStopFile=5;
   //  'ptSetBG=6;
   //  'ptPlaySSF=7;
   //  'ptSkipSameP=8;
   //  'ptCustomFunc=9;
   //  'ptForcePlay=10;
   //  'ptQueueSameP=11;
   //  'ptQueueAlways=12;
   switch (playtype) {
   case 0:
      // Normal
      break;
   case 1: // Loop
      pScreen->SetLoop(1);
      break;
   case 6: // SetBG
      pScreen->SetAsBackGround(1);
      break;
   default:
      NOT_IMPLEMENTED("Not implemented: playevent playtype=%d", playtype);
   }
}

void PUPPinDisplay::SetPosVideo(int screenNum, int StartPos, int EndPos, int mode, const string& Special)
{
   std::shared_ptr<PUPScreen> pScreen = m_pupManager.GetScreen(screenNum, true);
   NOT_IMPLEMENTED("Not implemented: screenNum=%d, startPos=%d, endPos=%d, mode=%d, special=%s", screenNum, StartPos, EndPos, mode, Special.c_str());
}

void PUPPinDisplay::PuPClose()
{
   m_pupManager.Unload();
}

}
