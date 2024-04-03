#pragma once

class PUPScreen {
public:
   PUPScreen();
   ~PUPScreen();

   static PUPScreen* CreateFromCSVLine(string line);

   int m_screenNum;
   string m_screenDes;
   string m_playList;
   string m_playFile;
   string m_loopit;
   int m_active;
   int m_priority;
   string m_customPos;
};

