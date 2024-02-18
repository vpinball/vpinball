#include "core/stdafx.h"
#include "PinUpPlayerPinDisplay.h"

#include <iostream>
#include <fstream>
#include <sstream>

PinUpPlayerPinDisplay::PinUpPlayerPinDisplay()
{
}

PinUpPlayerPinDisplay::~PinUpPlayerPinDisplay()
{
}

STDMETHODIMP PinUpPlayerPinDisplay::Init(LONG ScreenNum, BSTR RootDir)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistadd(LONG ScreenNum, BSTR folder, LONG sort, LONG restSeconds)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistplay(LONG ScreenNum, BSTR playlist){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistplayex(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::play(LONG ScreenNum, BSTR playlist, BSTR playfilename){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setWidth(LONG ScreenNum, LONG width){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setHeight(LONG ScreenNum, LONG Height){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setPosX(LONG ScreenNum, LONG Posx){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setPosY(LONG ScreenNum, LONG PosY){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setAspect(LONG ScreenNum, LONG aspectWide, LONG aspectHigh){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setVolume(LONG ScreenNum, LONG vol){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playpause(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playresume(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playstop(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::CloseApp(){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_isPlaying(LONG ScreenNum, LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_isPlaying(LONG ScreenNum, LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetLength(LONG ScreenNum, LONG StopSecs){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetLoop(LONG ScreenNum, LONG LoopState){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetBackGround(LONG ScreenNum, LONG Mode)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::BlockPlay(LONG ScreenNum, LONG Mode){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetScreen(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetScreenEx(LONG ScreenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_SN(LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_SN(LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::B2SData(BSTR tIndex, int Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_B2SFilter(BSTR *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_B2SFilter(BSTR Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::Show(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::Hide(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
   string szRomName = MakeString(RomName);

   string path = g_pvp->m_currentTablePath + szRomName;

   if (!DirExists(path)) {
      PLOGE.printf("Directory does not exist: path=%s", path.c_str());
      return S_OK;
   }

   string playlistsPath = path + PATH_SEPARATOR_CHAR + "playlists.pup";
   std::ifstream playlistsFile;
   playlistsFile.open(playlistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerPlaylist* pPlaylist = PinUpPlayerPlaylist::CreateFromCSVLine(line);

         if (pPlaylist)
            m_playlists.push_back(pPlaylist);
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", playlistsPath.c_str(), m_playlists.size());
   }
   else {
      PLOGE.printf("Unable to load %s", playlistsPath.c_str());
   }

   string triggersPath = path + PATH_SEPARATOR_CHAR + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(triggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerTrigger* pTrigger = PinUpPlayerTrigger::CreateFromCSVLine(line);

         if (pTrigger)
            m_triggers.push_back(pTrigger);
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", triggersPath.c_str(), m_triggers.size());
   }
   else {
      PLOGE.printf("Unable to load %s", triggersPath.c_str());
   }

   string screensPath = path + PATH_SEPARATOR_CHAR + "screens.pup";
   std::ifstream screensFile;
   screensFile.open(screensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerScreen* pScreen = PinUpPlayerScreen::CreateFromCSVLine(line);

         if (pScreen)
            m_screens.push_back(pScreen);
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", screensPath.c_str(), m_screens.size());
   }
   else {
      PLOGE.printf("Unable to load %s", screensPath.c_str());
   }

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SendMSG(BSTR cMSG)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelNew(LONG ScreenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xMargin, LONG yMargin, LONG PageNum, LONG Visible)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelSet(LONG ScreenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelSetEx(){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelShowPage(LONG ScreenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelInit(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GetGame(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_GetGame(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GetRoot(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_GetRoot(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SoundAdd(BSTR sname, BSTR fname, LONG svol, double sX, double sy, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SoundPlay(BSTR sname)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::PuPSound(BSTR sname, LONG sX, LONG sy, LONG sz, LONG vol, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::InitPuPMenu(LONG Param1)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_B2SDisplays(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setVolumeCurrent(LONG ScreenNum, LONG vol)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GameUpdate(BSTR GameTitle, LONG Func, LONG FuncData, BSTR Extra, LONG *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::GrabDC(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::GetVersion(BSTR *pver)
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

STDMETHODIMP PinUpPlayerPinDisplay::GrabDC2(LONG pWidth, LONG pHeight, BSTR wintitle, SAFEARRAY **pixels)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playevent(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG priority, LONG playtype, LONG Seconds, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetPosVideo(LONG ScreenNum, LONG StartPos, LONG EndPos, LONG Mode, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::PuPClose()
{
   PLOGW << "Not implemented";

   return S_OK;
}