#pragma once

#include "AnimatedActor.h"

class Video : public AnimatedActor 
{
public:
   Video(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop);
   ~Video();

   STDMETHOD(Seek)(single posInSeconds);

   virtual void SetVisible(bool visible) { AnimatedActor::SetVisible(visible); OnStageStateChanged(); }

   virtual void Rewind();
   virtual void ReadNextFrame();
   virtual void Draw(Graphics* graphics);
   virtual void OnStageStateChanged();

private:
   float m_seek;
};