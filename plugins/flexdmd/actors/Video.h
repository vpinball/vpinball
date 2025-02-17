#pragma once

#include "AnimatedActor.h"

class Video final : public AnimatedActor 
{
public:
   ~Video();

   static Video* Create(FlexDMD* pFlexDMD, const string& path, const string& name, bool loop);

   void SetPaused(bool paused) { m_paused = paused; }
   bool GetPaused() const { return m_paused; }
   void SetPlaySpeed(float playSpeed) { m_playSpeed = playSpeed; }
   float GetPlaySpeed() const { return m_playSpeed; }
   
   void Seek(float posInSeconds) override;

   void Rewind() override;
   void ReadNextFrame() override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;
   void OnStageStateChanged() override;

   void SetVisible(bool visible) override { AnimatedActor::SetVisible(visible); OnStageStateChanged(); }

private:
   Video(FlexDMD* pFlexDMD, const string& name);

   float m_seek = 0;
   float m_playSpeed = 1.f;
   bool m_paused = false;
};
