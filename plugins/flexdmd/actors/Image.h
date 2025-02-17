#pragma once

#include "Actor.h"
#include "VPXPlugin.h"

class AssetManager;
class AssetSrc;
class Bitmap;

class Image final : public Actor
{
public:
   ~Image();

   static Image* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& image, const string& name);

   ActorType GetType() const override { return AT_Image; }

   Bitmap* GetBitmap();
   void SetBitmap(Bitmap* pBitmap);
   Scaling GetScaling() const { return m_scaling; }
   void SetScaling(Scaling scaling) { m_scaling = scaling; }
   Alignment GetAlignment() const { return m_alignment; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }

   void Draw(VP::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   Image(FlexDMD* pFlexDMD, const string& name);

   Scaling m_scaling;
   Alignment m_alignment;
   AssetManager* m_pAssetManager;
   AssetSrc* m_pSrc;
   Bitmap* m_pBitmap;
};
