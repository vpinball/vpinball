// license:GPLv3+

#include "PUPCustomPos.h"

namespace PUP {

/*
   First number is always the screen. 1,22.5,45.3,55.1,54.7 [pupid # source screen,xpos,ypos,width,height] values in % of pupdisplay.
   Some packs add a 6th (and beyond) value (seen in PinUp Player). We don't know what it does and don't
   implement it yet, but it is valid input, so the trailing parts are ignored rather than rejected.
   TODO implement the trailing value(s)
*/

std::unique_ptr<PUPCustomPos> PUPCustomPos::CreateFromCSV(const string& line)
{
   if (line.empty())
      return nullptr;
   vector<string> parts = parse_csv_line(line);
   if (parts.size() < 5){
      LOGE(std::format("Expected 5 parts for CustomPos, got {}: {}", parts.size(), line));
      return nullptr;
   }
   if (parts.size() > 5){
      LOGW(std::format("Ignoring trailing parts for CustomPos (6th and beyond not yet implemented), handling 5 of {} parts: {}", parts.size(), line));
   }

   std::unique_ptr<PUPCustomPos> pCustomPos = std::make_unique<PUPCustomPos>();

   pCustomPos->m_sourceScreen = string_to_int(parts[0], 0);
   pCustomPos->m_frect =
   {
       string_to_float(parts[1], 0.0f),
       string_to_float(parts[2], 0.0f),
       string_to_float(parts[3], 0.0f),
       string_to_float(parts[4], 0.0f)
   };

   return pCustomPos;
}

SDL_Rect PUPCustomPos::ScaledRect(int w, int h) const
{
   return {
      (int)((m_frect.x / 100.f) * (float)w),
      (int)((m_frect.y / 100.f) * (float)h),
      (int)((m_frect.w / 100.f) * (float)w),
      (int)((m_frect.h / 100.f) * (float)h)
   };
}

string PUPCustomPos::ToString() const
{
   return "sourceScreen=" + std::to_string(m_sourceScreen) +
      ", frect={" +
      std::to_string(m_frect.x) + ", " +
      std::to_string(m_frect.y) + ", " +
      std::to_string(m_frect.w) + ", " +
      std::to_string(m_frect.h) + '}';
}

}
