#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

class MotionGainCalibratorAxis final
{
public:
   struct Config
   {
      // Initial gain guess.
      // The estimated gain multiplies RAW ACCELERATION so that:
      //   scaledAcceleration = gain * rawAcceleration
      float m_initialGain = 1.0f;

      // Segment acceptance criteria
      uint64_t m_minSegmentDurationUs = 30000;   // 30 ms
      uint64_t m_maxSegmentDurationUs = 2000000; // 2 s
      size_t m_minSampleCount = 8;

      // Minimum excitation required to accept a segment
      float m_minVelocityPeakToPeak = 0.02f;      // in velocity sensor units
      float m_minIntegratedAccelPeakToPeak = 0.002f; // in integrated accel raw units
      float m_minRegressionDenominator = 1.0e-6f;

      // Forgetting factor for accumulation across segments.
      // 1.0f  => pure accumulation (no forgetting)
      // <1.0f => slow adaptation over time
      float m_forgettingFactor = 0.995f;

      // Clamp range for the global gain
      float m_minGain = 0.001f;
      float m_maxGain = 1000.0f;

      // Optional robustification:
      // suppress the first and last sample from the regression if desired
      bool m_excludeSegmentEndpointsFromFit = true;

      // Optional weighting:
      // if true, samples farther from the segment boundaries get more weight
      bool m_useTriangularWeights = true;

      // Hard safety epsilon
      float m_epsilon = 1.0e-9f;
   };

   struct SegmentResult
   {
      bool m_accepted = false;

      uint64_t m_durationUs = 0;
      size_t m_sampleCount = 0;

      float m_segmentGain = 1.0f;          // best gain for this segment alone
      float m_segmentQuality = 0.0f;       // 0..1
      float m_segmentResidualRms = 0.0f;   // after fitting
      float m_velocityPeakToPeak = 0.0f;
      float m_integratedAccelPeakToPeak = 0.0f;

      // Regression sufficient statistics of the accepted segment
      float m_segmentNumerator = 0.0f;     // sum(w*x*y)
      float m_segmentDenominator = 0.0f;   // sum(w*x*x)
   };

private:
   struct Sample
   {
      uint64_t m_timeUs = 0;
      float m_velocity = 0.0f;
      float m_acceleration = 0.0f;
   };

public:
   explicit MotionGainCalibratorAxis(const Config& config)
      : m_config(config)
   {
      Reset();
   }

   void Configure(const Config& config)
   {
      m_config = config;

      // Keep current accumulated stats but clamp gain to the new range
      m_gain = clamp(m_gain, m_config.m_minGain, m_config.m_maxGain);
   }

   const Config& GetConfig() const
   {
      return m_config;
   }

   void Reset()
   {
      m_segmentActive = false;
      m_samples.clear();

      m_gain = m_config.m_initialGain;
      m_accumulatedNumerator = 0.0;
      m_accumulatedDenominator = 0.0;

      m_startedSegmentCount = 0;
      m_acceptedSegmentCount = 0;
      m_rejectedSegmentCount = 0;

      m_totalAcceptedDurationUs = 0;
      m_lastResult = SegmentResult{};
      m_globalConfidence = 0.0f;
   }

   bool IsSegmentActive() const
   {
      return m_segmentActive;
   }

   float GetGain() const
   {
      return m_gain;
   }

   // Convenience helpers
   float ScaleAcceleration(float rawAcceleration) const
   {
      return m_gain * rawAcceleration;
   }

   float ScaleVelocityToAccelerationUnits(float rawVelocity) const
   {
      if (std::fabs(m_gain) <= m_config.m_epsilon)
         return 0.0f;
      return rawVelocity / m_gain;
   }

   size_t GetStartedSegmentCount() const
   {
      return m_startedSegmentCount;
   }

   size_t GetAcceptedSegmentCount() const
   {
      return m_acceptedSegmentCount;
   }

