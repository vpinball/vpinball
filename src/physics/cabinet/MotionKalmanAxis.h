// license:GPLv3+

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>


namespace VPX::Physics
{

class MotionKalmanAxis final
{
public:
   struct Config
   {
      // Process noise
      float m_processJerkVariance = 800.0f; // true motion responsiveness (high value for low latency)
      float m_velocityBiasProcessVariance = 0.0001f; // velocity bias random walk (slow drift only)
      float m_accelerationBiasProcessVariance = 0.0001f; // acceleration bias random walk (slow drift only)

      // Measurement noise
      float m_velocityMeasurementVariance = 0.05f * 0.05f;
      float m_accelerationMeasurementVariance = 0.40f * 0.40f;

      // Rest pseudo-measurement noise
      float m_zeroPositionMeasurementVariance = 0.0005f * 0.0005f;
      float m_zeroVelocityMeasurementVariance = 0.01f * 0.01f;
      float m_zeroAccelerationMeasurementVariance = 0.10f * 0.10f;

      // Initial covariance
      float m_initialPositionVariance = 1.0e-4f;
      float m_initialVelocityVariance = 1.0f;
      float m_initialAccelerationVariance = 25.0f;
      float m_initialVelocityBiasVariance = 0.01f; // tight: bias starts near zero and drifts slowly
      float m_initialAccelerationBiasVariance = 0.01f; // tight: bias starts near zero and drifts slowly

      // Bias mean-reversion time constant (seconds).
      // Models the bias as an Ornstein-Uhlenbeck process: b(t+dt) = exp(-dt/tau)*b(t) + noise.
      // Any impulse energy absorbed by the bias state drains back to zero over ~tau seconds.
      // Use ~5s for loosely-mounted / environment-coupled sensors, ~30-60s for fixed MEMS.
      float m_biasMeanReversionTimeS = 5.0f;

      // dt bounds
      float m_minDt = 1.0e-6f;
      float m_maxDt = 0.001f;

      // Number of history snapshots retained for rewind support.
      int m_historyCapacity = 64;
   };

   explicit MotionKalmanAxis(const Config& config)
      : m_config(config)
   {
      ClearState();
   }

   void Configure(const Config& config) { m_config = config; }

   const Config& GetConfig() const { return m_config; }

   bool IsInitialized() const { return m_initialized; }

   uint64_t GetTimeNs() const { return m_timeNs; }

   void Reset(uint64_t timeNs, float position = 0.0f, float velocity = 0.0f, float acceleration = 0.0f, float velocityBias = 0.0f, float accelerationBias = 0.0f)
   {
      m_initialized = true;
      m_timeNs = timeNs;

      m_state = Vector5f(position, velocity, acceleration, velocityBias, accelerationBias);

      m_P.SetZero();
      m_P.m[STATE_POSITION][STATE_POSITION] = m_config.m_initialPositionVariance;
      m_P.m[STATE_VELOCITY][STATE_VELOCITY] = m_config.m_initialVelocityVariance;
      m_P.m[STATE_ACCELERATION][STATE_ACCELERATION] = m_config.m_initialAccelerationVariance;
      m_P.m[STATE_VELOCITY_BIAS][STATE_VELOCITY_BIAS] = m_config.m_initialVelocityBiasVariance;
      m_P.m[STATE_ACCELERATION_BIAS][STATE_ACCELERATION_BIAS] = m_config.m_initialAccelerationBiasVariance;

      m_historyHead = 0;
      m_historyCount = 0;
      SaveSnapshot();
   }

   void SetState(float position, float velocity, float acceleration, float velocityBias, float accelerationBias)
   {
      m_state[STATE_POSITION] = position;
      m_state[STATE_VELOCITY] = velocity;
      m_state[STATE_ACCELERATION] = acceleration;
      m_state[STATE_VELOCITY_BIAS] = velocityBias;
      m_state[STATE_ACCELERATION_BIAS] = accelerationBias;
   }

   float GetPosition() const { return m_state[STATE_POSITION]; }

   float GetVelocity() const { return m_state[STATE_VELOCITY]; }

   float GetAcceleration() const { return m_state[STATE_ACCELERATION]; }

   float GetVelocityBias() const { return m_state[STATE_VELOCITY_BIAS]; }

