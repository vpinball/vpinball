#pragma once

class PinUpPlayerPlaylist {
public:
   PinUpPlayerPlaylist();
   ~PinUpPlayerPlaylist();

   static PinUpPlayerPlaylist* CreateFromCSVLine(string line);

   int m_screenNum;
   string m_folder;
   string m_des;
   int m_alphaSort;
   int m_restSeconds;
   int m_volume;
   int m_priority;
};
