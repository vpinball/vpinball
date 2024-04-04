#include "core/stdafx.h"
#include "PUPPinDisplay.h"

#include <iostream>
#include <fstream>
#include <sstream>

PUPPinDisplay::PUPPinDisplay()
{
}

PUPPinDisplay::~PUPPinDisplay()
{
}

STDMETHODIMP PUPPinDisplay::Init(LONG ScreenNum, BSTR RootDir)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistadd(LONG ScreenNum, BSTR folder, LONG sort, LONG restSeconds)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplay(LONG ScreenNum, BSTR playlist){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playlistplayex(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::play(LONG ScreenNum, BSTR playlist, BSTR playfilename){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setWidth(LONG ScreenNum, LONG width){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setHeight(LONG ScreenNum, LONG Height){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosX(LONG ScreenNum, LONG Posx){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setPosY(LONG ScreenNum, LONG PosY){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setAspect(LONG ScreenNum, LONG aspectWide, LONG aspectHigh){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setVolume(LONG ScreenNum, LONG vol){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playpause(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playresume(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playstop(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::CloseApp(){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_isPlaying(LONG ScreenNum, LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_isPlaying(LONG ScreenNum, LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLength(LONG ScreenNum, LONG StopSecs){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetLoop(LONG ScreenNum, LONG LoopState){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetBackGround(LONG ScreenNum, LONG Mode)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::BlockPlay(LONG ScreenNum, LONG Mode){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreen(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetScreenEx(LONG ScreenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_SN(LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_SN(LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::B2SData(BSTR tIndex, int Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_B2SFilter(BSTR *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_B2SFilter(BSTR Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::Show(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::Hide(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
   string szRomName = MakeString(RomName);

   string szPath = g_pvp->m_currentTablePath + szRomName + PATH_SEPARATOR_CHAR;

   if (!DirExists(szPath)) {
      PLOGE.printf("Directory does not exist: path=%s", szPath.c_str());
      return S_OK;
   }

   // Screens

   string screensPath = szPath + "screens.pup";
   std::ifstream screensFile;
   screensFile.open(screensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;

         PUPScreen* pScreen = PUPScreen::CreateFromCSVLine(line);

         if (pScreen)
            m_screens.push_back(pScreen);
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", screensPath.c_str(), m_screens.size());
   }
   else {
      PLOGE.printf("Unable to load %s", screensPath.c_str());
   }

   // Playlists

   string playlistsPath = szPath + "playlists.pup";
   std::ifstream playlistsFile;
   playlistsFile.open(playlistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;

         PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSVLine(szPath, line);

         if (pPlaylist)
            m_playlists.push_back(pPlaylist);
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", playlistsPath.c_str(), m_playlists.size());
   }
   else {
      PLOGE.printf("Unable to load %s", playlistsPath.c_str());
   }

   // Triggers

   string triggersPath = szPath + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(triggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PUPTrigger* pTrigger = PUPTrigger::CreateFromCSVLine(line, m_screens, m_playlists);

         if (pTrigger)
            m_triggers.push_back(pTrigger);
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", triggersPath.c_str(), m_triggers.size());
   }
   else {
      PLOGE.printf("Unable to load %s", triggersPath.c_str());
   }

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SendMSG(BSTR cMSG)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelNew(LONG ScreenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xMargin, LONG yMargin, LONG PageNum, LONG Visible)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelSet(LONG ScreenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelSetEx(){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelShowPage(LONG ScreenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::LabelInit(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GetGame(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_GetGame(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GetRoot(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::put_GetRoot(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SoundAdd(BSTR sname, BSTR fname, LONG svol, double sX, double sy, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SoundPlay(BSTR sname)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::PuPSound(BSTR sname, LONG sX, LONG sy, LONG sz, LONG vol, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::InitPuPMenu(LONG Param1)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_B2SDisplays(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::setVolumeCurrent(LONG ScreenNum, LONG vol)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::get_GameUpdate(BSTR GameTitle, LONG Func, LONG FuncData, BSTR Extra, LONG *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::GrabDC(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels)
{
   PLOGW << "Not implemented";

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
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::playevent(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG priority, LONG playtype, LONG Seconds, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::SetPosVideo(LONG ScreenNum, LONG StartPos, LONG EndPos, LONG Mode, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PUPPinDisplay::PuPClose()
{
   PLOGW << "Not implemented";

   return S_OK;
}