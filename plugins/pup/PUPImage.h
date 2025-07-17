#pragma once

#include "common.h"

namespace PUP {
   
class PUPImage
{
public:
   PUPImage();
   ~PUPImage();

   const string& GetFile() const { return m_file; }

   void Load(const string& szFile);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect);

private:
   string m_file;
   std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> m_pSurface;
   VPXTexture m_pTexture = nullptr;
};

}