   float GetAccelerationBias() const { return m_state[STATE_ACCELERATION_BIAS]; }

   float GetBiasedVelocity() const { return m_state[STATE_VELOCITY] + m_state[STATE_VELOCITY_BIAS]; }

   float GetBiasedAcceleration() const { return m_state[STATE_ACCELERATION] + m_state[STATE_ACCELERATION_BIAS]; }

   void PredictTo(uint64_t timeNs)
   {
      if (!m_initialized)
         return;

      if (timeNs <= m_timeNs)
         return;

      SaveSnapshot();
      PredictInternal(timeNs);
   }

   void PredictInternal(uint64_t timeNs)
   {
      const uint64_t deltaNs = timeNs - m_timeNs;
      float remainingDt = static_cast<float>(deltaNs) * 1.0e-9f;

      while (remainingDt > 0.0f)
      {
         const float dt = std::min(remainingDt, m_config.m_maxDt);
         PredictStep(dt);
         remainingDt -= dt;
      }

      m_timeNs = timeNs;
   }

   template <typename ApplyMeasurementFunc> void RewindAndUpdate(uint64_t timeNs, ApplyMeasurementFunc applyFunc)
   {
      if (timeNs >= m_timeNs)
      {
         if (timeNs > m_timeNs)
         {
            PredictTo(timeNs);
         }
         applyFunc();
      }
      else
      {
         const uint64_t presentNs = m_timeNs;
         if (!RestoreSnapshot(timeNs))
         {
            // Cannot rewind: apply measurement to current state as a best effort.
            applyFunc();
         }
         else
         {
            if (m_timeNs < timeNs)
            {
               PredictInternal(timeNs);
            }
            applyFunc();
         }

         if (m_timeNs < presentNs)
         {
            SaveSnapshot();
            PredictInternal(presentNs);
         }
      }
   }

   // Real velocity measurement:
   // z = v + b_v + noise
   void UpdateVelocity(uint64_t timeNs, float velocity)
   {
      if (!m_initialized)
      {
         // Practical idle-start assumption:
         // first measured velocity is bias-dominated
         Reset(timeNs, 0.0f, 0.0f, 0.0f, velocity, 0.0f);
         m_P.m[STATE_VELOCITY_BIAS][STATE_VELOCITY_BIAS] = m_config.m_velocityMeasurementVariance;
         UpdateLatestSnapshot();
         return;
      }

      Vector5f H;
      H.SetZero();
      H[STATE_VELOCITY] = 1.0f;
      H[STATE_VELOCITY_BIAS] = 1.0f;

      RewindAndUpdate(timeNs, [&]() { UpdateScalarMeasurement(H, velocity, m_config.m_velocityMeasurementVariance); });
   }

   // Real acceleration measurement:
   // z = a + b_a + noise
   void UpdateAcceleration(uint64_t timeNs, float acceleration)
   {
      if (!m_initialized)
      {
         // Practical idle-start assumption:
         // first measured acceleration is bias-dominated
         Reset(timeNs, 0.0f, 0.0f, 0.0f, 0.0f, acceleration);
         m_P.m[STATE_ACCELERATION_BIAS][STATE_ACCELERATION_BIAS] = m_config.m_accelerationMeasurementVariance;
         UpdateLatestSnapshot();
         return;
      }

      Vector5f H;
      H.SetZero();
      H[STATE_ACCELERATION] = 1.0f;
      H[STATE_ACCELERATION_BIAS] = 1.0f;

      RewindAndUpdate(timeNs, [&]() { UpdateScalarMeasurement(H, acceleration, m_config.m_accelerationMeasurementVariance); });
   }

   // Rest pseudo-measurement: true position = 0
   void UpdateZeroPosition(uint64_t timeNs)
   {
      if (!m_initialized)
         return;

      Vector5f H;
      H.SetZero();
      H[STATE_POSITION] = 1.0f;

      RewindAndUpdate(timeNs, [&]() { UpdateScalarMeasurement(H, 0.0f, m_config.m_zeroPositionMeasurementVariance); });
   }

   // Rest pseudo-measurement: true velocity = 0
   void UpdateZeroVelocity(uint64_t timeNs)
   {
      if (!m_initialized)
         return;

      Vector5f H;
      H.SetZero();
      H[STATE_VELOCITY] = 1.0f;

      RewindAndUpdate(timeNs, [&]() { UpdateScalarMeasurement(H, 0.0f, m_config.m_zeroVelocityMeasurementVariance); });
   }

