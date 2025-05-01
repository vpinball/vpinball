#pragma once

#include "common.h"

class PUPManager;

class PUPPinDisplay
{
public:
   PUPPinDisplay(PUPManager& manager);
   ~PUPPinDisplay();

   PSC_IMPLEMENT_REFCOUNT()

   void Init(int screenNum, string rootDir);
   void playlistadd(int screenNum, string folder, int sort, int restSeconds);
   void playlistplay(int screenNum, string playlist);
   void playlistplayex(int screenNum, string playlist, string playfilename, int volume, int forceplay);
   void play(int screenNum, string playlist, string playfilename);
   void setWidth(int screenNum, int width);
   void setHeight(int screenNum, int Height);
   void setPosX(int screenNum, int Posx);
   void setPosY(int screenNum, int PosY);
   void setAspect(int screenNum, int aspectWide, int aspectHigh);
   void setVolume(int screenNum, int vol);
   void playpause(int screenNum);
   void playresume(int screenNum);
   void playstop(int screenNum);
   void CloseApp();
   bool GetisPlaying(int screenNum);
   void SetisPlaying(int screenNum, bool value);
   void SetLength(int screenNum, int stopSecs);
   void SetLoop(int screenNum, int loopState);
   void SetBackGround(int screenNum, int mode);
   void BlockPlay(int screenNum, int mode);
   void SetScreen(int screenNum);
   void SetScreenEx(int screenNum, int xpos, int ypos, int swidth, int sheight, int popup);
   int GetSN();
   void SetSN(int value);
   void B2SData(string tIndex, int value);
   string GetB2SFilter();
   void SetB2SFilter(string Value);
   void Show(int screeNum);
   void Hide(int screeNum);
   void B2SInit(string tName, string romName);
   void SendMSG(string cMsg);
   void LabelNew(int screenNum, string labelName, string fontName, int size, int color, int angle, int xAlign, int yAlign, int xMargin, int yMargin, int pageNum, bool visible);
   void LabelSet(int screenNum, string labelName, string caption, bool visible, string special);
   void LabelSetEx();
   void LabelShowPage(int screenNum, int pageNum, int seconds, string special);
   void LabelInit(int screenNum);
   string GetGetGame();
   void SetGetGame(string value);
   string GetGetRoot();
   void SetGetRoot(string value);
   void SoundAdd(string sname, string fname, int svol, double sX, double sy, string SP);
   void SoundPlay(string sname);
   void PuPSound(string sname, int sX, int sy, int sz, int vol, string SP);
   void InitPuPMenu(int Param1);
   string GetB2SDisplays();
   void setVolumeCurrent(int ScreenNum, int vol);
   int GetGameUpdate(string GameTitle, int Func, int FuncData, string Extra);
   // void GrabDC(int pWidth, int pHeight, string wintitle, VARIANT *pixels);
   string GetVersion();
   // void GrabDC2(int pWidth, int pHeight, string wintitle, SAFEARRAY **pixels);
   void playevent(int ScreenNum, string playlist, string playfilename, int volume, int priority, int playtype, int Seconds, string Special);
   void SetPosVideo(int ScreenNum, int StartPos, int EndPos, int Mode, string Special);
   void PuPClose();

private:
   PUPManager& m_pupManager;
};