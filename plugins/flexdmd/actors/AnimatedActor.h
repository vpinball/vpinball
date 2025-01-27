#pragma once

#include "Actor.h"
#include "VPXPlugin.h"

class AnimatedActor : public Actor
{
public:
   AnimatedActor(FlexDMD* pFlexDMD, const string& name)
      : Actor(pFlexDMD, name) { }
   ~AnimatedActor() = default;

   void Update(float delta) override;
   virtual void Seek(float posInSeconds);
   virtual void Advance(float delta);
   virtual void Rewind();
   virtual void ReadNextFrame() = 0;

   float GetFrameTime() { return m_frameTime; }
   void SetFrameTime(float frameTime) { m_frameTime = frameTime; }
   float GetFrameDuration() { return m_frameDuration; }
   void SetFrameDuration(float frameDuration) { assert(frameDuration > 0.f); m_frameDuration = frameDuration; }
   void SetTime(float time) { m_time = time; }
   void SetEndOfAnimation(bool endOfAnimation) { m_endOfAnimation = endOfAnimation; }
   Scaling GetScaling() { return m_scaling; }
   void SetScaling(Scaling scaling) { m_scaling = scaling; }
   Alignment GetAlignment() { return m_alignment; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }
   virtual float GetLength() { return m_length; }
   void SetLength(float length) { m_length = length; }
   bool GetLoop() { return m_loop; }
   void SetLoop(bool loop) { m_loop = loop; }

private:
   float m_frameTime = 0.f;
   float m_frameDuration = 1.f / 60.f;
   float m_time = 0.f;
   bool m_endOfAnimation = 0.f;

   Scaling m_scaling = Scaling_Stretch;
   Alignment m_alignment = Alignment_Center;
   bool m_paused = false;
   bool m_loop = true;
   float m_length = 0.f;
   float m_playSpeed = 1.0f;
};