   // Rest pseudo-measurement: true acceleration = 0
   void UpdateZeroAcceleration(uint64_t timeNs)
   {
      if (!m_initialized)
         return;

      Vector5f H;
      H.SetZero();
      H[STATE_ACCELERATION] = 1.0f;

      RewindAndUpdate(timeNs, [&]() { UpdateScalarMeasurement(H, 0.0f, m_config.m_zeroAccelerationMeasurementVariance); });
   }

   // Convenience method for confirmed rest periods
   void UpdateRestConstraints(uint64_t timeNs, bool applyPositionConstraint = true, bool applyVelocityConstraint = true, bool applyAccelerationConstraint = true)
   {
      if (!m_initialized)
         return;

      if (applyVelocityConstraint)
         UpdateZeroVelocity(timeNs);

      if (applyAccelerationConstraint)
         UpdateZeroAcceleration(timeNs);

      if (applyPositionConstraint)
         UpdateZeroPosition(timeNs);
   }

private:
   enum StateIndex
   {
      STATE_POSITION = 0,
      STATE_VELOCITY = 1,
      STATE_ACCELERATION = 2,
      STATE_VELOCITY_BIAS = 3,
      STATE_ACCELERATION_BIAS = 4,
      STATE_COUNT = 5
   };

   struct Vector5f
   {
      float m_v[STATE_COUNT];

      Vector5f() { SetZero(); }

      Vector5f(float p, float v, float a, float bv, float ba)
      {
         m_v[0] = p;
         m_v[1] = v;
         m_v[2] = a;
         m_v[3] = bv;
         m_v[4] = ba;
      }

      void SetZero()
      {
         for (int i = 0; i < STATE_COUNT; ++i)
            m_v[i] = 0.0f;
      }

      float& operator[](int i) { return m_v[i]; }
      const float& operator[](int i) const { return m_v[i]; }
   };

   struct Matrix5f
   {
      float m[STATE_COUNT][STATE_COUNT];

      Matrix5f() { SetZero(); }

      void SetZero() { std::memset(&m[0][0], 0, sizeof(m)); }

      void SetIdentity()
      {
         SetZero();
         for (int i = 0; i < STATE_COUNT; ++i)
            m[i][i] = 1.0f;
      }
   };

   static Matrix5f Identity()
   {
      Matrix5f m;
      m.SetIdentity();
      return m;
   }

   static Matrix5f Transposed(const Matrix5f& m)
   {
      Matrix5f t;
      for (int i = 0; i < STATE_COUNT; ++i)
         for (int j = 0; j < STATE_COUNT; ++j)
            t.m[i][j] = m.m[j][i];
      return t;
   }

   static Matrix5f Add(const Matrix5f& a, const Matrix5f& b)
   {
      Matrix5f r;
      for (int i = 0; i < STATE_COUNT; ++i)
         for (int j = 0; j < STATE_COUNT; ++j)
            r.m[i][j] = a.m[i][j] + b.m[i][j];
      return r;
   }

   static Matrix5f Mul(const Matrix5f& a, const Matrix5f& b)
   {
      Matrix5f r;
      for (int i = 0; i < STATE_COUNT; ++i)
      {
         for (int j = 0; j < STATE_COUNT; ++j)
         {
            float sum = 0.0f;
            for (int k = 0; k < STATE_COUNT; ++k)
               sum += a.m[i][k] * b.m[k][j];
            r.m[i][j] = sum;
         }
      }
      return r;
   }

   static Vector5f Mul(const Matrix5f& a, const Vector5f& v)
   {
      Vector5f r;
      for (int i = 0; i < STATE_COUNT; ++i)
      {
         float sum = 0.0f;
         for (int j = 0; j < STATE_COUNT; ++j)
            sum += a.m[i][j] * v.m_v[j];
         r[i] = sum;
      }
      return r;
   }

   static float Dot(const Vector5f& a, const Vector5f& b)
   {
      float sum = 0.0f;
      for (int i = 0; i < STATE_COUNT; ++i)
         sum += a.m_v[i] * b.m_v[i];
      return sum;
   }

