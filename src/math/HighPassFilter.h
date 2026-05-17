// license:GPLv3+

#pragma once

// HighPassFilter
//
// 2nd-order Butterworth IIR high-pass filter implemented as a biquad section.
// Uses the Transposed Direct Form II structure for numerical stability.
class HighPassFilter
{
public:
   // cutoff_hz      – -3 dB cutoff frequency in Hz. Must be in the open interval (0, sample_rate / 2).
   // sample_rate_hz – sample rate in Hz
   // q_factor       – filter Q / resonance
   //                  0.7071f (= 1/sqrt(2)) gives a maximally flat Butterworth
   //                  response with no resonant peak at the cutoff.
   //                  Higher Q -> sharper knee with a resonant bump.
   explicit HighPassFilter(float cutoff_hz, float sample_rate_hz, float q_factor = 0.7071067811865476f);

   // Push one input sample through the filter
   float Push(float x) noexcept;

   // Return the last filtered sample (0.0f before any sample has been pushed)
   [[nodiscard]] float Get() const noexcept { return m_output; }

   void ResetState() noexcept;

   // Each setter recomputes coefficients and resets state.
   void SetCutoff(float cutoff_hz);
   void SetQ(float q_factor);
   void SetSampleRate(float sample_rate_hz);
   [[nodiscard]] float GetCutoffHz() const noexcept { return m_cutoff; }
   [[nodiscard]] float GetSampleRateHz() const noexcept { return m_sample_rate; }
   [[nodiscard]] float GetQFactor() const noexcept { return m_q; }

private:
   void ComputeCoefficients();

   // Configuration
   float m_sample_rate;
   float m_cutoff;
   float m_q;

   // Biquad coefficients
   float m_b0 {}, m_b1 {}, m_b2 {}; // feed-forward (numerator)
   float m_a1 {}, m_a2 {}; // feed-back    (denominator; a0 = 1)

   // Transposed DF-II delay elements
   float m_s1 = 0.0f;
   float m_s2 = 0.0f;

   // Last computed output
   float m_output = 0.0f;
};
