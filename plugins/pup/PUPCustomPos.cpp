// license:GPLv3+

#include "PUPCustomPos.h"

namespace PUP {

/*
   First number is always the screen. 1,22.5,45.3,55.1,54.7 [pupid # source screen,xpos,ypos,width,height] values in % of pupdisplay.
   Source screens 99 (overlay only), 100 (absolute desktop pixels) and 101 (percent of the display's own window) are PinUP specials.
   A 6th value is a flag: 1, 2, 3 rotate by 90, 180, 270 degrees, 5 resets the rotation, 4, 6 and 7 are window flags.
   With 7 values, the 6th and 7th are an aspect ratio pair instead.
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
   const int sourceScreen = string_to_int(parts[0], 0);
   if (sourceScreen >= 99) {
      NOT_IMPLEMENTED(std::format("CustomPos source {} (overlay, absolute or self relative placement) is not implemented: {}", sourceScreen, line));
      return nullptr;
   }

   std::unique_ptr<PUPCustomPos> pCustomPos = std::make_unique<PUPCustomPos>();

   pCustomPos->m_sourceScreen = sourceScreen;
   if (parts.size() == 6) {
      switch (string_to_int(parts[5], 0)) {
      case 1: pCustomPos->m_rotation = 90; break;
      case 2: pCustomPos->m_rotation = 180; break;
      case 3: pCustomPos->m_rotation = 270; break;
      default: break;
      }
      if (pCustomPos->m_rotation != 0)
         NOT_IMPLEMENTED(std::format("CustomPos rotation {} is not implemented: {}", pCustomPos->m_rotation, line));
   }
   else if (parts.size() >= 7) {
      NOT_IMPLEMENTED(std::format("CustomPos aspect ratio {}:{} is not implemented: {}", parts[5], parts[6], line));
   }
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
      ", rotation=" + std::to_string(m_rotation) +
      ", frect={" +
      std::to_string(m_frect.x) + ", " +
      std::to_string(m_frect.y) + ", " +
      std::to_string(m_frect.w) + ", " +
      std::to_string(m_frect.h) + '}';
}

}
