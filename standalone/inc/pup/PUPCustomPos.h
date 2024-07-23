#pragma once

#include "PUPManager.h"

class PUPCustomPos {
public:
   ~PUPCustomPos() {}

   static PUPCustomPos* CreateFromCSV(string line);
   const int GetSourceScreen() const { return m_sourceScreen; }
   SDL_Rect ScaledRect(int w, int h);
   string ToString() const;

private:
   PUPCustomPos() {}

   int m_sourceScreen;
   SDL_FRect m_frect;
};

