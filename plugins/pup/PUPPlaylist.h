#pragma once

#include "PUPManager.h"

namespace PUP {

class PUPPlaylist final
{
public:
   PUPPlaylist(PUPManager* manager, const string& szFolder, const string& szDescription, bool randomize, int restSeconds, float volume, int priority);
   ~PUPPlaylist();

   static PUPPlaylist* CreateFromCSV(PUPManager* manager, const string& line);

   enum class Function
   {
      Default,
      Overlays, // you put 32bit pngs in there and when you play them it will set them as current overlay frame.
      Frames, // you put 32bit pngs in there and when you play them it will set the background png (good for performance)
      Alphas, // you put 32bit pngs in there and when you play them it will set them as current overlay frame with alpha blending (performance, v1.4.5+)
      Shapes // you put 24bit bmps files in there and the pixel color (0,0) will be used as a mask to make a see=through shape.
   };

   const string& GetFolder() const { return m_szFolder; }
   const string& GetDescription() const { return m_szDescription; }
   bool IsRandomize() const { return m_randomize; }
   int GetRestSeconds() const { return m_restSeconds; }
   float GetVolume() const { return m_volume; }
   int GetPriority() const { return m_priority; }
   PUPPlaylist::Function GetFunction() const { return m_function; }
   const string& GetPlayFile(const string& szFilename);
   const string& GetNextPlayFile();
   string GetPlayFilePath(const string& szFilename);
   string ToString() const;

   static const string& ToString(Function value);

private:
   string m_szFolder;
   string m_szDescription;
   bool m_randomize;
   int m_restSeconds;
   float m_volume;
   int m_priority;
   PUPPlaylist::Function m_function;
   int m_lastIndex;
   vector<string> m_files;
   ankerl::unordered_dense::map<string, string> m_fileMap;
   string m_szBasePath;
};

}
