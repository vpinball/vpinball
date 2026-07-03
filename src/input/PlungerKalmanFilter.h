#pragma once

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdint>

#include "math/matrix.h"

class PlungerKalmanFilter final
{
public:
   struct Config
   {
      // Continuous-time acceleration noise intensity.
      //
      // This does not mean acceleration is part of the state.
      // It models unobserved acceleration as process noise affecting
      // position and velocity.
      //
      // Higher value:
      //    - faster response
      //    - noisier velocity
      //
      // Lower value:
      //    - smoother output
      //    - more lag
      float accelerationProcessNoise = 1.0f;

      // Position measurement variance.
      //
      // This is variance, not standard deviation.
      // For normalized position [0, 1], start around 1e-5 to 1e-4.
      float positionMeasurementVariance = 1.0e-5f;

      // Initial covariance.
      float initialPositionVariance = 1.0e-4f;
      float initialVelocityVariance = 1.0e-2f;

      // Optional physical limits.
      bool enablePositionLimits = false;
      float minPosition = 0.0f;
      float maxPosition = 1.0f;

      // Number of history snapshots retained for rewind support.
      //
      // Each slot covers one call to PredictTo / the internal step size.
      // 64 slots at 1 ms per step covers up to ~64 ms of latency.
      // Must be >= 1.
      int historyCapacity = 64;
   };

public:
   explicit PlungerKalmanFilter()
      : m_config(Config())
   {
      Reset(0.0f, 0.0f, 0ull);
   }

   void SetConfig(const Config& config) { m_config = config; }

   const Config& GetConfig() const { return m_config; }

   // Reset the filter to the given position/velocity and timestamp.
   //
   // timestampNs: the nanosecond timestamp of this initial state.
   //              Pass 0 if the time base is not yet known; the first
   //              UpdatePosition call will anchor the time base.
   void Reset(const float position, const float velocity = 0.0f, const uint64_t timestampNs = 0ull)
   {
      m_x.Set(position, velocity);

      SetZero(m_P);

      m_P.m_d[0][0] = std::max(m_config.initialPositionVariance, 1.0e-12f);
      m_P.m_d[1][1] = std::max(m_config.initialVelocityVariance, 1.0e-12f);

      ApplyPositionLimits();

      m_timeNs = timestampNs;

      // Seed the history with the initial state.
      m_historyHead = 0;
      m_historyCount = 0;
      SaveSnapshot();
   }

   float GetPosition() const { return m_x.x; }
   float GetVelocity() const { return m_x.y; }

   // Returns the filter's current internal time, in nanoseconds.
   uint64_t GetTimeNs() const { return m_timeNs; }

   // Advance the filter to timestampNs.
   //
   // Does nothing if timestampNs <= current time.
   void PredictTo(const uint64_t timestampNs)
   {
      if (timestampNs <= m_timeNs)
         return;

      const uint64_t deltaNs = timestampNs - m_timeNs;
      const float dt = static_cast<float>(deltaNs) * 1.0e-9f;

      SaveSnapshot();
      PredictInternal(dt);

      m_timeNs = timestampNs;
   }

   // Position-only measurement update with timestamp.
   //
   // If timestampNs is in the past relative to the current filter time,
   // the filter rewinds to the nearest saved state before timestampNs,
   // re-predicts to that timestamp, applies the measurement, then
   // re-predicts forward to the original present time.
   //
   // z = position
   // H = [1 0]
   void UpdatePosition(const uint64_t timestampNs, const float measuredPosition)
   {
      if (!IsFinite(measuredPosition))
         return;

      if (m_timeNs == 0ull)
      {
         // Time base not yet established; anchor now.
         m_timeNs = timestampNs;
         m_historyHead = 0;
         m_historyCount = 0;
         SaveSnapshot();
      }

      if (timestampNs >= m_timeNs)
      {
         // Sample is at or after current time: normal forward path.
         if (timestampNs > m_timeNs)
         {
            SaveSnapshot();
            PredictInternal(static_cast<float>(timestampNs - m_timeNs) * 1.0e-9f);
            m_timeNs = timestampNs;
         }
         ApplyMeasurement(measuredPosition);
      }
      else
      {
         // Sample arrived late: rewind, update, then re-predict to now.
         const uint64_t presentNs = m_timeNs;
         RewindAndUpdate(timestampNs, measuredPosition);
         // Re-predict from the corrected state up to the original present.
         if (m_timeNs < presentNs)
         {
            SaveSnapshot();
            PredictInternal(static_cast<float>(presentNs - m_timeNs) * 1.0e-9f);
            m_timeNs = presentNs;
         }
      }
   }

