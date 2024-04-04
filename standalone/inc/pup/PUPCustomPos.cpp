#include "stdafx.h"

#include "PUPCustomPos.h"

/*
   First number is always the screen. 1,22.5,45.3,55.1,54.7 [pupid # source screen,xpos,ypos,width,height] values in % of pupdisplay. 
*/

PUPCustomPos* PUPCustomPos::CreateFromCSVLine(string line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 5)
      return nullptr;

   PUPCustomPos* pCustomPos = new PUPCustomPos();

   pCustomPos->m_sourceScreen = string_to_int(parts[0], 0);
   pCustomPos->m_xpos = string_to_float(parts[1], 0.0f);
   pCustomPos->m_ypos = string_to_float(parts[2], 0.0f);
   pCustomPos->m_width = string_to_float(parts[3], 0.0f);
   pCustomPos->m_height = string_to_float(parts[4], 0.0f);

   return pCustomPos;
}

string PUPCustomPos::ToString() const
{
   return "m_sourceScreen=" + std::to_string(m_sourceScreen) 
      + ", m_xpos=" + std::to_string(m_xpos) 
      + ", m_ypos=" + std::to_string(m_ypos)
      + ", m_width=" + std::to_string(m_width) 
      + ", m_height=" + std::to_string(m_width);
}
