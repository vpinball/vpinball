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
{
   m_impulse.SetZero();
}

Vertex2D NudgeIntentHandler::GetImpulseAceleration() const
{
   if (!IsImpulseInProgress())
      return { 0.f, 0.f };
   const float t = static_cast<float>(m_impulseElapsed) / static_cast<float>(m_impulseLength);
   return m_impulse * 0.5f * (1.0f - cosf((float)(2. * M_PI) * t));
}

float NudgeIntentHandler::GetImpulseStrengthFactor() const
{
   // Gamepad evaluate strength from depth and speed of stick move: very fast is full strength, then decrease
   // return m_isGamepad ? (1.f - clamp((static_cast<float>(m_segmentEnd - m_segmentStart) - 50.f) / 500.f, 0.f, 1.f)) : 1.f;
   // FIXME strength factor would destroy some impulses (if starts is not directly followed by the raise to max, but wander a bit in between), so for the time being, we only use the final stick position
   return 1.f;
}

void NudgeIntentHandler::EvaluateImpulse(const Vertex2D& impulse)
{
   // Send impulse early to limit latency, taking the risk of missing the peak apex if it does not happens during the impulse length (length of contact physics)
   const float impulseThreshold = 1.0f; // m/s^2
   const float strengthFactor = GetImpulseStrengthFactor();
   bool fireImpulse = (strengthFactor * m_segmentStrength) > impulseThreshold;
   if (!m_isGamepad) // Filter cabinet oscillation (smaller peaks shortly after a stronger one)
      fireImpulse &= (m_segmentStrength > m_lastImpulseStrength) || (m_segmentEnd - m_lastImpulseTime > 300);
   if (fireImpulse)
   {
      m_impulse = strengthFactor * impulse;
      m_impulseElapsed = 0;
      m_segmentImpulseSent = true;
      //PLOGD << std::format("Impulse sent: {:8.5f}, {:8.5f} (strength factor: {:8.5f})", m_impulse.x, m_impulse.y, GetImpulseStrengthFactor());
   }
   else
   {
      //PLOGD << std::format("Impulse not sent: {:8.5f}, {:8.5f} => {:8.5f} (strength factor: {:8.5f})", impulse.x, impulse.y, m_segmentStrength, GetImpulseStrengthFactor());
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
         else if (const Vertex2D newImpulse = GetImpulseStrengthFactor() * nudge; newImpulse.LengthSquared() > m_impulse.LengthSquared())
         {
            m_impulse = newImpulse; // Update strength and direction of ongoing impulse
            //PLOGD << std::format("Impulse updated: {:8.5f}, {:8.5f} (strength factor: {:8.5f})", m_impulse.x, m_impulse.y, GetImpulseStrengthFactor());
         }
      }
      else if (strength < m_segmentStrength * 0.9f) // Peak is finished, switch to rearm mode
      {
         if (m_segmentImpulseSent)
         {
            //PLOGD << std::format("Last impulse strength: {:8.5f}, {:8.5f}", m_impulse.x, m_impulse.y);
         }
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