   // Optional pseudo-measurement:
   //
   // velocity ~= 0
   //
   // This is useful when a rest detector determines that the plunger is
   // stationary. Do not call this unconditionally during fast motion.
   void UpdateVelocityZero(const float velocityVariance)
   {
      const float R = std::max(velocityVariance, 1.0e-12f);

      // H = [0 1]
      // y = 0 - velocity
      const float y = -m_x.y;

      // S = P11 + R
      const float S = m_P.m_d[1][1] + R;
      if (!IsFinite(S) || S <= 1.0e-20f)
         return;

      // K = second column of P / S
      const Vertex2D K { m_P.m_d[0][1] / S, m_P.m_d[1][1] / S };

      m_x += K * y;

      m_P.m_d[0][0] -= K.x * S * K.x;
      m_P.m_d[0][1] -= K.x * S * K.y;
      m_P.m_d[1][0] -= K.y * S * K.x;
      m_P.m_d[1][1] -= K.y * S * K.y;

      SymmetrizeCovariance();
      ApplyPositionLimits();
   }

private:
   // -------------------------------------------------------------------------
   // History buffer for rewind support
   // -------------------------------------------------------------------------

   // Maximum history slots.  The true capacity used is m_config.historyCapacity,
   // clamped to this compile-time upper bound to keep this class allocation-free.
   static constexpr int k_maxHistoryCapacity = 256;

   struct HistoryEntry
   {
      uint64_t timestampNs;
      Vertex2D x;
      Matrix3 P;
   };

   // Save a snapshot of the current state *before* the next prediction step.
   void SaveSnapshot()
   {
      const int cap = std::max(1, std::min(m_config.historyCapacity, k_maxHistoryCapacity));

      HistoryEntry& entry = m_historyBuffer[m_historyHead];
      entry.timestampNs = m_timeNs;
      entry.x = m_x;
      entry.P = m_P;

      m_historyHead = (m_historyHead + 1) % cap;
      if (m_historyCount < cap)
         ++m_historyCount;
   }

   // Restore the most recent saved state whose timestamp <= targetNs.
   //
   // Returns true if a suitable entry was found and restored; the filter
   // time is set to that entry's timestamp.
   // Returns false if the buffer is empty (no rewind possible).
   bool RestoreSnapshot(const uint64_t targetNs)
   {
      const int cap = std::max(1, std::min(m_config.historyCapacity, k_maxHistoryCapacity));

      if (m_historyCount == 0)
         return false;

      // Walk backwards through the circular buffer (most-recent first) to find
      // the latest snapshot at or before targetNs.
      int bestSlot = -1;
      uint64_t bestTs = 0ull;

      for (int i = 0; i < m_historyCount; ++i)
      {
         // Index of the i-th most-recent entry.
         const int slot = ((m_historyHead - 1 - i) % cap + cap) % cap;
         const uint64_t ts = m_historyBuffer[slot].timestampNs;

         if (ts <= targetNs)
         {
            if (bestSlot == -1 || ts > bestTs)
            {
               bestSlot = slot;
               bestTs = ts;
            }
            // Since we walk newest-first, the first match is always the best.
            break;
         }
      }

      if (bestSlot == -1)
      {
         // All snapshots are newer than targetNs: fall back to the oldest.
         const int oldestSlot = ((m_historyHead - m_historyCount) % cap + cap) % cap;
         bestSlot = oldestSlot;
         bestTs = m_historyBuffer[oldestSlot].timestampNs;
      }

      m_x = m_historyBuffer[bestSlot].x;
      m_P = m_historyBuffer[bestSlot].P;
      m_timeNs = bestTs;

      // Truncate the history to entries up to and including bestSlot so that
      // SaveSnapshot() starts fresh from the rewound point.
      // Compute how many entries to keep (from oldest up to bestSlot inclusive).
      // We do this by resetting head to bestSlot+1 and adjusting count.
      //
      // Walk forward from oldest to find bestSlot's distance from oldest.
      int keepCount = 0;
      for (int i = 0; i < m_historyCount; ++i)
      {
         const int slot = ((m_historyHead - m_historyCount + i) % cap + cap) % cap;
         keepCount = i + 1;
         if (slot == bestSlot)
            break;
      }

      m_historyCount = keepCount;
      m_historyHead = (bestSlot + 1) % cap;

      return true;
   }

