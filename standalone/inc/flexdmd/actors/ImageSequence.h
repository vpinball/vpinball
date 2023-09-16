#pragma once

#include "AnimatedActor.h"
#include "Image.h"

class ImageSequence : public AnimatedActor 
{
public:
   ImageSequence(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop);
   ~ImageSequence();

   void SetFPS(int fps) { m_fps = fps; }

   virtual single GetPrefWidth() { return m_frames[0]->GetWidth(); };
   virtual single GetPrefHeight() { return m_frames[0]->GetHeight(); };
   virtual float GetLength() { return m_frames.size() * GetFrameDuration(); };

   virtual void Rewind();
   virtual void ReadNextFrame();
   virtual void Draw(Graphics* graphics);
   virtual void OnStageStateChanged();

private:
   int m_fps;
   int m_frame;
   vector<Image*> m_frames;
};