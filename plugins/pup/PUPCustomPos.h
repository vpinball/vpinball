#pragma once

#include "PUPManager.h"

namespace PUP {

class PUPCustomPos final {
public:
   ~PUPCustomPos() {}

   static PUPCustomPos* CreateFromCSV(const string& line);
   int GetSourceScreen() const { return m_sourceScreen; }
   SDL_Rect ScaledRect(int w, int h) const;
   string ToString() const;

private:
   PUPCustomPos() {}

   int m_sourceScreen;
   SDL_FRect m_frect;
};

}