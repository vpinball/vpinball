#pragma once

#include "common.h"

namespace PUP {
   
class PUPImage
{
public:
   PUPImage() = default;
   ~PUPImage();

   void Load(const string& szFile);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect);

private:
   SDL_Surface* m_pSurface = nullptr;
   VPXTexture m_pTexture = nullptr;
};

}