#pragma once

#include "PUPManager.h"

namespace PUP {

class PUPCustomPos final {
public:
   PUPCustomPos() = default;
   ~PUPCustomPos() = default;

   static std::unique_ptr<PUPCustomPos> CreateFromCSV(const string& line);

   int GetSourceScreen() const { return m_sourceScreen; }
   SDL_Rect ScaledRect(int w, int h) const;
   string ToString() const;

private:
   int m_sourceScreen = 0;
   SDL_FRect m_frect { 0.f };
};

}