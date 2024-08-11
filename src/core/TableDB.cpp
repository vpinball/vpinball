// license:GPLv3+

#include "core/stdafx.h"
#include "TableDB.h"
#include <iostream>
#include <fstream>

TableDB::TableDB()
{
}

// Loads database file
void TableDB::Load()
{
   m_data.clear();
   std::ifstream dbFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "TableSizes.csv");
   while (dbFile.good())
   {
      // Simple CSV parsing
      std::stringstream ss;
      bool inQuotes = false;
      std::vector<std::string> fields;
      while (dbFile.good())
      {
         char c = dbFile.get();
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
            ss.str("");
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
         try
         {
            dim.width = std::stof(fields[1]);
         }
         catch (const std::exception&)
         {
            dim.width = 20.25f;
         }
         try
         {
            dim.height = std::stof(fields[2]);
         }
         catch (const std::exception&)
         {
            dim.height = 46.f;
         }
         try
         {
            dim.glassBottom = std::stof(fields[3]);
         }
         catch (const std::exception&)
         {
            dim.glassBottom = 3.f;
         }
         try
         {
            dim.glassTop = std::stof(fields[4]);
         }
         catch (const std::exception&)
         {
            dim.glassTop = 8.5f;
         }
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
int TableDB::GetBestSizeMatch(float width, float height, float topGlassHeight, float bottomGlassHeight)
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
