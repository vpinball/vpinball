#pragma once

#include "common.h"
#include "pole/pole.h"
#include <map>

class VPXFile
{
public:
   VPXFile(const string& path);
   ~VPXFile();

   const string& GetImportFile(const string& path);
   std::pair<unsigned int, uint8_t*> GetImage(const string& path);

private:
   std::pair<unsigned int, uint8_t*> ReadImage(const string& path, bool nameOnly);

   struct Entry
   {
      string path;
      string file;
      string name;
   };

   std::map<string, Entry> m_images;
   const string m_path;
   POLE::Storage* m_pStorage;
};