   static Matrix5f OuterProduct(const Vector5f& a, const Vector5f& b, float scale = 1.0f)
   {
      Matrix5f r;
      for (int i = 0; i < STATE_COUNT; ++i)
         for (int j = 0; j < STATE_COUNT; ++j)
            r.m[i][j] = scale * a.m_v[i] * b.m_v[j];
      return r;
   }

   static void Symmetrize(Matrix5f& m)
   {
      for (int i = 0; i < STATE_COUNT; ++i)
      {
         for (int j = i + 1; j < STATE_COUNT; ++j)
         {
            const float value = 0.5f * (m.m[i][j] + m.m[j][i]);
            m.m[i][j] = value;
            m.m[j][i] = value;
         }
      }
   }

   void ClearState()
   {
      m_initialized = false;
      m_timeNs = 0;
      m_state.SetZero();
      m_P.SetZero();
      m_historyHead = 0;
      m_historyCount = 0;
   }

   Matrix5f BuildTransitionMatrix(float dt) const
   {
      const float dt2 = dt * dt;

      Matrix5f F;
      F.SetIdentity();

      // [p, v, a, b_v, b_a]
      F.m[STATE_POSITION][STATE_VELOCITY] = dt;
      F.m[STATE_POSITION][STATE_ACCELERATION] = 0.5f * dt2;
      F.m[STATE_VELOCITY][STATE_ACCELERATION] = dt;

      // Ornstein-Uhlenbeck mean-reversion for bias states.
      // Instead of a pure random walk (alpha=1), the bias decays exponentially
      // toward zero with time constant tau. This ensures that any impulse energy
      // mistakenly absorbed by the bias drains away over ~tau seconds rather than
      // persisting indefinitely as a DC offset in the output.
      const float tau = m_config.m_biasMeanReversionTimeS;
      const float alpha = (tau > 0.0f) ? expf(-dt / tau) : 1.0f;
      F.m[STATE_VELOCITY_BIAS][STATE_VELOCITY_BIAS] = alpha;
      F.m[STATE_ACCELERATION_BIAS][STATE_ACCELERATION_BIAS] = alpha;

      return F;
   }

   Matrix5f BuildProcessNoiseMatrix(float dt) const
   {
      const float qj = m_config.m_processJerkVariance;
      const float qbv = m_config.m_velocityBiasProcessVariance;
      const float qba = m_config.m_accelerationBiasProcessVariance;

      const float dt2 = dt * dt;
      const float dt3 = dt2 * dt;
      const float dt4 = dt3 * dt;
      const float dt5 = dt4 * dt;

      Matrix5f Q;
      Q.SetZero();

      // Jerk-driven process noise on [p, v, a]
      Q.m[STATE_POSITION][STATE_POSITION] = qj * (dt5 / 20.0f);
      Q.m[STATE_POSITION][STATE_VELOCITY] = qj * (dt4 / 8.0f);
      Q.m[STATE_POSITION][STATE_ACCELERATION] = qj * (dt3 / 6.0f);

      Q.m[STATE_VELOCITY][STATE_POSITION] = qj * (dt4 / 8.0f);
      Q.m[STATE_VELOCITY][STATE_VELOCITY] = qj * (dt3 / 3.0f);
      Q.m[STATE_VELOCITY][STATE_ACCELERATION] = qj * (dt2 / 2.0f);

      Q.m[STATE_ACCELERATION][STATE_POSITION] = qj * (dt3 / 6.0f);
      Q.m[STATE_ACCELERATION][STATE_VELOCITY] = qj * (dt2 / 2.0f);
      Q.m[STATE_ACCELERATION][STATE_ACCELERATION] = qj * dt;

      // Bias random walks
      Q.m[STATE_VELOCITY_BIAS][STATE_VELOCITY_BIAS] = qbv * dt;
      Q.m[STATE_ACCELERATION_BIAS][STATE_ACCELERATION_BIAS] = qba * dt;

      return Q;
   }

   void PredictStep(float dt)
   {
      dt = std::max(dt, m_config.m_minDt);

      const Matrix5f F = BuildTransitionMatrix(dt);
      const Matrix5f Ft = Transposed(F);
      const Matrix5f Q = BuildProcessNoiseMatrix(dt);

      m_state = Mul(F, m_state);
      m_P = Add(Mul(Mul(F, m_P), Ft), Q);
      Symmetrize(m_P);
   }

