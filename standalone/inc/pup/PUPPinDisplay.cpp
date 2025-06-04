#include "core/stdafx.h"
#include "PUPPinDisplay.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPLabel.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "RSJparser/RSJparser.tcc"

#include "standalone/Standalone.h"

PUPPinDisplay::PUPPinDisplay()
{
   m_pManager = Standalone::GetInstance()->GetPUPManager();
}

PUPPinDisplay::~PUPPinDisplay()
{
   m_pManager->Stop();
}

STDMETHODIMP PUPPinDisplay::Init(LONG screenNum, BSTR RootDir)
{
   if (m_pManager->HasScreen(screenNum)) {
      PLOGW.printf("Screen already exists: screenNum=%d", screenNum);
      return S_OK;
   }
   m_pManager->AddScreen(screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistadd(LONG screenNum, BSTR folder, LONG sort, LONG restSeconds)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   if (pScreen->GetPlaylist(MakeString(folder))) {
      PLOGW.printf("Playlist already exists: screenNum=%d, folder=%s", screenNum, MakeString(folder).c_str());
      return S_OK;
   }

   pScreen->AddPlaylist(new PUPPlaylist(m_pManager, MakeString(folder), "", sort, restSeconds, 100, 1));

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplay(LONG screenNum, BSTR playlist)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s", screenNum, MakeString(playlist).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplayex(LONG screenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   pScreen->QueuePlay(MakeString(playlist), MakeString(playfilename), volume, forceplay);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::play(LONG screenNum, BSTR playlist, BSTR playfilename)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s, playfilename=%s", screenNum, MakeString(playlist).c_str(), MakeString(playfilename).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setWidth(LONG screenNum, LONG width)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, width=%d", screenNum, width);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setHeight(LONG screenNum, LONG Height)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, height=%d", screenNum, Height);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosX(LONG screenNum, LONG Posx)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, Posx=%d", screenNum, Posx);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosY(LONG screenNum, LONG PosY)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, PosY=%d", screenNum, PosY);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setAspect(LONG screenNum, LONG aspectWide, LONG aspectHigh)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, aspectWide=%d, aspectHigh=%d", screenNum, aspectWide, aspectHigh);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setVolume(LONG screenNum, LONG vol)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, vol=%d", screenNum, vol);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playpause(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d", screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playresume(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d");
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d", screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playstop(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   pScreen->QueueStop();

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::CloseApp()
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_isPlaying(LONG screenNum, LONG *Value)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d", screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_isPlaying(LONG screenNum, LONG Value)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, value=%d", screenNum, Value);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLength(LONG screenNum, LONG StopSecs)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, stopSecs=%d", screenNum, StopSecs);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLoop(LONG screenNum, LONG LoopState)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   pScreen->SetLoop(LoopState);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetBackGround(LONG screenNum, LONG Mode)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   pScreen->SetBG(Mode);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::BlockPlay(LONG screenNum, LONG Mode)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, mode=%d", screenNum, Mode);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreen(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d", screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreenEx(LONG screenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
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
   PLOGW.printf("Not fully implemented: screenNum=%d, xpos=%d, ypos=%d, swidth=%d, sheight=%d, popup=%d", screenNum, xpos, ypos, swidth, sheight, popup);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_SN(LONG *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_SN(LONG Value)
{
   PLOGW.printf("Not implemented: value=%d", Value);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::B2SData(BSTR tIndex, int Value)
{
   string szData = MakeString(tIndex);
   m_pManager->QueueTriggerData({ szData[0], std::stoi(szData.substr(1)), Value });

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_B2SFilter(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_B2SFilter(BSTR Value)
{
   PLOGW.printf("Not implemented: value=%s", MakeString(Value).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::Show(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d", screenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::Hide(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   // Is off the same as Hide?
   // Seems to be called for the music screens
   pScreen->SetMode(PUP_SCREEN_MODE_MUSIC_ONLY);


   return S_OK;
}

STDMETHODIMP PUPPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
   if (m_pManager->IsInit()) {
      PLOGW.printf("PUP already initialized");
      return S_OK;
   }

   m_pManager->LoadConfig(MakeString(RomName));

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SendMSG(BSTR cMSG)
{
   string szMsg = MakeString(cMSG);

   RSJresource json(szMsg);
   if (json["mt"s].exists()) {
      int mt = json["mt"s].as<int>();
      switch(mt) {
         case 301:
            if (json["SN"s].exists() && json["FN"s].exists()) {
               int sn = json["SN"s].as<int>();
               PUPScreen* pScreen = m_pManager->GetScreen(sn);
               if (pScreen) {
                  int fn = json["FN"s].as<int>();
                  switch (fn) {
                     case 4:
                        // set StayOnTop { "mt":301, "SN": XX, "FN":4, "FS":1/0 }
                        PLOGD.printf("Stay on top requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetMode((json["FS"s].exists() && json["FS"s].as<int>() == 1) ? PUP_SCREEN_MODE_FORCE_ON : PUP_SCREEN_MODE_FORCE_BACK);
                        break;
                     case 6:
                        // Bring screen to the front
                        PLOGD.printf("Bring screen to front requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SendToFront();
                        break;
                     case 10:
                        // set all displays all volume { "mt":301, "SN": XX, "FN":10, "VL":9}  VL=volume level
                        PLOGE.printf("Set all displays all volume not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                     case 11:
                        // set all volume { "mt":301, "SN": XX, "FN":11, "VL":9}  VL=volume level
                        PLOGD.printf("Set all volume requested: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetVolume(static_cast<float>(json["VL"s].as<double>()));
                        break;
                     case 15:
                        // set screen custompos { 'mt':301, 'SN':15,'FN':15,'CP':'parent_screen,x,y,w,h'} CP = CustomPos String, coordinates relative in %
                        PLOGD.printf("Set screen custompos requested: screen={%s}, fn=%d, szMsg=%s",pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        pScreen->SetCustomPos(json["CP"s].as_str());
                        break;
                     case 22:
                        // set screen transparency { "mt":301, "SN": 16, "FN":22, "AM":1, "AV":255 } AV: Alpha Value (0-255), AM: Alpha mode enabled 0/1?
                        PLOGE.printf("Set screen transparency not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                     default:
                        PLOGE.printf("Not implemented: screen={%s}, fn=%d, szMsg=%s", pScreen->ToString(false).c_str(), fn, szMsg.c_str());
                        break;
                  }
               }
               else {
                  PLOGW.printf("Screen not found: screenNum=%d, szMsg=%s", sn, szMsg.c_str());
               }
            }
            else {
               PLOGW.printf("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
            }
            break;
         default:
            PLOGW.printf("Not implemented: mt=%d, szMsg=%s", mt, szMsg.c_str());
            break;
      }
   }
   else {
      PLOGW.printf("No message type found: szMsg=%s", szMsg.c_str());
   }

   return S_OK;
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

STDMETHODIMP PUPPinDisplay::LabelNew(LONG screenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xMargin, LONG yMargin, LONG PageNum, LONG Visible)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   if (!pScreen->IsLabelInit()) {
      PLOGW.printf("LabelInit has not been called: screenNum=%d", screenNum);
      return S_OK;
   }

   pScreen->AddLabel(
      new PUPLabel(m_pManager, MakeString(LabelName), MakeString(FontName), Size, Color, Angle, (PUP_LABEL_XALIGN)xAlign, (PUP_LABEL_YALIGN)yAlign, xMargin, yMargin, PageNum, Visible));

   return S_OK;
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

STDMETHODIMP PUPPinDisplay::LabelSet(LONG screenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
   static ankerl::unordered_dense::map<LONG, ankerl::unordered_dense::set<string>> warnedLabels;

   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   string szLabelName = MakeString(LabelName);
   PUPLabel* pLabel = pScreen->GetLabel(szLabelName);
   if (!pLabel) {
      if (warnedLabels[screenNum].find(szLabelName) == warnedLabels[screenNum].end())
      {
         PLOGW.printf("Invalid label: screen={%s}, labelName=%s", pScreen->ToString(false).c_str(), szLabelName.c_str());
         warnedLabels[screenNum].insert(szLabelName);
      }
      return S_OK;
   }

   pLabel->SetCaption(MakeString(Caption));
   pLabel->SetVisible(Visible);
   pLabel->SetSpecial(MakeString(Special));

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelSetEx()
{
   PLOGW << "Not implemented";

   return S_OK;
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

STDMETHODIMP PUPPinDisplay::LabelShowPage(LONG screenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }
   pScreen->SetPage(PageNum, Seconds);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelInit(LONG screenNum)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }
   pScreen->SetLabelInit();

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GetGame(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_GetGame(BSTR Value)
{
   PLOGW.printf("Not implemented: value=%s", MakeString(Value).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GetRoot(BSTR *Value)
{
   CComBSTR Val(m_pManager->GetRootPath().c_str());
   *Value = Val.Detach();

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_GetRoot(BSTR Value)
{
   PLOGW.printf("Not implemented: value=%s", MakeString(Value).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SoundAdd(BSTR sname, BSTR fname, LONG svol, double sX, double sy, BSTR SP)
{
   PLOGW.printf("Not implemented: sname=%s, fname=%s, svol=%d, sX=%f, sy=%f, SP=%s", MakeString(sname).c_str(), MakeString(fname).c_str(), svol, sX, sy, MakeString(SP).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SoundPlay(BSTR sname)
{
   PLOGW.printf("Not implemented: sname=%s", MakeString(sname).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::PuPSound(BSTR sname, LONG sX, LONG sy, LONG sz, LONG vol, BSTR SP)
{
   PLOGW.printf("Not implemented: sname=%s, sX=%d, sy=%d, sz=%d, vol=%d, SP=%s", MakeString(sname).c_str(), sX, sy, sz, vol, MakeString(SP).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::InitPuPMenu(LONG Param1)
{
   PLOGW.printf("Not implemented: param1=%d", Param1);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_B2SDisplays(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setVolumeCurrent(LONG screenNum, LONG vol)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, vol=%d", screenNum, vol);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GameUpdate(BSTR GameTitle, LONG Func, LONG FuncData, BSTR Extra, LONG *Value)
{
   PLOGW.printf("Not implemented: gameTitle=%s, func=%d, funcData=%d, extra=%s", MakeString(GameTitle).c_str(), Func, FuncData, MakeString(Extra).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::GrabDC(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels)
{
   PLOGW.printf("Not implemented: pWidth=%d, pHeight=%d, wintitle=%s", pWidth, pHeight, MakeString(wintitle).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::GetVersion(BSTR *pver)
{
   int nVersionNo0 = 1;
   int nVersionNo1 = 5;
   int nVersionNo2 = 99;
   int nVersionNo3 = 99;

   char szVersion[MAXTOKEN];
   sprintf_s(szVersion, sizeof(szVersion), "%d.%d.%d.%d", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);

   WCHAR wzVersion[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, szVersion, -1, wzVersion, MAXTOKEN);

   *pver = SysAllocString(wzVersion);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::GrabDC2(LONG pWidth, LONG pHeight, BSTR wintitle, SAFEARRAY **pixels)
{
   PLOGW.printf("Not implemented: pWidth=%d, pHeight=%d, wintitle=%s", pWidth, pHeight, MakeString(wintitle).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playevent(LONG screenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG priority, LONG playtype, LONG Seconds, BSTR Special)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }
   // TODO handle seconds and Special
   pScreen->QueuePlay(MakeString(playlist), MakeString(playfilename), volume, priority);

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
      PLOGW.printf("Not implemented: playevent playtype=%d", playtype);
   }

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetPosVideo(LONG screenNum, LONG StartPos, LONG EndPos, LONG Mode, BSTR Special)
{
   PUPScreen* pScreen = m_pManager->GetScreen(screenNum);
   if (!pScreen) {
      PLOGW.printf("Screen not found: screenNum=%d", screenNum);
      return S_OK;
   }

   PLOGW.printf("Not implemented: screenNum=%d, startPos=%d, endPos=%d, mode=%d, special=%s", screenNum, StartPos, EndPos, Mode, MakeString(Special).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::PuPClose()
{
   PLOGW << "Not implemented";

   return S_OK;
}
