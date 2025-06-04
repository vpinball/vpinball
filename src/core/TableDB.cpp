// license:GPLv3+

#include "core/stdafx.h"
#include "TableDB.h"
#include <charconv>
#include <iostream>
#include <fstream>

TableDB::TableDB()
{
}

// Loads database file
void TableDB::Load()
{
   m_data.clear();
   std::ifstream dbFile(g_pvp->m_myPath + "assets" + PATH_SEPARATOR_CHAR + "TableSizes.csv");
   while (dbFile.good())
   {
      // Simple CSV parsing
      std::stringstream ss;
      bool inQuotes = false;
      std::vector<std::string> fields;
      while (dbFile.good())
      {
         char c = (char)dbFile.get();
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
      if (fields.size() >= 6)
      {
         Entry dim;
         dim.name = fields[0];
         float result;
         dim.width = (std::from_chars(fields[1].c_str(), fields[1].c_str() + fields[1].length(), result).ec == std::errc{})
            ? result
            : 20.25f; // Default width if parsing fails
         dim.height = (std::from_chars(fields[2].c_str(), fields[2].c_str() + fields[2].length(), result).ec == std::errc{})
            ? result
            : 46.f; // Default height if parsing fails
         dim.glassBottom = (std::from_chars(fields[3].c_str(), fields[3].c_str() + fields[3].length(), result).ec == std::errc{})
            ? result
            : 3.f; // Default glassBottom if parsing fails
         dim.glassTop = (std::from_chars(fields[4].c_str(), fields[4].c_str() + fields[4].length(), result).ec == std::errc{})
            ? result
            : 8.5f; // Default glassTop if parsing fails
         dim.comment = fields[5];
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
int TableDB::GetBestSizeMatch(float width, float height, float topGlassHeight, float bottomGlassHeight) const
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
      if (err < bestErr)
      {
         bestErr = err;
         selection = i;
      }
   }
   return selection;
}
