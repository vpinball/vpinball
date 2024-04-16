#pragma once

#include "PUPScreen.h"

class PUPPlaylist {
public:
   PUPPlaylist();
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSVLine(const string& basePath, const string& line);

   const string& GetFolder() const { return m_folder; }
   const string& GetDescription() const { return m_des; }
   int GetAlphaSort() const { return m_alphaSort; }
   int GetRestSeconds() const { return m_restSeconds; }
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   const string& GetPlayFile();
   string ToString() const;

private:
   string m_folder;
   string m_des;
   int m_alphaSort;
   int m_restSeconds;
   int m_volume;
   int m_priority;

   int m_lastIndex;
   vector<string> m_files;
};
