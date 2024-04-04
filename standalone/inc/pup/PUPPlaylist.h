#pragma once

#include "PUPManager.h"

class PUPPlaylist {
public:
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSVLine(const string& basePath, const string& line);
   const string& GetFolder() const { return m_folder; }
   const string& GetDescription() const { return m_description; }
   bool IsRandomize() const { return m_randomize; }
   int GetRestSeconds() const { return m_restSeconds; }
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   PUP_PLAYLIST_FUNCTION GetFunction() const { return m_function; }
   const string& GetPlayFile();
   string ToString() const;

private:
   PUPPlaylist();

   string m_folder;
   string m_description;
   bool m_randomize;
   int m_restSeconds;
   int m_volume;
   int m_priority;
   PUP_PLAYLIST_FUNCTION m_function;
   int m_lastIndex;
   vector<string> m_files;
};
