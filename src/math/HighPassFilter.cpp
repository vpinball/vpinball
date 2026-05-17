// license:GPLv3+

#include "core/stdafx.h"
#include "HighPassFilter.h"

#include <cmath>

HighPassFilter::HighPassFilter(float cutoff_hz, float sample_rate_hz, float q_factor)
   : m_sample_rate(sample_rate_hz)
   , m_cutoff(cutoff_hz)
   , m_q(q_factor)
{
   ComputeCoefficients();
}

void HighPassFilter::SetCutoff(float cutoff_hz)
{
   m_cutoff = cutoff_hz;
   ComputeCoefficients();
}

void HighPassFilter::SetQ(float q_factor)
{
   m_q = q_factor;
   ComputeCoefficients();
}

void HighPassFilter::SetSampleRate(float sample_rate_hz)
{
   m_sample_rate = sample_rate_hz;
   ComputeCoefficients();
}

// Compute biquad coefficients for a 2nd-order high-pass via the bilinear transform with frequency pre-warping.
//
// Analogue prototype (normalised): H(s) = s^2 / (s^2 + s/Q + 1)
// Pre-warped gain constant: K = tan(pi * fc / fs)
//
// Mapped digital coefficients (a0 = 1, incorporated into all terms):
//   norm = 1 + K/Q + K^2
//
//   b0 =  1             / norm
//   b1 = -2             / norm
//   b2 =  1             / norm
//   a1 =  2*(K^2 - 1)   / norm
//   a2 = (1 - K/Q + K^2)/ norm
void HighPassFilter::ComputeCoefficients()
{
   assert(m_sample_rate > 0.0f);
   assert(m_cutoff > 0.0f && m_cutoff < m_sample_rate * 0.5f);
   assert(m_q > 0.0f);

   const float K = std::tan(M_PIf * m_cutoff / m_sample_rate);
   const float K2 = K * K;
   const float norm = 1.0f + K / m_q + K2;

   m_b0 = 1.0f / norm;
   m_b1 = -2.0f / norm;
   m_b2 = 1.0f / norm; // same as b0
   m_a1 = 2.0f * (K2 - 1.0f) / norm;
   m_a2 = (1.0f - K / m_q + K2) / norm;

   // Reset state
   m_s1 = 0.0f;
   m_s2 = 0.0f;
   m_output = 0.0f;
}

float HighPassFilter::Push(float x) noexcept
{
   // Transposed Direct Form II:
   //   y    = b0*x  + s1
   //   s1'  = b1*x  - a1*y + s2
   //   s2'  = b2*x  - a2*y
   const float y = m_b0 * x + m_s1;
   m_s1 = m_b1 * x - m_a1 * y + m_s2;
   m_s2 = m_b2 * x - m_a2 * y;
   m_output = y;
   return y;
}
