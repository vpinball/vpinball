#pragma once

#include "AnimatedActor.h"
#include "Image.h"

class ImageSequence : public AnimatedActor 
{
public:
   ~ImageSequence();

   static ImageSequence* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop);

   void SetFPS(int fps) { m_fps = fps; }
   single GetPrefWidth() override { return m_frames[0]->GetWidth(); };
   single GetPrefHeight() override { return m_frames[0]->GetHeight(); };
   float GetLength() override { return m_frames.size() * GetFrameDuration(); };
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   ImageSequence(FlexDMD* pFlexDMD, const string& name);

   int m_fps;
   int m_frame;
   vector<Image*> m_frames;
};