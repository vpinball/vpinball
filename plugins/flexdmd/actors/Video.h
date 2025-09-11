#pragma once

#include "AnimatedActor.h"
#include "resources/AssetManager.h"

#include "LibAv.h"

namespace Flex {

class Video final : public AnimatedActor
{
public:
   ~Video() override;

   static Video* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name, bool loop);

   void OnStageStateChanged() override;
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(Flex::SurfaceGraphics* pGraphics) override;
   float GetLength() const override { return m_length; }

private:
   Video(FlexDMD* pFlexDMD, const string& name);

   std::vector<SDL_Surface*> m_frames;
   SDL_Surface* m_pActiveFrameSurface = nullptr;
   int m_pos = 0;
   float m_frameDuration = 0.0f;
   float m_length;

   const LibAV::LibAV& m_libAv;
};

}