   size_t GetRejectedSegmentCount() const
   {
      return m_rejectedSegmentCount;
   }

   uint64_t GetTotalAcceptedDurationUs() const
   {
      return m_totalAcceptedDurationUs;
   }

   float GetGlobalConfidence() const
   {
      return m_globalConfidence;
   }

   const SegmentResult& GetLastSegmentResult() const
   {
      return m_lastResult;
   }

   // Start a new segment, typically when leaving rest.
   // If a segment is already active, it is discarded and restarted.
   void StartSegment(uint64_t timeUs)
   {
      m_segmentActive = true;
      m_samples.clear();
      m_startedSegmentCount++;
      m_segmentStartUs = timeUs;
   }

   // Add a synchronized pair of raw samples.
   //
   // IMPORTANT:
   //   This class assumes velocity and acceleration values correspond to the same timestamp.
   //   If your channels are asynchronous, resample or align them before calling AddSample().
   //
   // The values can be in arbitrary/raw units:
   //   - velocity in velocity-sensor units
   //   - acceleration in acceleration-sensor units
   //
   // The calibrator estimates the gain that scales acceleration to the velocity channel.
   bool AddSample(uint64_t timeUs, float rawVelocity, float rawAcceleration)
   {
      if (!m_segmentActive)
         return false;

      if (!m_samples.empty() && timeUs <= m_samples.back().m_timeUs)
      {
         // Reject non-monotonic timestamps for safety
         return false;
      }

      m_samples.push_back(Sample{ timeUs, rawVelocity, rawAcceleration });
      return true;
   }

   // End the current segment, typically when entering rest again.
   // Returns true if the segment was accepted and incorporated into the global estimate.
   bool EndSegment()
   {
      if (!m_segmentActive)
      {
         m_lastResult = SegmentResult{};
         return false;
      }

      m_segmentActive = false;

      m_lastResult = EvaluateCurrentSegment();

      if (!m_lastResult.m_accepted)
      {
         m_rejectedSegmentCount++;
         m_samples.clear();
         return false;
      }

      // Recursive accumulation with forgetting factor
      const double lambda = static_cast<double>(clamp(m_config.m_forgettingFactor, 0.0f, 1.0f));

      m_accumulatedNumerator = lambda * m_accumulatedNumerator
                             + static_cast<double>(m_lastResult.m_segmentNumerator);

      m_accumulatedDenominator = lambda * m_accumulatedDenominator
                               + static_cast<double>(m_lastResult.m_segmentDenominator);

      if (m_accumulatedDenominator > static_cast<double>(m_config.m_epsilon))
      {
         const double gain = m_accumulatedNumerator / m_accumulatedDenominator;
         m_gain = clamp(static_cast<float>(gain), m_config.m_minGain, m_config.m_maxGain);
      }

      m_acceptedSegmentCount++;
      m_totalAcceptedDurationUs += m_lastResult.m_durationUs;
      m_globalConfidence = ComputeGlobalConfidence();

      m_samples.clear();
      return true;
   }

private:
   float ComputeGlobalConfidence() const
   {
      // Confidence based on accepted sample volume and denominator strength
      if (m_acceptedSegmentCount == 0)
         return 0.0f;

      const float segFactor = 1.0f - std::exp(-0.25f * static_cast<float>(m_acceptedSegmentCount));

      const float durationS = static_cast<float>(m_totalAcceptedDurationUs) * 1.0e-6f;
      const float durationFactor = 1.0f - std::exp(-durationS * 0.5f);

      const float denomFactor = 1.0f - std::exp(-static_cast<float>(m_accumulatedDenominator) * 0.25f);

      return clamp(0.40f * segFactor + 0.35f * durationFactor + 0.25f * denomFactor, 0.0f, 1.0f);
   }

