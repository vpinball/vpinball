// license:GPLv3+

#include "core/stdafx.h"
#include "TableDB.h"
#include <charconv>
#include <iostream>
#include <fstream>

// Loads database file
void TableDB::Load()
{
   m_data.clear();
   std::ifstream dbFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "TableSizes.csv"));
   while (dbFile.good())
   {
      // Simple CSV parsing
      std::stringstream ss;
      bool inQuotes = false;
      std::vector<std::string> fields;
      while (dbFile.good())
      {
         const char c = (char)dbFile.get();
         if (!inQuotes && c == '"')
            inQuotes = true;
         else if (inQuotes && c == '"')
         {
            if (dbFile.peek() == '"') // 2 consecutive quotes resolve to 1
               ss << (char)dbFile.get();
            else
               inQuotes = false;
         }
         else if (!inQuotes && c == ',')
         {
            fields.push_back(ss.str());
            ss.str(string());
         }
         else if (!inQuotes && (c == '\r' || c == '\n'))
         {
            if (dbFile.peek() == '\n')
               dbFile.get();
            fields.push_back(ss.str());
            break;
         }
         else
            ss << c;
      }

      // Parse fields
      if (fields.size() >= 8)
      {
         Entry dim;
         dim.name = fields[0];
         if (!try_parse_float(fields[1], dim.width))
            dim.width = 20.25f;
         if (!try_parse_float(fields[2], dim.height))
            dim.height = 46.00f;
         if (!try_parse_float(fields[3], dim.glassBottom))
            dim.glassBottom = 2.375f;
         if (!try_parse_float(fields[4], dim.glassTop))
            dim.glassTop = 2.375f;
         if (!try_parse_int(fields[5], dim.minYear))
            dim.minYear = 0;
         if (!try_parse_int(fields[6], dim.maxYear))
            dim.maxYear = 9999;
         dim.comment = fields[7];
         m_data.push_back(dim);
      }
   }
   if (!m_data.empty())
      m_data.erase(m_data.begin()); // Remove header line
   dbFile.close();
}

// Find the nearest match in the database, based on given size, search being mainly weighted on the playfield size
// since it is always well defined by table author, then top glass height (sometime right, but has been there for a 
// long time), then bottom glass height if given (added in 10.8)
int TableDB::GetBestSizeMatch(float width, float height, float topGlassHeight, float bottomGlassHeight, int yearHint) const
{
   int selection = -1;
   float bestErr = FLT_MAX;
   for (unsigned int i = 0; i < (unsigned int)m_data.size(); i++)
   {
      float wErr = VPUTOINCHES(width) - m_data[i].width;
      float hErr = VPUTOINCHES(height) - m_data[i].height;
      float gthErr = VPUTOINCHES(topGlassHeight) - m_data[i].glassTop;
      float gbhErr = VPUTOINCHES(bottomGlassHeight) - m_data[i].glassBottom;
      float err = bottomGlassHeight < 0 ? wErr * wErr + hErr * hErr + 0.01f * gthErr * gthErr
                                        : wErr * wErr + hErr * hErr + 0.01f * gthErr * gthErr + 0.00001f * gbhErr * gbhErr;

      if (yearHint >= 1900 && (yearHint < m_data[i].minYear || yearHint > m_data[i].maxYear))
         err *= 10.f; // Penalize entries not matching the year hint

      if (err < bestErr)
      {
         bestErr = err;
         selection = i;
      }
   }
   return selection;
}
