#include "stdafx.h"

#include "AnimatedActor.h"

AnimatedActor::AnimatedActor(FlexDMD* pFlexDMD, const string& name) : Actor(pFlexDMD, name)
{
   m_scaling = Scaling_Stretch;
   m_alignment = Alignment_Center;
   m_paused = false;
   m_loop = true;
   m_playSpeed = 1.0f;
}

AnimatedActor::~AnimatedActor()
{
}

void AnimatedActor::Missing34() { };
void AnimatedActor::Missing35() { };
void AnimatedActor::Missing36() { };
void AnimatedActor::Missing37() { };

STDMETHODIMP AnimatedActor::get_Length(single *pRetVal)
{
   *pRetVal = m_length;

   return S_OK;
}

STDMETHODIMP AnimatedActor::get_Loop(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_loop ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP AnimatedActor::put_Loop(VARIANT_BOOL pRetVal)
{
   m_loop = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP AnimatedActor::get_Paused(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_paused ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP AnimatedActor::put_Paused(VARIANT_BOOL pRetVal)
{
   m_paused = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP AnimatedActor::get_PlaySpeed(single *pRetVal)
{
   *pRetVal = m_playSpeed;

   return S_OK;
}

STDMETHODIMP AnimatedActor::put_PlaySpeed(single pRetVal)
{
   m_playSpeed = pRetVal;

   return S_OK;
}

STDMETHODIMP AnimatedActor::Seek(single posInSeconds)
{
   Rewind();
   Advance(posInSeconds);

   return S_OK;
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

   if (m_endOfAnimation && m_loop) {
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