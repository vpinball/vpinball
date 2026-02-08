// license:GPLv3+

#include "core/stdafx.h"
#include "SensorFilter.h"

#ifdef DEBUG_NUDGE
#define IF_DEBUG_NUDGE(code) code
#else
#define IF_DEBUG_NUDGE(code)
#endif



void VelocityToAccelerationFilter::Push(uint64_t timestampNs, float value)
{
   m_target = value;
}

float VelocityToAccelerationFilter::Get(uint64_t timestampNs)
{
   constexpr float dt = static_cast<float>(DEFAULT_STEPTIME_S / 0.001); // Millisecond step (in Visual Pinball Time)

   const uint32_t timestampMs = static_cast<uint32_t>(timestampNs / 1000000);

   // If we lost sync (defined at 30ms as we are supposed to be synced for every ms step), just resync
   if (m_filterTimestampMs + 30 < timestampMs)
      m_filterTimestampMs = timestampMs;

   // Magic numbers below are the result of trial and error to get a good feeling of nudge using velocity integrated nudge (as this is the main usecase)...
   constexpr float smoothingFactor = 0.1f;
   constexpr float smoothingCompensation = 1.0f;

   while (m_filterTimestampMs < timestampMs)
   {
      m_p[1] = m_p[0];
      m_p[0] = lerp(m_p[0], m_target, smoothingFactor); // Steer toward sensor value (low pass filter)

      // Compute velocity (first derivative)
      const float value = (m_p[0] - m_p[1]) * dt;

      // Smooth output (low pass filter)
      m_value = lerp(m_value, value, 0.1f);

      m_filterTimestampMs += 1;
   }

   // To avoid VBS overflows
   return clamp(m_value * smoothingCompensation, -10.f, 10.f);
}


void PositionToAccelerationFilter::Push(uint64_t timestampNs, float value)
{
   m_target = value;
}

float PositionToAccelerationFilter::Get(uint64_t timestampNs)
{
   constexpr float dt = static_cast<float>(DEFAULT_STEPTIME_S / 0.001); // Millisecond step (in Visual Pinball Time)

   const uint32_t timestampMs = static_cast<uint32_t>(timestampNs / 1000000);

   // If we lost sync (defined at 30ms as we are supposed to be synced for every ms step), just resync
   if (m_filterTimestampMs + 30 < timestampMs)
      m_filterTimestampMs = timestampMs;

   // Magic numbers below are the result of trial and error to get a good feeling of nudge useing gamepad controller (as this is the main usecase)...
   constexpr float smoothingFactor = 0.025f;
   constexpr float smoothingCompensation = 2.5f;

   while (m_filterTimestampMs < timestampMs)
   {
      m_p[2] = m_p[1];
      m_p[1] = m_p[0];
      m_p[0] = lerp(m_p[0], m_subTarget, smoothingFactor); // Steer toward sensor value (2 tap low pass filter as we want to avoid steps on second derivative)
      m_subTarget = lerp(m_subTarget, m_target, smoothingFactor);

      // Compute velocities (first derivative)
      const float v1 = (m_p[1] - m_p[2]) * dt;
      const float v0 = (m_p[0] - m_p[1]) * dt;

      // Compute acceleration (second derivative)
      const float value = (v0 - v1) * dt;

      // Smooth output (low pass filter)
      m_value = lerp(m_value, value, smoothingFactor);

      m_filterTimestampMs += 1;
   }

   // To avoid VBS overflows
   return clamp(m_value * smoothingCompensation, -10.f, 10.f);
}



