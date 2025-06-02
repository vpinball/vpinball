#include "PUPPinDisplay.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPLabel.h"

#include <iostream>
#include <fstream>
#include <sstream>

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
}

void PUPPinDisplay::Init(int screenNum, const string& RootDir)
{
   if (m_pupManager.HasScreen(screenNum))
   {
      LOGE("Screen already exists: screenNum=%d", screenNum);
      return;
   }
   m_pupManager.AddScreen(screenNum);
}

void PUPPinDisplay::playlistadd(int screenNum, const string& folder, int sort, int restSeconds)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }

   if (pScreen->GetPlaylist(folder)) {
      LOGE("Playlist already exists: screenNum=%d, folder=%s", screenNum, folder.c_str());
      return;
   }

   pScreen->AddPlaylist(new PUPPlaylist(&m_pupManager, folder, ""s, sort, restSeconds, 100, 1));
}

void PUPPinDisplay::playlistplay(int screenNum, const string& playlist)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, playlist=%s", screenNum, playlist.c_str());
}

void PUPPinDisplay::playlistplayex(int screenNum, const string& playlist, const string& playfilename, int volume, int forceplay)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->QueuePlay(playlist, playfilename, static_cast<float>(volume), forceplay);
}

void PUPPinDisplay::play(int screenNum, const string& playlist, const string& playfilename)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, playlist=%s, playfilename=%s", screenNum, playlist.c_str(), playfilename.c_str());
   return;
}

void PUPPinDisplay::setWidth(int screenNum, int width)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, width=%d", screenNum, width);
}

void PUPPinDisplay::setHeight(int screenNum, int Height)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, height=%d", screenNum, Height);
}

void PUPPinDisplay::setPosX(int screenNum, int Posx)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, Posx=%d", screenNum, Posx);
}

void PUPPinDisplay::setPosY(int screenNum, int PosY)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, PosY=%d", screenNum, PosY);
}

void PUPPinDisplay::setAspect(int screenNum, int aspectWide, int aspectHigh)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, aspectWide=%d, aspectHigh=%d", screenNum, aspectWide, aspectHigh);
}

void PUPPinDisplay::setVolume(int screenNum, int vol)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d, vol=%d", screenNum, vol);
}

void PUPPinDisplay::playpause(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d", screenNum);
   return;
}

void PUPPinDisplay::playresume(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d");
      return;
   }
   LOGE("Not implemented: screenNum=%d", screenNum);
   return;
}

void PUPPinDisplay::playstop(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->QueueStop();
}

void PUPPinDisplay::CloseApp()
{
   LOGE("Not implemented");
}

bool PUPPinDisplay::GetisPlaying(int screenNum) const
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return false;
   }
   LOGE("Not implemented: screenNum=%d", screenNum);
   return false;
}

void PUPPinDisplay::SetisPlaying(int screenNum, bool value)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d, value=%d", screenNum, value);
}

void PUPPinDisplay::SetLength(int screenNum, int StopSecs)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d, stopSecs=%d", screenNum, StopSecs);
}

void PUPPinDisplay::SetLoop(int screenNum, int LoopState)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->SetLoop(LoopState);
}

void PUPPinDisplay::SetBackGround(int screenNum, int Mode)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->SetBG(Mode);
}

void PUPPinDisplay::BlockPlay(int screenNum, int Mode)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d, mode=%d", screenNum, Mode);
}

void PUPPinDisplay::SetScreen(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d", screenNum);
}

void PUPPinDisplay::SetScreenEx(int screenNum, int xpos, int ypos, int swidth, int sheight, int popup) 
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   switch (popup) {
      case 0:
         pScreen->SetMode(PUP_SCREEN_MODE_SHOW);
         break;
      case 1:
         pScreen->SetMode(PUP_SCREEN_MODE_FORCE_POP);
         break;
      default:
         pScreen->SetMode(PUP_SCREEN_MODE_MUSIC_ONLY);
         break;
   }
   LOGE("Not fully implemented: screenNum=%d, xpos=%d, ypos=%d, swidth=%d, sheight=%d, popup=%d", screenNum, xpos, ypos, swidth, sheight, popup);
}

int PUPPinDisplay::GetSN() const
{
   LOGE("Not implemented");
   return 0;
}

void PUPPinDisplay::SetSN(int Value)
{
   LOGE("Not implemented: value=%d", Value);
}

void PUPPinDisplay::B2SData(const string& tIndex, int Value)
{
   const string tmp = tIndex.substr(1);
   int result;
   std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), result);

   m_pupManager.QueueTriggerData({ tIndex[0], result, Value });
}

const string& PUPPinDisplay::GetB2SFilter() const
{
   LOGE("Not implemented");
   return emptystring;
}

void PUPPinDisplay::SetB2SFilter(const string& value)
{
   LOGE("Not implemented: value=%s", value.c_str());
}

void PUPPinDisplay::Show(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d", screenNum);
}

void PUPPinDisplay::Hide(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }

   // Is off the same as Hide?
   // Seems to be called for the music screens
   pScreen->SetMode(PUP_SCREEN_MODE_MUSIC_ONLY);
}

