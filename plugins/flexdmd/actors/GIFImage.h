#pragma once

#include "AnimatedActor.h"
#include "resources/AssetManager.h"

namespace Flex {

class GIFImage final : public AnimatedActor
{
public:
   ~GIFImage() override;

   static GIFImage* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name);

   void Rewind() override;
   void ReadNextFrame() override;
   void UpdateFrame();
   void Draw(Flex::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;
   float GetLength() const override { return m_pBitmap ? static_cast<float>(m_pBitmap->GetLength() / 1000.0) : 0.f; }

private:
   GIFImage(FlexDMD* pFlexDMD, const string& name);

   int m_pos;
   AssetManager* m_pAssetManager;
   AssetSrc* m_pSrc;
   Bitmap* m_pBitmap;
   SDL_Surface* m_pActiveFrameSurface;
};

}