   void UpdateScalarMeasurement(const Vector5f& H, float measurement, float measurementVariance)
   {
      const float predictedMeasurement = Dot(H, m_state);
      const float innovation = measurement - predictedMeasurement;

      Vector5f PHt;
      for (int i = 0; i < STATE_COUNT; ++i)
      {
         float sum = 0.0f;
         for (int j = 0; j < STATE_COUNT; ++j)
            sum += m_P.m[i][j] * H.m_v[j];
         PHt[i] = sum;
      }

      const float S = Dot(H, PHt) + measurementVariance;
      if (S <= 0.0f)
         return;

      const float invS = 1.0f / S;

      Vector5f K;
      for (int i = 0; i < STATE_COUNT; ++i)
         K[i] = PHt[i] * invS;

      for (int i = 0; i < STATE_COUNT; ++i)
         m_state[i] += K[i] * innovation;

      // Joseph stabilized covariance update:
      // P = (I - K H) P (I - K H)^T + K R K^T
      Matrix5f A = Identity();
      for (int i = 0; i < STATE_COUNT; ++i)
      {
         for (int j = 0; j < STATE_COUNT; ++j)
            A.m[i][j] -= K[i] * H.m_v[j];
      }

      const Matrix5f At = Transposed(A);
      const Matrix5f APAt = Mul(Mul(A, m_P), At);
      const Matrix5f KRKt = OuterProduct(K, K, measurementVariance);

      m_P = Add(APAt, KRKt);
      Symmetrize(m_P);
   }

private:
   Config m_config;

   bool m_initialized = false;
   uint64_t m_timeNs = 0;

   // State = [position, velocity, acceleration, velocityBias, accelerationBias]
   Vector5f m_state;
   Matrix5f m_P;

   // -------------------------------------------------------------------------
   // History buffer for rewind support
   // -------------------------------------------------------------------------

   // Maximum history slots.  The true capacity used is m_config.historyCapacity,
   // clamped to this compile-time upper bound to keep this class allocation-free.
   static constexpr int k_maxHistoryCapacity = 256;

   struct HistoryEntry
   {
      uint64_t timeNs;
      Vector5f state;
      Matrix5f P;
   };

   // Circular buffer of past filter states for rewind support.
   HistoryEntry m_historyBuffer[k_maxHistoryCapacity];

   // Index of the next write slot in m_historyBuffer.
   int m_historyHead = 0;

   // Number of valid entries currently in m_historyBuffer.
   int m_historyCount = 0;

   // Save a snapshot of the current state *before* the next prediction step.
   void SaveSnapshot()
   {
      const int cap = std::max(1, std::min(m_config.m_historyCapacity, k_maxHistoryCapacity));

      HistoryEntry& entry = m_historyBuffer[m_historyHead];
      entry.timeNs = m_timeNs;
      entry.state = m_state;
      entry.P = m_P;

      m_historyHead = (m_historyHead + 1) % cap;
      if (m_historyCount < cap)
         ++m_historyCount;
   }

   // Update the most recently saved snapshot (useful after post-Reset manual updates).
   void UpdateLatestSnapshot()
   {
      if (m_historyCount > 0)
      {
         const int cap = std::max(1, std::min(m_config.m_historyCapacity, k_maxHistoryCapacity));
         const int lastSlot = (m_historyHead - 1 + cap) % cap;
         m_historyBuffer[lastSlot].state = m_state;
         m_historyBuffer[lastSlot].P = m_P;
      }
   }

   // Restore the most recent saved state whose timestamp <= targetNs.
   //
   // Returns true if a suitable entry was found and restored; the filter
   // time is set to that entry's timestamp.
   // Returns false if the buffer is empty (no rewind possible).
   bool RestoreSnapshot(const uint64_t targetNs)
   {
      const int cap = std::max(1, std::min(m_config.m_historyCapacity, k_maxHistoryCapacity));

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
         const uint64_t ts = m_historyBuffer[slot].timeNs;

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
         bestTs = m_historyBuffer[oldestSlot].timeNs;
      }

      m_state = m_historyBuffer[bestSlot].state;
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
};

};
