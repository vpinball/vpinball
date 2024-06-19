#pragma once

#include "PUPManager.h"

typedef enum
{
   PUP_PLAYLIST_FUNCTION_DEFAULT,
   PUP_PLAYLIST_FUNCTION_OVERLAYS,         // you put 32bit pngs in there and when you play them it will set them as current overlay frame.
   PUP_PLAYLIST_FUNCTION_FRAMES,           // you put 32bit pngs in there and when you play them it will set the background png (good for performance)
   PUP_PLAYLIST_FUNCTION_ALPHAS,           // you put 32bit pngs in there and when you play them it will set them as current overlay frame with alpha blending (performance, v1.4.5+)
   PUP_PLAYLIST_FUNCTION_SHAPES            // you put 24bit bmps files in there and the pixel color (0,0) will be used as a mask to make a see=through shape.
} PUP_PLAYLIST_FUNCTION;

const char* PUP_PLAYLIST_FUNCTION_TO_STRING(PUP_PLAYLIST_FUNCTION value);

class PUPPlaylist
{
public:
   PUPPlaylist(const string& szFolder, const string& szDescription, bool randomize, int restSeconds, float volume, int priority);
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSV(const string& line);
   const string& GetFolder() const { return m_szFolder; }
   const string& GetDescription() const { return m_szDescription; }
   bool IsRandomize() const { return m_randomize; }
   int GetRestSeconds() const { return m_restSeconds; }
   float GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   PUP_PLAYLIST_FUNCTION GetFunction() const { return m_function; }
   const string& GetPlayFile(const string& szFilename);
   const string& GetNextPlayFile();
   string GetPlayFilePath(const string& szFilename);
   string ToString() const;

private:
   string m_szFolder;
   string m_szDescription;
   bool m_randomize;
   int m_restSeconds;
   float m_volume;
   int m_priority;
   PUP_PLAYLIST_FUNCTION m_function;
   int m_lastIndex;
   vector<string> m_files;
   std::map<string, string> m_fileMap;
   string m_szBasePath;
};
