#pragma once

#include "AnimatedActor.h"
#include "Image.h"

class ImageSequence : public AnimatedActor 
{
public:
   ImageSequence(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop);
   ~ImageSequence();

   void SetFPS(int fps) { m_fps = fps; }
   single GetPrefWidth() override { return m_frames[0]->GetWidth(); };
   single GetPrefHeight() override { return m_frames[0]->GetHeight(); };
   float GetLength() override { return m_frames.size() * GetFrameDuration(); };
   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(VP::Graphics* pGraphics) override;
   void OnStageStateChanged() override;

private:
   int m_fps;
   int m_frame;
   vector<Image*> m_frames;
};