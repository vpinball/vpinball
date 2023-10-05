#pragma once

#include "AnimatedActor.h"
#include "../AssetManager.h"

class GIFImage : public AnimatedActor 
{
public:
   GIFImage(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name);
   ~GIFImage();

   void Rewind() override;
   void ReadNextFrame() override;
   void UpdateFrame();
   void Draw(VP::Graphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   int m_pos;
   AssetManager* m_pAssetManager;
   AssetSrc* m_pSrc;
   Bitmap* m_pBitmap;
   SDL_Surface* m_pActiveFrameSurface;
};