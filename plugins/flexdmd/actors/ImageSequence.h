#pragma once

#include "resources/AssetManager.h"
#include "AnimatedActor.h"
#include "Image.h"

class ImageSequence final : public AnimatedActor
{
public:
   ~ImageSequence();

   static ImageSequence* Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& paths, const string& name, int fps, bool loop);

   void SetFPS(int fps) { m_fps = fps; }
   float GetPrefWidth() const override { return m_frames[0]->GetWidth(); };
   float GetPrefHeight() const override { return m_frames[0]->GetHeight(); };
   float GetLength() const override { return (float)m_frames.size() * GetFrameDuration(); };
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
