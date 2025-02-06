#include "Actor.h"
#include "Actions.h"


void AnimatedActor::Seek(float posInSeconds)
{
   Rewind();
   Advance(posInSeconds);
}

void AnimatedActor::Update(float delta)
{
   Actor::Update(delta);

   if (!GetVisible())
      return;

   if (!m_paused)
      Advance(delta * m_playSpeed);
}

void AnimatedActor::Advance(float delta)
{
   m_time += delta;
   while (!m_endOfAnimation && m_time >= m_frameTime + m_frameDuration)
      ReadNextFrame();

   if (m_endOfAnimation && m_loop)
   {
      float length = m_frameTime + m_frameDuration;
      m_time = fmod(m_time, length);
      Rewind();
   }
}

void AnimatedActor::Rewind()
{
   m_time = 0;
   m_frameTime = 0;
   m_endOfAnimation = false;
}