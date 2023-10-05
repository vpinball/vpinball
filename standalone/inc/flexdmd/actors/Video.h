#pragma once

#include "AnimatedActor.h"

class Video : public AnimatedActor 
{
public:
   Video(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop);
   ~Video();

   STDMETHOD(Seek)(single posInSeconds);

   void SetVisible(bool visible) { AnimatedActor::SetVisible(visible); OnStageStateChanged(); }
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(VP::Graphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   float m_seek;
};