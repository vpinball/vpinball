#pragma once

class PUPPlaylist {
public:
   PUPPlaylist();
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSVLine(string line);

   int m_screenNum;
   string m_folder;
   string m_des;
   int m_alphaSort;
   int m_restSeconds;
   int m_volume;
   int m_priority;
};
