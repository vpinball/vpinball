// license:GPLv3+

#pragma once

#include "PUPManager.h"

namespace PUP {

class PUPCustomPos final {
public:
   PUPCustomPos() = default;
   ~PUPCustomPos() = default;

   static std::unique_ptr<PUPCustomPos> CreateFromCSV(const string& line);

   int GetSourceScreen() const { return m_sourceScreen; }
   int GetRotation() const { return m_rotation; }
   SDL_Rect ScaledRect(int w, int h) const;
   string ToString() const;

private:
   int m_sourceScreen = 0;
   int m_rotation = 0;
   SDL_FRect m_frect { };
};

}
