#pragma once

#include "AnimatedActor.h"

class Video : public AnimatedActor 
{
public:
   ~Video();

   static Video* Create(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop);

   STDMETHOD(Seek)(single posInSeconds);

   void SetVisible(bool visible) { AnimatedActor::SetVisible(visible); OnStageStateChanged(); }
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   Video(FlexDMD* pFlexDMD, const string& name);

   float m_seek;
};