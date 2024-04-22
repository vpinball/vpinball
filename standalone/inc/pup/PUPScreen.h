#pragma once

#include "PUPManager.h"

class PUPTrigger;

class PUPScreen {
public:
   PUPScreen();
   ~PUPScreen();

   static PUPScreen* CreateFromCSVLine(string line);
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetPlayList() const { return m_playList; }
   const string& GetPlayFile() const { return m_playFile; }
   bool IsLoopit() const { return m_loopit; }
   PUP_SCREEN_ACTIVE GetActive() const { return m_active; }
   int GetPriority() const { return m_priority; }
   const string& GetCustomPos() const { return m_customPos; }
   PUPTrigger* GetTrigger(const string& szTrigger);
   void SetTrigger(PUPTrigger* pTrigger);
   string ToString() const;

private:
   int m_screenNum;
   string m_screenDes;
   string m_playList;
   string m_playFile;
   bool m_loopit;
   PUP_SCREEN_ACTIVE m_active;
   int m_priority;
   string m_customPos;
   std::map<string, PUPTrigger*> m_triggerMap;
};

