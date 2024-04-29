#include "core/stdafx.h"
#include "PUPPinDisplay.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPLabel.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <SDL2/SDL_ttf.h>
#include "RSJparser/RSJparser.tcc"

PUPPinDisplay::PUPPinDisplay()
{
   m_pManager = PUPManager::GetInstance();

   TTF_Init();
}

PUPPinDisplay::~PUPPinDisplay()
{
   m_pManager->Stop();
}

STDMETHODIMP PUPPinDisplay::Init(LONG ScreenNum, BSTR RootDir)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistadd(LONG ScreenNum, BSTR folder, LONG sort, LONG restSeconds)
{
   PLOGW.printf("Not implemented: screenNum=%d, folder=%s, sort=%d, restSeconds=%d", ScreenNum, MakeString(folder).c_str(), sort, restSeconds);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplay(LONG ScreenNum, BSTR playlist)
{
   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s", ScreenNum, MakeString(playlist).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplayex(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s, playfilename=%s, volume=%d, forceplay=%d", ScreenNum, MakeString(playlist).c_str(), MakeString(playfilename).c_str(), volume, forceplay);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::play(LONG ScreenNum, BSTR playlist, BSTR playfilename)
{
   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s, playfilename=%s", ScreenNum, MakeString(playlist).c_str(), MakeString(playfilename).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setWidth(LONG ScreenNum, LONG width)
{
   PLOGW.printf("Not implemented: screenNum=%d, width=%d", ScreenNum, width);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setHeight(LONG ScreenNum, LONG Height)
{
   PLOGW.printf("Not implemented: screenNum=%d, height=%d", ScreenNum, Height);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosX(LONG ScreenNum, LONG Posx)
{
   PLOGW.printf("Not implemented: screenNum=%d, Posx=%d", ScreenNum, Posx);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosY(LONG ScreenNum, LONG PosY)
{
   PLOGW.printf("Not implemented: screenNum=%d, PosY=%d", ScreenNum, PosY);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setAspect(LONG ScreenNum, LONG aspectWide, LONG aspectHigh)
{
   PLOGW.printf("Not implemented: screenNum=%d, aspectWide=%d, aspectHigh=%d", ScreenNum, aspectWide, aspectHigh);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setVolume(LONG ScreenNum, LONG vol)
{
   PLOGW.printf("Not implemented: screenNum=%d, vol=%d", ScreenNum, vol);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playpause(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playresume(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playstop(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::CloseApp()
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_isPlaying(LONG ScreenNum, LONG *Value)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_isPlaying(LONG ScreenNum, LONG Value)
{
   PLOGW.printf("Not implemented: screenNum=%d, value=%d", ScreenNum, Value);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLength(LONG ScreenNum, LONG StopSecs)
{
   PLOGW.printf("Not implemented: screenNum=%d, stopSecs=%d", ScreenNum, StopSecs);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLoop(LONG ScreenNum, LONG LoopState)
{
   PLOGW.printf("Not implemented: screenNum=%d, loopState=%d", ScreenNum, LoopState);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetBackGround(LONG ScreenNum, LONG Mode)
{
   PLOGW.printf("Not implemented: screenNum=%d, mode=%d", ScreenNum, Mode);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::BlockPlay(LONG ScreenNum, LONG Mode)
{
   PLOGW.printf("Not implemented: screenNum=%d, mode=%d", ScreenNum, Mode);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreen(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreenEx(LONG ScreenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
   PLOGW.printf("Not implemented: screenNum=%d, xpos=%d, ypos=%d, swidth=%d, sheight=%d, popup=%d", ScreenNum, xpos, ypos, swidth, sheight, popup);

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

   m_pManager->QueueTriggerData({ szData[0] , std::stoi(szData.substr(1)), Value });

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

STDMETHODIMP PUPPinDisplay::Show(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::Hide(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
   if (m_pManager->IsInit()) {
      PLOGW.printf("PUP already initialized");
      return S_OK;
   }

   string szRomName = MakeString(RomName);

   if (m_pManager->LoadConfig(szRomName))
      m_pManager->Start();

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SendMSG(BSTR cMSG)
{
   PLOGW << "Not implemented";

   return S_OK;
}

/*
   ScreenNum - in standard we'd set this to pDMD ( or 1)
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

STDMETHODIMP PUPPinDisplay::LabelNew(LONG ScreenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xMargin, LONG yMargin, LONG PageNum, LONG Visible)
{
   PUPScreen* pScreen = m_pManager->GetScreen(ScreenNum);
   if (!pScreen) {
      PLOGW.printf("Invalid target screen: screenNum=%d", ScreenNum);
      return S_OK;
   }

   TTF_Font* pFont = m_pManager->GetFont(MakeString(FontName));
   if (!pFont) {
      PLOGW.printf("Invalid font: fontName=%s", MakeString(FontName).c_str());
      return S_OK;
   }

   pScreen->AddLabel(MakeString(LabelName),
      new PUPLabel(pFont, Size, Color, Angle, (PUP_LABEL_XALIGN)xAlign, (PUP_LABEL_YALIGN)yAlign, xMargin, yMargin, Visible, PageNum));

   return S_OK;
}

/*
   ScreenNum - same as when adding new label.
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

STDMETHODIMP PUPPinDisplay::LabelSet(LONG ScreenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
   PUPScreen* pScreen = m_pManager->GetScreen(ScreenNum);
   if (!pScreen) {
      PLOGW.printf("Invalid target screen: screenNum=%d", ScreenNum);
      return S_OK;
   }

   PUPLabel* pLabel = pScreen->GetLabel(MakeString(LabelName));
   if (!pLabel) {
      PLOGW.printf("Invalid label: labelName=%s", MakeString(LabelName).c_str());
      return S_OK;
   }

   string szNewText = MakeString(Caption);
   if (pLabel->GetText() != szNewText) {
      pLabel->SetText(szNewText);
   }

   pLabel->SetVisible(Visible);

   string szJSON = MakeString(Special);
   if (!szJSON.empty()) {
      RSJresource json(szJSON);

      switch (json["mt"].as<int>(0)) {
         case 2: {
            SDL_Rect rect = pScreen->GetRect();
      
            if (json["size"].exists())
               pLabel->SetSize(std::stof(json["size"].as_str()));

            if (json["xpos"].exists())
               pLabel->SetXPos(std::stof(json["xpos"].as_str()));

            if (json["ypos"].exists())
               pLabel->SetYPos(std::stof(json["ypos"].as_str()));

            if (json["fname"].exists()) {
               string szFontName = json["fname"].as_str();
               TTF_Font* pFont = m_pManager->GetFont(szFontName);
                if (pFont)
                    pLabel->SetFont(pFont);
                else {
                    PLOGW.printf("Invalid font: fontName=%s", szFontName.c_str());
                }
            }

            if (json["color"].exists())
               pLabel->SetColor(json["color"].as<int>());

            if (json["xalign"].exists())
               pLabel->SetXAlign((PUP_LABEL_XALIGN)json["xalign"].as<int>());

            if (json["yalign"].exists())
               pLabel->SetYAlign((PUP_LABEL_YALIGN)json["yalign"].as<int>());

            if (json["pagenum"].exists())
                pLabel->SetPageNum(json["pagenum"].as<int>());
         }
         break;
         
         case 1: { 
            /*
               Animate
               at = animate type (1=flashing, 2=motion)
               fq = when flashing its the frequency of flashing
               len = length in ms of animation
               fc = foreground color of text during animation
               PLOGW << "Label animation not implemented";
            */
         }
         break;

         default:
            PLOGW.printf("Unknown message type: mt=%s", json["mt"].as_str().c_str());
            break;
      }
   }

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelSetEx()
{
   PLOGW << "Not implemented";

   return S_OK;
}

/*
   ScreenNum
   PageNum - page to view
   Seconds - seconds to show page...this is useful for 'splash' pages. If your page is a splash page then
             you would set this to 3 for 3 second splash. It will auto-return to the default page. The
             default page is the one that was last set with seconds=0; So we set seconds to 0 to tell driver 
             that this is the current active page.
   Special - future.
*/

STDMETHODIMP PUPPinDisplay::LabelShowPage(LONG ScreenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
   PUPScreen* pScreen = m_pManager->GetScreen(ScreenNum);
   if (!pScreen) {
      PLOGW.printf("Invalid target screen: screenNum=%d", ScreenNum);
      return S_OK;
   }

   pScreen->SetPage(PageNum, Seconds);

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelInit(LONG ScreenNum)
{
   PLOGW.printf("Not implemented: screenNum=%d", ScreenNum);

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
   string szPath = g_pvp->m_currentTablePath;

   CComBSTR Val(szPath.c_str());
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

STDMETHODIMP PUPPinDisplay::setVolumeCurrent(LONG ScreenNum, LONG vol)
{
   PLOGW.printf("Not implemented: screenNum=%d, vol=%d", ScreenNum, vol);

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

STDMETHODIMP PUPPinDisplay::playevent(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG priority, LONG playtype, LONG Seconds, BSTR Special)
{
   PLOGW.printf("Not implemented: screenNum=%d, playlist=%s, playfilename=%s, volume=%d, priority=%d, playtype=%d, seconds=%d, special=%s", ScreenNum, MakeString(playlist).c_str(), MakeString(playfilename).c_str(), volume, priority, playtype, Seconds, MakeString(Special).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetPosVideo(LONG ScreenNum, LONG StartPos, LONG EndPos, LONG Mode, BSTR Special)
{
   PLOGW.printf("Not implemented: screenNum=%d, startPos=%d, endPos=%d, mode=%d, special=%s", ScreenNum, StartPos, EndPos, Mode, MakeString(Special).c_str());

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::PuPClose()
{
   PLOGW << "Not implemented";

   return S_OK;
}