// license:GPLv3+

#pragma once

#include "common.h"
#include <future>

namespace PUP {

class PUPImage
{
public:
   PUPImage();
   ~PUPImage();

   const std::filesystem::path& GetFile() const { return m_file; }

   void Load(const std::filesystem::path& szFile);
   void Clear();
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, float alpha = 1.f);

private:
   using SurfacePtr = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>;

   std::filesystem::path m_file;
   SurfacePtr m_pSurface;
   VPXTexture m_pTexture = nullptr;
   std::future<SurfacePtr> m_pendingSurface;
   vector<std::future<SurfacePtr>> m_abandonedLoads;
   void AbandonPendingLoad();
};

}