   SegmentResult EvaluateCurrentSegment() const
   {
      SegmentResult result{};

      const size_t n = m_samples.size();
      result.m_sampleCount = n;

      if (n < m_config.m_minSampleCount)
         return result;

      const uint64_t startUs = m_samples.front().m_timeUs;
      const uint64_t endUs = m_samples.back().m_timeUs;
      const uint64_t durationUs = endUs - startUs;
      result.m_durationUs = durationUs;

      if (durationUs < m_config.m_minSegmentDurationUs || durationUs > m_config.m_maxSegmentDurationUs)
         return result;

      // -------------------------------------------------------------------
      // Build:
      //   - detrended velocity trajectory y[i]
      //   - detrended integrated-acceleration trajectory x[i]
      //
      // Because the segment begins and ends at rest, the FULL segment net
      // delta-v is near zero. So we fit the INTERNAL shape, not the net value.
      //
      // We remove a linear baseline from both signals to suppress:
      //   - offset/drift on the velocity channel
      //   - integrated acceleration drift due to accel bias
      // -------------------------------------------------------------------

      std::vector<float> tNorm(n, 0.0f);
      std::vector<float> vel(n, 0.0f);
      std::vector<float> velDetrended(n, 0.0f);
      std::vector<float> integAccel(n, 0.0f);
      std::vector<float> integAccelDetrended(n, 0.0f);

      const double totalDurationS = static_cast<double>(durationUs) * 1.0e-6;
      if (totalDurationS <= 0.0)
         return result;

      // Copy velocity samples and normalized time
      for (size_t i = 0; i < n; ++i)
      {
         const double relUs = static_cast<double>(m_samples[i].m_timeUs - startUs);
         tNorm[i] = static_cast<float>(relUs / static_cast<double>(durationUs)); // [0..1]
         vel[i] = m_samples[i].m_velocity;
      }

      // Trapezoidal integration of raw acceleration
      integAccel[0] = 0.0f;
      for (size_t i = 1; i < n; ++i)
      {
         const float dt = static_cast<float>(m_samples[i].m_timeUs - m_samples[i - 1].m_timeUs) * 1.0e-6f;
         const float a0 = m_samples[i - 1].m_acceleration;
         const float a1 = m_samples[i].m_acceleration;
         integAccel[i] = integAccel[i - 1] + 0.5f * (a0 + a1) * dt;
      }

      // Detrend velocity with a linear baseline from first to last sample
      const float vel0 = vel.front();
      const float vel1 = vel.back();
      for (size_t i = 0; i < n; ++i)
      {
         const float baseline = lerp(vel0, vel1, tNorm[i]);
         velDetrended[i] = vel[i] - baseline;
      }

      // Detrend integrated acceleration with a linear baseline from first to last value
      const float int0 = integAccel.front(); // expected 0
      const float int1 = integAccel.back();
      for (size_t i = 0; i < n; ++i)
      {
         const float baseline = lerp(int0, int1, tNorm[i]);
         integAccelDetrended[i] = integAccel[i] - baseline;
      }

      // Segment excitation checks
      result.m_velocityPeakToPeak = PeakToPeak(velDetrended);
      result.m_integratedAccelPeakToPeak = PeakToPeak(integAccelDetrended);

      if (result.m_velocityPeakToPeak < m_config.m_minVelocityPeakToPeak)
         return result;

      if (result.m_integratedAccelPeakToPeak < m_config.m_minIntegratedAccelPeakToPeak)
         return result;

      // -------------------------------------------------------------------
      // Weighted least-squares fit through the origin:
      //   y ~= k * x
      //
      // where:
      //   x = detrended integrated accel
      //   y = detrended velocity
      //
      // k = sum(w*x*y) / sum(w*x*x)
      // -------------------------------------------------------------------

      double numerator = 0.0;
      double denominator = 0.0;

      const size_t fitBegin = (m_config.m_excludeSegmentEndpointsFromFit && n >= 3) ? 1 : 0;
      const size_t fitEnd = (m_config.m_excludeSegmentEndpointsFromFit && n >= 3) ? (n - 1) : n;

      for (size_t i = fitBegin; i < fitEnd; ++i)
      {
         const float x = integAccelDetrended[i];
         const float y = velDetrended[i];
         const float w = ComputeSampleWeight(tNorm[i]);

         numerator += static_cast<double>(w) * static_cast<double>(x) * static_cast<double>(y);
         denominator += static_cast<double>(w) * static_cast<double>(x) * static_cast<double>(x);
      }

      result.m_segmentNumerator = static_cast<float>(numerator);
      result.m_segmentDenominator = static_cast<float>(denominator);

      if (denominator < static_cast<double>(m_config.m_minRegressionDenominator))
         return result;

      const double segmentGain = numerator / denominator;
      result.m_segmentGain = static_cast<float>(segmentGain);

      if (!std::isfinite(result.m_segmentGain))
         return result;

      // Residual RMS to evaluate segment quality
      double weightedResidual2 = 0.0;
      double weightedCount = 0.0;

      for (size_t i = fitBegin; i < fitEnd; ++i)
      {
         const float x = integAccelDetrended[i];
         const float y = velDetrended[i];
         const float w = ComputeSampleWeight(tNorm[i]);

         const double r = static_cast<double>(y) - segmentGain * static_cast<double>(x);
         weightedResidual2 += static_cast<double>(w) * r * r;
         weightedCount += static_cast<double>(w);
      }

      if (weightedCount <= static_cast<double>(m_config.m_epsilon))
         return result;

      result.m_segmentResidualRms = static_cast<float>(std::sqrt(weightedResidual2 / weightedCount));

      // Quality metric: normalized to [0..1]
      //
      // Compare residual RMS to the velocity peak-to-peak amplitude.
      // Low residuals vs signal amplitude => good quality.
      const float signalScale = std::max(result.m_velocityPeakToPeak, m_config.m_epsilon);
      const float normalizedResidual = result.m_segmentResidualRms / signalScale;

      result.m_segmentQuality = 1.0f - clamp(normalizedResidual * 2.0f, 0.0f, 1.0f);

      // Final acceptance
      if (!std::isfinite(result.m_segmentQuality))
         return result;

      // Reject obviously absurd segment gains
      if (result.m_segmentGain < m_config.m_minGain || result.m_segmentGain > m_config.m_maxGain)
         return result;

      // Reject very poor matches
      if (result.m_segmentQuality <= 0.05f)
         return result;

      result.m_accepted = true;
      return result;
   }