   // Rewind to the state just before timestampNs, apply the measurement,
   // leaving m_timeNs at the sample timestamp.
   void RewindAndUpdate(const uint64_t timestampNs, const float measuredPosition)
   {
      if (!RestoreSnapshot(timestampNs))
      {
         // Cannot rewind: apply measurement to current state as a best effort.
         ApplyMeasurement(measuredPosition);
         return;
      }

      // Predict from the restored time up to the sample timestamp.
      if (m_timeNs < timestampNs)
      {
         PredictInternal(static_cast<float>(timestampNs - m_timeNs) * 1.0e-9f);
         m_timeNs = timestampNs;
      }

      ApplyMeasurement(measuredPosition);
   }

   // -------------------------------------------------------------------------
   // Core Kalman operations
   // -------------------------------------------------------------------------

   static bool IsFinite(const float v) { return std::isfinite(v); }

   static void SetZero(Matrix3& m)
   {
      for (int r = 0; r < 3; ++r)
      {
         for (int c = 0; c < 3; ++c)
            m.m_d[r][c] = 0.0f;
      }
   }

   static Vertex2D Mul2(const Matrix3& m, const Vertex2D& v) { return Vertex2D { m.m_d[0][0] * v.x + m.m_d[0][1] * v.y, m.m_d[1][0] * v.x + m.m_d[1][1] * v.y }; }

   static Matrix3 Mul2(const Matrix3& a, const Matrix3& b)
   {
      Matrix3 result;
      SetZero(result);

      for (int r = 0; r < 2; ++r)
      {
         for (int c = 0; c < 2; ++c)
         {
            result.m_d[r][c] = a.m_d[r][0] * b.m_d[0][c] + a.m_d[r][1] * b.m_d[1][c];
         }
      }

      return result;
   }

   // result = a * b^T, using only the top-left 2x2 block.
   static Matrix3 Mul2RightTranspose(const Matrix3& a, const Matrix3& b)
   {
      Matrix3 result;
      SetZero(result);

      for (int r = 0; r < 2; ++r)
      {
         for (int c = 0; c < 2; ++c)
         {
            result.m_d[r][c] = a.m_d[r][0] * b.m_d[c][0] + a.m_d[r][1] * b.m_d[c][1];
         }
      }

      return result;
   }

   static Matrix3 Add2(const Matrix3& a, const Matrix3& b)
   {
      Matrix3 result;
      SetZero(result);

      for (int r = 0; r < 2; ++r)
      {
         for (int c = 0; c < 2; ++c)
            result.m_d[r][c] = a.m_d[r][c] + b.m_d[r][c];
      }

      return result;
   }

   void ComputeProcessNoise(const float dt, Matrix3& Q) const
   {
      SetZero(Q);

      // Continuous white acceleration noise model for [position, velocity].
      //
      // Q = q * [ dt^3 / 3   dt^2 / 2
      //           dt^2 / 2   dt       ]
      //
      // q is the acceleration noise intensity.
      const float q = std::max(m_config.accelerationProcessNoise, 0.0f);

      const float dt2 = dt * dt;
      const float dt3 = dt2 * dt;

      Q.m_d[0][0] = q * dt3 / 3.0f;
      Q.m_d[0][1] = q * dt2 / 2.0f;
      Q.m_d[1][0] = Q.m_d[0][1];
      Q.m_d[1][1] = q * dt;
   }

