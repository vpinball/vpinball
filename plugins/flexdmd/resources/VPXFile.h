#pragma once

#include "common.h"
#include "pole/pole.h"
#include <unordered_map>

class VPXFile final
{
public:
   VPXFile(const string& path);
   ~VPXFile();

   string GetImportFile(const string& path);
   std::pair<unsigned int, uint8_t*> GetImage(const string& path);

private:
   std::pair<unsigned int, uint8_t*> ReadImage(const string& path, bool nameOnly);

   struct Entry
   {
      string path;
      string file;
      string name;
   };

   std::unordered_map<string, Entry> m_images;
   const string m_path;
   POLE::Storage* m_pStorage;
};
