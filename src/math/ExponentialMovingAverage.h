#pragma once

class ExponentialMovingAverage final
{
public:
   // tauSeconds: smoothing time constant (e.g. 0.004 = 4 ms)
   explicit ExponentialMovingAverage(float tauSeconds)
      : m_tau(tauSeconds)
   {
   }

   void Reset(float value = 0.0f)
   {
      m_initialized = false;
      m_value = value;
   }

   // dtSeconds: elapsed time since last sample (e.g. 0.001 for 1 ms loop)
   float Update(float input, float dtSeconds)
   {
      if (!m_initialized)
      {
         m_value = input;
         m_initialized = true;
         return m_value;
      }

      // alpha = dt / (tau + dt)
      const float alpha = dtSeconds / (m_tau + dtSeconds);

      m_value += alpha * (input - m_value);
      return m_value;
   }

   float Get() const
   {
      return m_value;
   }

private:
   float m_tau = 0.0f;
   float m_value = 0.0f;
   bool m_initialized = false;
};