   void SymmetrizeCovariance()
   {
      const float p01 = 0.5f * (m_P.m_d[0][1] + m_P.m_d[1][0]);

      m_P.m_d[0][1] = p01;
      m_P.m_d[1][0] = p01;

      m_P.m_d[0][0] = std::max(m_P.m_d[0][0], 1.0e-12f);
      m_P.m_d[1][1] = std::max(m_P.m_d[1][1], 1.0e-12f);

      // Keep unused Matrix3 area clean.
      m_P.m_d[0][2] = 0.0f;
      m_P.m_d[1][2] = 0.0f;
      m_P.m_d[2][0] = 0.0f;
      m_P.m_d[2][1] = 0.0f;
      m_P.m_d[2][2] = 0.0f;
   }

   void ApplyPositionLimits()
   {
      if (!m_config.enablePositionLimits)
         return;

      m_x.x = std::clamp(m_x.x, m_config.minPosition, m_config.maxPosition);

      // If the state is clamped against a physical stop, remove velocity
      // pushing further outside the valid range.
      if (m_x.x <= m_config.minPosition && m_x.y < 0.0f)
         m_x.y = 0.0f;

      if (m_x.x >= m_config.maxPosition && m_x.y > 0.0f)
         m_x.y = 0.0f;
   }

   // Predict state forward by dt seconds (internal, no snapshot saved).
   //
   // State:
   //    x = [ position, velocity ]
   //
   // Model:
   //    p' = p + v dt
   //    v' = v
   //
   // Unobserved acceleration is represented through process noise Q.
   void PredictInternal(float timeStep)
   {
      if (!IsFinite(timeStep) || timeStep <= 0.0f)
         return;

      while (timeStep > 0.f)
      {
         const float dt = std::min(timeStep, 0.001f);
         timeStep -= dt;

         // F = [ 1 dt
         //       0 1  ]
         Matrix3 F;
         SetZero(F);

         F.m_d[0][0] = 1.0f;
         F.m_d[0][1] = dt;
         F.m_d[1][0] = 0.0f;
         F.m_d[1][1] = 1.0f;

         // x = F x
         m_x = Mul2(F, m_x);

         // P = F P F^T + Q
         const Matrix3 FP = Mul2(F, m_P);
         Matrix3 FPFt = Mul2RightTranspose(FP, F);

         Matrix3 Q;
         ComputeProcessNoise(dt, Q);

         m_P = Add2(FPFt, Q);

         SymmetrizeCovariance();
         ApplyPositionLimits();
      }
   }

   // Apply a position-only measurement to the current state (no time advance).
   void ApplyMeasurement(const float measuredPosition)
   {
      const float R = std::max(m_config.positionMeasurementVariance, 1.0e-12f);

      // residual y = z - Hx
      const float y = measuredPosition - m_x.x;

      // S = H P H^T + R = P00 + R
      const float S = m_P.m_d[0][0] + R;
      if (!IsFinite(S) || S <= 1.0e-20f)
         return;

      // K = P H^T / S = first column of P / S
      const Vertex2D K { m_P.m_d[0][0] / S, m_P.m_d[1][0] / S };

      // x = x + K y
      m_x += K * y;

      // Symmetric covariance update:
      //
      // P = P - K S K^T
      //
      // This is equivalent to the standard Kalman update for this scalar
      // measurement and keeps the covariance symmetric.
      m_P.m_d[0][0] -= K.x * S * K.x;
      m_P.m_d[0][1] -= K.x * S * K.y;
      m_P.m_d[1][0] -= K.y * S * K.x;
      m_P.m_d[1][1] -= K.y * S * K.y;

      SymmetrizeCovariance();
      ApplyPositionLimits();
   }

private:
   Config m_config;

   // x = [position, velocity]
   //
   // Vertex2D::x = position
   // Vertex2D::y = velocity
   Vertex2D m_x;

   // Top-left 2x2 block is used as covariance.
   Matrix3 m_P;

   // Current filter time, in nanoseconds.
   // 0 means the time base has not been established yet.
   uint64_t m_timeNs = 0ull;

   // Circular buffer of past filter states for rewind support.
   HistoryEntry m_historyBuffer[k_maxHistoryCapacity];

   // Index of the next write slot in m_historyBuffer.
   int m_historyHead = 0;

   // Number of valid entries currently in m_historyBuffer.
   int m_historyCount = 0;
};