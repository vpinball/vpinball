#pragma once

#include "AnimatedActor.h"

namespace Flex {

class Video final : public AnimatedActor 
{
public:
   ~Video() override;

   static Video* Create(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop);

   void Seek(float posInSeconds) override;
   void Advance(float delta) override;
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(Flex::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

   float GetLength() const override { return 0.f; }

   void SetVisible(bool visible) override { AnimatedActor::SetVisible(visible); OnStageStateChanged(); }

private:
   Video(FlexDMD* pFlexDMD, const string& name);

   float m_seek = 0;
};

}