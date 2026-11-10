#pragma once

#include <cmath>
#include <numbers>

namespace Flex {

// Easing functions (linear progress t in [0..1] to eased progress) from Glide's Ease.cs, as used by the original C# FlexDMD
// (Glide: Copyright (c) 2013 Jacob Albano, MIT license). Unlike Glide, computed in float, with exact PI (Glide: 3.14159f), and exp2f (Glide: pow(2, x)):
// results differ from Glide's by at most about 1e-5 of the range
namespace Ease
{
   using Func = float (*)(float t);

   // Computed in double, rounded once to float
   constexpr double PI_D = std::numbers::pi;
   constexpr float PI = static_cast<float>(PI_D);
   constexpr float PI2 = static_cast<float>(PI_D / 2);
   constexpr float ELASTIC_FREQ = static_cast<float>(13 * PI_D / 2);
   constexpr float B1 = static_cast<float>(1 / 2.75);
   constexpr float B2 = static_cast<float>(2 / 2.75);
   constexpr float B3 = static_cast<float>(1.5 / 2.75);
   constexpr float B4 = static_cast<float>(2.5 / 2.75);
   constexpr float B5 = static_cast<float>(2.25 / 2.75);
   constexpr float B6 = static_cast<float>(2.625 / 2.75);
   constexpr double BACK_S_D = 1.70158; // Penner's overshoot (about 10%), standard across easing libraries
   constexpr float BACK_S = static_cast<float>(BACK_S_D);
   constexpr float BACK_S1 = static_cast<float>(BACK_S_D + 1);

   inline float Linear(float t) { return t; } // Glide's null easing

   inline float ElasticIn(float t) { return sinf(ELASTIC_FREQ * t) * exp2f(10 * (t - 1)); }
   inline float ElasticOut(float t)
   {
      if (t == 1)
         return 1;
      return sinf(-ELASTIC_FREQ * (t + 1)) * exp2f(-10 * t) + 1;
   }
   inline float ElasticInOut(float t)
   {
      if (t < 0.5f)
         return 0.5f * sinf(ELASTIC_FREQ * (2 * t)) * exp2f(10 * ((2 * t) - 1));
      return 0.5f * (sinf(-ELASTIC_FREQ * ((2 * t - 1) + 1)) * exp2f(-10 * (2 * t - 1)) + 2);
   }

   inline float QuadIn(float t) { return t * t; }
   inline float QuadOut(float t) { return -t * (t - 2); }
   inline float QuadInOut(float t)
   {
      if (t <= .5f)
         return t * t * 2;
      t -= 1;
      return 1 - t * t * 2;
   }

   inline float CubeIn(float t) { return t * t * t; }
   inline float CubeOut(float t)
   {
      t -= 1;
      return 1 + t * t * t;
   }
   inline float CubeInOut(float t)
   {
      if (t <= .5f)
         return t * t * t * 4;
      t -= 1;
      return 1 + t * t * t * 4;
   }

   inline float QuartIn(float t) { return t * t * t * t; }
   inline float QuartOut(float t)
   {
      t -= 1;
      return 1 - t * t * t * t;
   }
   inline float QuartInOut(float t)
   {
      if (t <= .5f)
         return t * t * t * t * 8;
      t = t * 2 - 2;
      return (1 - t * t * t * t) / 2 + .5f;
   }

   inline float QuintIn(float t) { return t * t * t * t * t; }
   inline float QuintOut(float t)
   {
      t -= 1;
      return t * t * t * t * t + 1;
   }
   inline float QuintInOut(float t)
   {
      t *= 2;
      if (t < 1)
         return (t * t * t * t * t) / 2;
      t -= 2;
      return (t * t * t * t * t + 2) / 2;
   }

   inline float SineIn(float t)
   {
      if (t == 1)
         return 1;
      return -cosf(PI2 * t) + 1;
   }
   inline float SineOut(float t) { return sinf(PI2 * t); }
   inline float SineInOut(float t) { return -cosf(PI * t) / 2 + .5f; }

   inline float BounceOut(float t)
   {
      if (t < B1)
         return 7.5625f * t * t;
      if (t < B2)
         return 7.5625f * (t - B3) * (t - B3) + .75f;
      if (t < B4)
         return 7.5625f * (t - B5) * (t - B5) + .9375f;
      return 7.5625f * (t - B6) * (t - B6) + .984375f;
   }
   inline float BounceIn(float t) { return 1 - BounceOut(1 - t); }
   inline float BounceInOut(float t)
   {
      if (t < .5f)
         return BounceIn(t * 2) / 2;
      return BounceOut(t * 2 - 1) / 2 + .5f;
   }

   inline float CircIn(float t) { return -(sqrtf(1 - t * t) - 1); }
   inline float CircOut(float t) { return sqrtf(1 - (t - 1) * (t - 1)); }
   inline float CircInOut(float t) { return t <= .5f ? (sqrtf(1 - t * t * 4) - 1) / -2 : (sqrtf(1 - (t * 2 - 2) * (t * 2 - 2)) + 1) / 2; }

   // Expo curves end exactly at 1 (tweeny, used before, stopped 2^-10 short). ExpoIn and ExpoInOut start at 2^-10, not 0
   inline float ExpoIn(float t) { return exp2f(10 * (t - 1)); }
   inline float ExpoOut(float t)
   {
      if (t == 1)
         return 1;
      return -exp2f(-10 * t) + 1;
   }
   inline float ExpoInOut(float t)
   {
      if (t == 1)
         return 1;
      return t < .5f ? exp2f(10 * (t * 2 - 1)) / 2 : (-exp2f(-10 * (t * 2 - 1)) + 2) / 2;
   }

   inline float BackIn(float t) { return t * t * (BACK_S1 * t - BACK_S); }
   inline float BackOut(float t)
   {
      t -= 1;
      return 1 - t * t * (-BACK_S1 * t - BACK_S);
   }
   inline float BackInOut(float t)
   {
      t *= 2;
      if (t < 1)
         return t * t * (BACK_S1 * t - BACK_S) / 2;
      t -= 2;
      return (1 - t * t * (-BACK_S1 * t - BACK_S)) / 2 + .5f;
   }
}

// Eased progress over a duration, as a Glide tween without delay or repeat. The owner applies it to its values with Lerp
class Tween final
{
public:
   explicit Tween(float durationS, Ease::Func ease = Ease::Linear)
      : m_duration(durationS)
      , m_ease(ease)
   {
   }

   // Returns false when there is nothing to apply: as in Glide, the first update only starts the tween; a finished one stays done
   bool Update(float secondsElapsed)
   {
      if (!m_started)
      {
         m_started = true;
         return false;
      }
      if (m_done)
         return false;
      m_time += secondsElapsed;
      float t;
      if (!(m_time < m_duration)) // also zero, negative and NaN durations
      {
         m_time = m_duration;
         t = 1.f;
         m_done = true;
      }
      else
         t = m_time / m_duration;
      m_progress = m_ease(t);
      return true;
   }

   bool IsDone() const { return m_done; }

   // Glide's numeric lerp, snapped to the end value once finished (from + (to - from) * 1 can be off by a float rounding)
   float Lerp(float from, float to) const { return m_done ? to : from + (to - from) * m_progress; }

private:
   float m_duration; // seconds
   Ease::Func m_ease;
   float m_time = 0.f; // seconds
   float m_progress = 0.f;
   bool m_started = false;
   bool m_done = false;
};

}
