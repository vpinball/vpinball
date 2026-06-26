// license:GPLv3+

#pragma once

#include "math/vector.h"


namespace VPX::Physics
{

class NudgeIntentHandler final
{
public:
   NudgeIntentHandler(bool isGamepad);

   void StepOneMillisecond(const Vertex2D& nudgeAcceleration); // m/s^2

   bool IsImpulseInProgress() const { return m_impulseElapsed <= m_impulseLength; }

   Vertex2D GetImpulseAceleration() const; // m/s^2

private:
   const bool m_isGamepad;
   const int m_impulseLength;
   
   float GetImpulseStrengthFactor() const;
   void EvaluateImpulse(const Vertex2D& impulse);

   int m_impulseElapsed;
   Vertex2D m_impulse;

   uint64_t m_time = 0;
   float m_segmentStrength = 0.f;
   uint64_t m_segmentStart = 0;
   uint64_t m_segmentEnd = 0;
   bool m_segmentIsPeak = false;
   bool m_segmentImpulseSent = false;
   float m_lastImpulseStrength = 0.f;
   uint64_t m_lastImpulseTime = 0;
};

}