void PUPPinDisplay::B2SInit(const string& tName, const string& romName)
{
   m_pupManager.LoadConfig(romName);
   m_pupManager.Start();
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
               PUPScreen* pScreen = m_pupManager.GetScreen(sn);
               if (pScreen) {
                  int fn = json["FN"s].as<int>();
                  switch (fn) {
                     case 4:
                        // set StayOnTop { "mt":301, "SN": XX, "FN":4, "FS":1/0 }
                        LOGD("Stay on top requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetMode((json["FS"s].exists() && json["FS"s].as<int>() == 1) ? PUP_SCREEN_MODE_FORCE_ON : PUP_SCREEN_MODE_FORCE_BACK);
                        break;
                     case 6:
                        // Bring screen to the front
                        LOGD("Bring screen to front requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SendToFront();
                        break;
                     case 10:
                        // set all displays all volume { "mt":301, "SN": XX, "FN":10, "VL":9}  VL=volume level
                        LOGE("Set all displays all volume not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 11:
                        // set all volume { "mt":301, "SN": XX, "FN":11, "VL":9}  VL=volume level
                        LOGD("Set all volume requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetVolume(std::stof(json["VL"].as_str()));
                        break;
                     case 15:
                        // set screen custompos { 'mt':301, 'SN':15,'FN':15,'CP':'parent_screen,x,y,w,h'} CP = CustomPos String, coordinates relative in %
                        LOGD("Set screen custompos requested: screen={%s}, fn=%d, szMsg=%s",pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetCustomPos(json["CP"].as_str());
                        break;
                     case 22:
                        // set screen transparency { "mt":301, "SN": 16, "FN":22, "AM":1, "AV":255 } AV: Alpha Value (0-255), AM: Alpha mode enabled 0/1?
                        LOGE("Set screen transparency not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                     default:
                        LOGE("Not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                  }
               }
               else {
                  LOGE("Screen not found: screenNum=%d, szMsg=%s", sn, szMsg.c_str());
               }
            }
            else {
               LOGE("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
            }
            break;
         default:
            LOGE("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
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
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }

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
   static ankerl::unordered_dense::map<int, ankerl::unordered_dense::set<string>> warnedLabels;

   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }

   PUPLabel* pLabel = pScreen->GetLabel(LabelName);
   if (!pLabel) {
      if (warnedLabels[screenNum].find(LabelName) == warnedLabels[screenNum].end())
      {
         LOGE("Invalid label: screen={%s}, labelName=%s", pScreen->ToString(false).c_str(), LabelName.c_str());
         warnedLabels[screenNum].insert(LabelName);
      }
      return;
   }

   pLabel->SetCaption(Caption);
   pLabel->SetVisible(Visible);
   pLabel->SetSpecial(Special);
}

void PUPPinDisplay::LabelSetEx()
{
   LOGE("Not implemented");
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
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->SetPage(PageNum, Seconds);
}

void PUPPinDisplay::LabelInit(int screenNum)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   pScreen->SetLabelInit();
}

const string& PUPPinDisplay::GetGetGame() const
{
   LOGE("Not implemented");
   return emptystring;
}

void PUPPinDisplay::SetGetGame(const string& value)
{
   LOGE("Not implemented: value=%s", value.c_str());
}

const string& PUPPinDisplay::GetGetRoot() const
{
   return m_pupManager.GetRootPath();
}

void PUPPinDisplay::SetGetRoot(const string& value)
{
   LOGE("Not implemented: value=%s", value.c_str());
}

void PUPPinDisplay::SoundAdd(const string& sname, const string& fname, int svol, double sX, double sy, const string& SP)
{
   LOGE("Not implemented: sname=%s, fname=%s, svol=%d, sX=%f, sy=%f, SP=%s", sname.c_str(), fname.c_str(), svol, sX, sy, SP.c_str());
}

void PUPPinDisplay::SoundPlay(const string& sname)
{
   LOGE("Not implemented: sname=%s", sname.c_str());
}

void PUPPinDisplay::PuPSound(const string& sname, int sX, int sy, int sz, int vol, const string& SP)
{
   LOGE("Not implemented: sname=%s, sX=%d, sy=%d, sz=%d, vol=%d, SP=%s", sname.c_str(), sX, sy, sz, vol, SP.c_str());
}

void PUPPinDisplay::InitPuPMenu(int Param1)
{
   LOGE("Not implemented: param1=%d", Param1);
}

const string& PUPPinDisplay::GetB2SDisplays() const
{
   LOGE("Not implemented");
   return emptystring;
}

void PUPPinDisplay::setVolumeCurrent(int screenNum, int vol)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d, vol=%d", screenNum, vol);
}

int PUPPinDisplay::GetGameUpdate(const string& GameTitle, int Func, int FuncData, const string& Extra) const
{
   LOGE("Not implemented: gameTitle=%s, func=%d, funcData=%d, extra=%s", GameTitle.c_str(), Func, FuncData, Extra.c_str());
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
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   // TODO handle seconds and Special
   pScreen->QueuePlay(playlist, playfilename, static_cast<float>(volume), priority);

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
      pScreen->QueueLoop(1);
      break;
   case 6: // SetBG
      pScreen->QueueBG(1);
      break;
   default:
      LOGE("Not implemented: playevent playtype=%d", playtype);
   }
}

void PUPPinDisplay::SetPosVideo(int screenNum, int StartPos, int EndPos, int Mode, const string& Special)
{
   PUPScreen* pScreen = m_pupManager.GetScreen(screenNum);
   if (!pScreen) {
      LOGE("Screen not found: screenNum=%d", screenNum);
      return;
   }
   LOGE("Not implemented: screenNum=%d, startPos=%d, endPos=%d, mode=%d, special=%s", screenNum, StartPos, EndPos, Mode, Special.c_str());
}

void PUPPinDisplay::PuPClose()
{
   LOGE("Not implemented");
}

}