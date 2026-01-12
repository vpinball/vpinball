// license:GPLv3+

#pragma once

class TableDB final
{
public:
   TableDB() { }

   void Load();
   int GetBestSizeMatch(float width, float height, float topGlassHeight, float bottomGlassHeight = -1.f, int yearHint = -1) const;

   struct Entry
   {
      string name;
      float width;
      float height;
      float glassBottom;
      float glassTop;
      string comment;
      int minYear = 0;
      int maxYear = 9999;
   };

   vector<Entry> m_data;
};
