#pragma once

#include "PUPManager.h"

class PUPCustomPos;
class PUPTrigger;

class PUPScreen {
public:
   ~PUPScreen();

   static PUPScreen* CreateFromCSVLine(string line);
   PUP_SCREEN_MODE GetMode() const { return m_mode; }
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetBackgroundPlaylist() const { return m_backgroundPlaylist; }
   const string& GetBackgroundFilename() const { return m_backgroundFilename; }
   bool IsTransparent() const { return m_transparent; }
   int GetVolume() const { return m_volume; }
   PUPCustomPos* GetCustomPos() { return m_pCustomPos; }
   PUPTrigger* GetTrigger(const string& szTrigger);
   void SetTrigger(PUPTrigger* pTrigger);
   string ToString() const;

private:
   PUPScreen();

   PUP_SCREEN_MODE m_mode;
   int m_screenNum;
   string m_screenDes;
   string m_backgroundPlaylist;
   string m_backgroundFilename;
   bool m_transparent;
   int m_volume;
   PUPCustomPos* m_pCustomPos;
   std::map<string, PUPTrigger*> m_triggerMap;
};

