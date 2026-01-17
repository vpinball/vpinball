// license:GPLv3+

#pragma once

#include "common.h"

namespace PUP {

class PUPImage
{
public:
   PUPImage();
   ~PUPImage();

   const std::filesystem::path& GetFile() const { return m_file; }

   void Load(const std::filesystem::path& szFile);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect);

private:
   std::filesystem::path m_file;
   std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> m_pSurface;
   VPXTexture m_pTexture = nullptr;
};

}
