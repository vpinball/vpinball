#pragma once

#include "AnimatedActor.h"
#include "../AssetManager.h"

class GIFImage : public AnimatedActor 
{
public:
   GIFImage(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name);
   ~GIFImage();

   virtual void Rewind();
   virtual void ReadNextFrame();
   virtual void UpdateFrame();
   virtual void Draw(Graphics* graphics);
   virtual void OnStageStateChanged();

private:
   int m_pos;
   AssetManager* m_pAssetManager;
   AssetSrc* m_pSrc;
   Bitmap* m_pBitmap;
   SDL_Surface* m_pActiveFrameSurface;
};