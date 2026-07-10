// license:GPLv3+

#include "core/stdafx.h"
#include "NudgeIntentHandler.h"

namespace VPX::Physics
{


///////////////////////////////////////////////////////////////////////////////////////////////////
// Nudge intent handler
//
// For a cabinet, filter intent by detecting the highest peaks in a sequence, but discarding lower peaks, as 
// these likely correspond to natural cabinet oscillation.
//
// For gamepads, evaluate the player intent from a gamepad stick move, only registering push (moving
// outward) and considering strength as a combination of speed and depth of a stick move.

NudgeIntentHandler::NudgeIntentHandler(bool isGamepad)
   : m_isGamepad(isGamepad)
   , m_impulseLength(25)
   , m_impulseElapsed(m_impulseLength + 1)
   , m_impulseDelay(0)
{
   m_impulse.SetZero();
}

Vertex2D NudgeIntentHandler::GetImpulseAceleration() const
{
   if (!IsImpulseInProgress() || m_impulseElapsed < m_impulseDelay)
      return { 0.f, 0.f };
   const float t = static_cast<float>(m_impulseElapsed - m_impulseDelay) / static_cast<float>(m_impulseLength);
   return m_impulse * 0.5f * (1.0f - cosf((float)(2. * M_PI) * t));
}

float NudgeIntentHandler::GetImpulseStrengthFactor() const
{
   // Gamepad evaluate strength from depth and speed of stick move: very fast is full strength, then decrease
   // return m_isGamepad ? (1.f - clamp((static_cast<float>(m_segmentEnd - m_segmentStart) - 50.f) / 500.f, 0.f, 1.f)) : 1.f;
   // FIXME strength factor would destroy some impulses (if starts is not directly followed by the raise to max, but wander a bit in between), so for the time being, we only use the final stick position
   return 1.f;
}

int NudgeIntentHandler::GetImpulseDelay(float impulseStrength) const
{
   constexpr float noDelayStrength = 5.f; // 5m/s^2 is a 0.5g strong nudge, do not delay if we are already near a tilt threshold
   const float firmness = clamp(impulseStrength / noDelayStrength, 0.f, 1.f);
   // Delay impulse to avoid missing the input apex. The length is pure magic here (if we knew the sensor update rate, we could do better)
   const float inputPollPeriod = m_isGamepad ? 16.f : 8.f;
   return (int)lerp(3.f * inputPollPeriod, inputPollPeriod, firmness);
}

void NudgeIntentHandler::EvaluateImpulse(const Vertex2D& impulse)
{
   assert(!m_segmentImpulseSent);

   // Filter cabinet oscillation (smaller peaks shortly after a stronger one)
   if (!m_isGamepad && (m_segmentStrength <= m_lastImpulseStrength) && (m_segmentEnd - m_lastImpulseTime <= 300))
      return;

   // Send the impulse early, but with a raising edge that depends on how strong the nudge is (delay on soft nudge where the risk of missing the apex is high)
   // This is needed as physically impulse are very short (around 25ms) while gamepad / sensor USB acquisition range from 8ms (standard USB 125Hz sensor polling rate) to 16ms (Xbox bluetooth controller)
   const float impulseThreshold = 1.0f; // m/s^2
   const float strengthFactor = GetImpulseStrengthFactor();
   const float impulseStrength = strengthFactor * m_segmentStrength;
   if (impulseStrength > impulseThreshold)
   {
      m_impulse = strengthFactor * impulse;
      m_impulseElapsed = 0;
      m_impulseDelay = GetImpulseDelay(impulseStrength);
      m_segmentImpulseSent = true;
      PLOGD_IF(false) << std::format("Impulse sent: {:8.5f}, {:8.5f} (strength factor: {:8.5f}) with delay:{:2d}ms", m_impulse.x, m_impulse.y, GetImpulseStrengthFactor(), m_impulseDelay);
   }
}

void NudgeIntentHandler::StepOneMillisecond(const Vertex2D& nudgeAcceleration)
{
   m_impulseElapsed++;

   m_time++;
   Vertex2D nudge;
   nudge.x = nudgeAcceleration.x;
   nudge.y = min(nudgeAcceleration.y, 0.f); // Front nudge are always negative peaks (as the player push and does not pull the cab)
   const float strength = nudge.Length();
   if (m_segmentIsPeak)
   {
      if (strength > m_segmentStrength) // Going up
      {
         m_segmentStrength = strength;
         m_segmentEnd = m_time;
         if (!m_segmentImpulseSent)
         {
            EvaluateImpulse(nudge);
         }
         else if (IsImpulseInProgress())
         {
            const Vertex2D newImpulse = GetImpulseStrengthFactor() * nudge; 
            const float impulseStrengthSqr = newImpulse.LengthSquared(); 
            if (impulseStrengthSqr > m_impulse.LengthSquared())
            {
               m_impulse = newImpulse; // Update strength and direction of ongoing impulse
               if (m_impulseElapsed < m_impulseDelay)
                  m_impulseDelay = max(m_impulseElapsed, GetImpulseDelay(sqrtf(impulseStrengthSqr)));
               PLOGD_IF(false) << std::format("Impulse updated: {:8.5f}, {:8.5f} (strength factor: {:8.5f}) at pos: {:d}ms, delay: {:d}ms", m_impulse.x, m_impulse.y, GetImpulseStrengthFactor(),
                  m_impulseElapsed - m_impulseDelay, m_impulseDelay);
            }
         }
      }
      else if (strength < m_segmentStrength * 0.9f) // Peak is finished, switch to rearm mode
      {
         m_lastImpulseTime = m_segmentEnd;
         m_lastImpulseStrength = m_segmentStrength;
         m_segmentStrength = strength;
         m_segmentStart = m_time;
         m_segmentEnd = m_time;
         m_segmentIsPeak = false;
      }
   }
   else // After a peak has been reached, search for the start of the next peak
   {
      if (strength < m_segmentStrength) // Going down
      {
         m_segmentStrength = strength;
         m_segmentEnd = m_time;
      }
      else if (strength > max(0.1f, m_segmentStrength * 1.1f)) // Start of a new peak detected, switch to peak mode
      {
         m_segmentStrength = strength;
         m_segmentStart = m_time;
         m_segmentEnd = m_time;
         m_segmentIsPeak = true;
         m_segmentImpulseSent = false;
         EvaluateImpulse(nudge);
      }
   }
}


}