   float ComputeSampleWeight(float tNorm) const
   {
      if (!m_config.m_useTriangularWeights)
         return 1.0f;

      // Triangular weight:
      //   0 at boundaries
      //   1 at middle
      //
      // This de-emphasizes boundary samples, which are often less reliable
      // near rest transitions.
      const float d = std::fabs(2.0f * tNorm - 1.0f);
      return std::max(0.0f, 1.0f - d);
   }

   static float PeakToPeak(const std::vector<float>& data)
   {
      if (data.empty())
         return 0.0f;

      float mn = data.front();
      float mx = data.front();

      for (float v : data)
      {
         mn = std::min(mn, v);
         mx = std::max(mx, v);
      }

      return mx - mn;
   }

private:
   Config m_config;

   bool m_segmentActive = false;
   uint64_t m_segmentStartUs = 0;
   std::vector<Sample> m_samples;

   float m_gain = 1.0f;

   // Global sufficient statistics for recursive accumulation
   double m_accumulatedNumerator = 0.0;
   double m_accumulatedDenominator = 0.0;

   size_t m_startedSegmentCount = 0;
   size_t m_acceptedSegmentCount = 0;
   size_t m_rejectedSegmentCount = 0;

   uint64_t m_totalAcceptedDurationUs = 0;

   SegmentResult m_lastResult;
   float m_globalConfidence = 0.0f;
};
