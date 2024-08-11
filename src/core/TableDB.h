// license:GPLv3+

#pragma once

class TableDB final
{
public:
   TableDB();

   void Load();
   int GetBestSizeMatch(float width, float height, float topGlassHeight, float bottomGlassHeight = -1.f);


   struct Entry
   {
      string name;
      float width;
      float height;
      float glassBottom;
      float glassTop;
      string comment;
   };

   vector<Entry> m_data;
};