float PlungerPositionFilter::Get(uint64_t timestampNs)
{
   const uint32_t timestampMs = static_cast<uint32_t>(timestampNs / 1000000);

   // If we lost sync (defined at 30ms as we are supposed to be synced for every ms step), just resync
   if (m_filterTimestampMs + 30 < timestampMs)
      m_filterTimestampMs = timestampMs;

   // Advance the filter by steps of 1ms until up to date with game (1KHz sampling, aligned on physics engine)
   while (m_filterTimestampMs < timestampMs)
   {
      m_x[0] = m_source->Get(m_filterTimestampMs * 1000000ull);
      m_y[0] = IIR_a[0] * m_x[0]; // initial
      for (int i = IIR_Order; i > 0; --i) // all terms but the zero-th
      {
         m_y[0] += (IIR_a[i] * m_x[i] - IIR_b[i] * m_y[i]); // /b[0]; always one     // sum terms from high to low
         m_x[i] = m_x[i - 1]; //shift
         m_y[i] = m_y[i - 1]; //shift
      }
      m_filterTimestampMs += 1;
   }

   return m_y[0];
}


// Accelerometer data filter.
//
// This is designed to process the raw acceleration data from a
// physical accelerometer installed in a cabinet to yield more
// realistic effects on the virtual ball.  With a physical
// accelerometer, there are inherent inaccuracies due to small
// measurement errors from the instrument and the finite sampling
// rate.  In addition, the VP simulation only approximates real
// time, so VP can only approximate the duration of each
// instantaneous acceleration - this can exaggerate some inputs
// and under-apply others.  Some of these sources of error are
// random and tend to cancel out over time, but others compound
// over many samples.  In practice there can be noticeable bias
// that causes unrealistic results in the simulation.
//
// This filter is designed to compensate for these various sources
// of error by applying some assumptions about how a real cabinet
// should behave, and adjusting the real input accelerations to
// more closely match how our ideal model cabinet would behave.
//
// The main constraint we use in this filter is net zero motion.
// When you nudge a real cabinet, you make it sway on its legs a
// little, but you don't usually move the cabinet across the
// floor - real cabinets are quite heavy so they tend to stay
// firmly rooted in place during normal play.  So once the swaying
// from a nudge dies out, which happens fairly quickly (in about
// one second, say), the cabinet is back where it started.  This
// means that the cabinet experienced a series of accelerations,
// back and forth, that ultimately cancelled out and left the
// box at rest at its original position.  This is the central
// assumption of this filter: we should be able to add up
// (integrate) the series of instantaneous velocities imparted
// by the instantaneous accelerations over the course of a nudge,
// and we know that at the end, the sum should be zero, because
// the cabinet is back at rest at its starting location.  In
// practice, real accelerometer data for a real nudge event will
// come *close* to netting to zero, but won't quite get there;
// the discrepancy is from the various sources of error described
// above.  This filter tries to reconcile the imperfect measured
// data with our mathematically ideal model by making small
// adjustments to the measured data to get it to match the
// ideal model results.
//
// The point of the filter is to make the results *feel* more
// realistic by reducing visible artifacts from the measurement
// inaccuracies.  We thus have to take care that the filter's
// meedling hand doesn't itself become apparent as another
// visible artifact.  We try to keep the filter's effects subtle
// by trying to limit its intervention to small adjustments.
// For the most part, it kicks in at the point in a nudge where
// the real accelerometer data says things are coming back to rest
// naturally, and simply cleans up the tail end of the nudge
// response to get it to mathematical exactness.  The filter also
// notices a special situation where it shouldn't intervene,
// which is when there's a sustained acceleration in one
// direction.  Assuming that we're not operating under
// extraordinary conditions (e.g., on board an airplane
// accelerating down the runway), a sustained acceleration can
// only mean that someone picked up one end of the cabinet and
// is holding it at an angle.  In this case the ball *should*
// be accelerated in the direction of the tilt, so we don't
// attempt to zero out the net accelerations when we notice this
// type of condition.
//
// It's important to understand that this filter is only useful
// when the nudge inputs are coming from a physical, analog
// accelerometer installed in a standard, free-standing pinball
// cabinet.  The model and the parameters are tailored for this
// particular physical configuration, and it won't give good results
// for other setups.  In particular, don't use this filter with
// "digital" on/off nudge inputs, such as keyboard-based nudging or
// with cabinet sensors based on mercury switches or plumb bobs.
// The nudge accelerations for these sorts of digital nudge inputs
// are simulated, so they're already free of the analog measurement
// errors that this filter is designed to compensate for.
//
// This filter *might* work with real accelerometers that aren't
// in standard cabinets, such as in mini-cabs or desktop controllers.
// It's designed to mimic the physics of a standard cabinet, so
// using it in another physical setup probably wouldn't emulate
// that setup's natural physical behavior.  But the filter might
// give pleasing results anyway simply because every VP setup is
// ultimately meant to simulate the cabinet experience, so you
// probably want the simulation to behave like a cabinet even when
// it's not actually running in a cabinet physically.
float NudgeAccelerationFilter::Get(uint64_t timestampNs)
{
   const uint32_t timestampMs = static_cast<uint32_t>(timestampNs / 1000000);

   // If we lost sync (defined at 30ms as we are supposed to be synced for every ms step), just resync
   if (m_filterTimestampMs + 30 < timestampMs)
      m_filterTimestampMs = timestampMs;

   // Advance the filter by steps of 1ms until up to date with game (1KHz sampling, aligned on physics engine)
   while (m_filterTimestampMs < timestampMs)
   {
      float a = m_source->Get(m_filterTimestampMs * 1000000ull);
      const uint64_t frameTime = m_filterTimestampMs * 1000ull;
      IF_DEBUG_NUDGE(string notes; float aIn = a;)

      // if we're not roughly at rest, reset the last motion timer
      if (fabsf(a) >= .02f)
         m_tMotion = frameTime;

      // check for a sign change
      if (fabsf(a) > .01f && fabsf(m_prv) > .01f
         && ((a < 0.f && m_prv > 0.f) || (a > 0.f && m_prv < 0.f)))
      {
         // sign change/zero crossing - note the time
         m_tzc = frameTime;
         IF_DEBUG_NUDGE(notes += "zc ";)
      }
      else if (fabsf(a) <= .01f)
      {
         // small value -> not a sustained one-way acceleration
         m_tzc = frameTime;
      }
      /*else if (fabsf(a) > .05f && frameTime - m_tzc > 500000) // disabling this fixes an issue with Mot-Ion / Pinball Wizard controllers that suffer from calibration drift as they warm up
      {
         // More than 500 ms in motion with same sign - we must be
         // experiencing a gravitational acceleration due to a tilt
         // of the playfield rather than a transient acceleration
         // from a nudge.  Don't attempt to correct these - clear
         // the sum and do no further processing.
         m_sum = 0;
         IF_DEBUG_NUDGE(dbg("%f >>>\n", a));
         return;
      }*/

      // if this sample is non-zero, remember it as the previous sample
      if (a != 0.f)
         m_prv = a;

      // add this sample to the running total
      m_sum += a;

      // If the running total is near zero, correct it to exactly zero.
      // 
      // Otherwise, if it's been too long since the last correction, or
      // we've been roughly at rest a while, add a damping correction to
      // bring the running total toward rest.
      if (fabsf(m_sum) < .02f)
      {
         // bring the residual acceleration exactly to rest
         IF_DEBUG_NUDGE(notes += "zero ";)
            a -= m_sum;
         m_sum = 0.f;

         // this counts as a zero crossing reset
         m_prv = 0;
         m_tzc = m_tCorr = frameTime;
      }
      else if (frameTime - m_tCorr > 50000 || frameTime - m_tMotion > 50000)
      {
         // bring the running total toward neutral
         const float corr = expf(0.33f*logf(fabsf(m_sum*(float)(1.0 / .02)))) * (m_sum < 0.0f ? -.02f : .02f);
         IF_DEBUG_NUDGE(notes += "damp ";)
            a -= corr;
         m_sum -= corr;

         // Advance the correction time slightly, but not all the
         // way to the present - we want to allow another forced
         // correction soon if necessary to get things back to
         // neutral quickly.
         m_tCorr = frameTime - 40000;
      }

      IF_DEBUG_NUDGE(if (a != 0.f || aIn != 0.f) { PLOGD << " NudgeFilter: " << aIn << " => " << a << ' ' << notes; })

      m_filterTimestampMs += 1;
      m_value = a;
   }
   return m_value;
}
