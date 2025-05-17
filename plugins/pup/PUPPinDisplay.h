#pragma once

#include "common.h"

class PUPManager;

class PUPPinDisplay final
{
public:
   PUPPinDisplay(PUPManager& manager);
   ~PUPPinDisplay();

   PSC_IMPLEMENT_REFCOUNT()

   void Init(int screenNum, const string& rootDir);
   void playlistadd(int screenNum, const string& folder, int sort, int restSeconds);
   void playlistplay(int screenNum, const string& playlist);
   void playlistplayex(int screenNum, const string& playlist, const string& playfilename, int volume, int forceplay);
   void play(int screenNum, const string& playlist, const string& playfilename);
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
   bool GetisPlaying(int screenNum) const;
   void SetisPlaying(int screenNum, bool value);
   void SetLength(int screenNum, int stopSecs);
   void SetLoop(int screenNum, int loopState);
   void SetBackGround(int screenNum, int mode);
   void BlockPlay(int screenNum, int mode);
   void SetScreen(int screenNum);
   void SetScreenEx(int screenNum, int xpos, int ypos, int swidth, int sheight, int popup);
   int GetSN() const;
   void SetSN(int value);
   void B2SData(const string& tIndex, int value);
   const string& GetB2SFilter() const;
   void SetB2SFilter(const string& Value);
   void Show(int screeNum);
   void Hide(int screeNum);
   void B2SInit(const string& tName, const string& romName);
   void SendMSG(const string& cMsg);
   void LabelNew(int screenNum, const string& labelName, const string& fontName, int size, int color, int angle, int xAlign, int yAlign, int xMargin, int yMargin, int pageNum, bool visible);
   void LabelSet(int screenNum, const string& labelName, const string& caption, bool visible, const string& special);
   void LabelSetEx();
   void LabelShowPage(int screenNum, int pageNum, int seconds, const string& special);
   void LabelInit(int screenNum);
   const string& GetGetGame() const;
   void SetGetGame(const string& value);
   const string& GetGetRoot() const;
   void SetGetRoot(const string& value);
   void SoundAdd(const string& sname, const string& fname, int svol, double sX, double sy, const string& SP);
   void SoundPlay(const string& sname);
   void PuPSound(const string& sname, int sX, int sy, int sz, int vol, const string& SP);
   void InitPuPMenu(int Param1);
   const string& GetB2SDisplays() const;
   void setVolumeCurrent(int ScreenNum, int vol);
   int GetGameUpdate(const string& GameTitle, int Func, int FuncData, const string& Extra) const;
   // void GrabDC(int pWidth, int pHeight, const string& wintitle, VARIANT *pixels);
   string GetVersion() const;
   // void GrabDC2(int pWidth, int pHeight, const string& wintitle, SAFEARRAY **pixels);
   void playevent(int ScreenNum, const string& playlist, const string& playfilename, int volume, int priority, int playtype, int Seconds, const string& Special);
   void SetPosVideo(int ScreenNum, int StartPos, int EndPos, int Mode, const string& Special);
   void PuPClose();

private:
   PUPManager& m_pupManager;
};
