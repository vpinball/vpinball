#pragma once

#include "PUPManager.h"

class PUPPlaylist {
public:
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSV(const string& line);
   const string& GetFolder() const { return m_szFolder; }
   const string& GetDescription() const { return m_szDescription; }
   bool IsRandomize() const { return m_randomize; }
   int GetRestSeconds() const { return m_restSeconds; }
   int GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   PUP_PLAYLIST_FUNCTION GetFunction() const { return m_function; }
   const string& GetFolderPath() const { return m_szFolderPath; }
   string ToString() const;
   string GetPlayFile(const string& szPlayFile);

private:
   PUPPlaylist();

   string m_szFolder;
   string m_szDescription;
   bool m_randomize;
   int m_restSeconds;
   int m_volume;
   int m_priority;
   PUP_PLAYLIST_FUNCTION m_function;
   int m_lastIndex;
   vector<string> m_files;
   string m_szFolderPath;
   std::mutex m_mutex;
};
