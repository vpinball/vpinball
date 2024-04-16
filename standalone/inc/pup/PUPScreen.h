#pragma once

class PUPScreen {
public:
   PUPScreen();
   ~PUPScreen();

   static PUPScreen* CreateFromCSVLine(string line);
   int GetScreenNum() const { return m_screenNum; }
   const string& GetScreenDes() const { return m_screenDes; }
   const string& GetPlayList() const { return m_playList; }
   const string& GetPlayFile() const { return m_playFile; }
   const string& GetLoopit() const { return m_loopit; }
   int GetActive() const { return m_active; }
   int GetPriority() const { return m_priority; }
   const string& GetCustomPos() const { return m_customPos; }
   string ToString() const;

private:
   int m_screenNum;
   string m_screenDes;
   string m_playList;
   string m_playFile;
   string m_loopit;
   int m_active;
   int m_priority;
   string m_customPos;
};

