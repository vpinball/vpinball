// license:GPLv3+

#include "core/stdafx.h"
#include "NudgeFilter.h"

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
// back and forth, that ultimately canceled out and left the
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
//

NudgeFilter::NudgeFilter(const char * axis) IF_DEBUG_NUDGE(: m_axis(axis))
{
}

// Process a sample.  Adds the sample to the running total, and checks
// to see if a correction should be applied.  Replaces 'a' with the
// corrected value if a correction is needed.
void NudgeFilter::sample(float &a, const U64 frameTime)
{
   IF_DEBUG_NUDGE(char notes[128] = ""; float aIn = a;)

   // if we're not roughly at rest, reset the last motion timer
   if (fabsf(a) >= .02f)
      m_tMotion = frameTime;

   // check for a sign change
   if (fabsf(a) > .01f && fabsf(m_prv) > .01f
      && ((a < 0.f && m_prv > 0.f) || (a > 0.f && m_prv < 0.f)))
   {
      // sign change/zero crossing - note the time
      m_tzc = frameTime;
      IF_DEBUG_NUDGE(strncat_s(notes, "zc ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
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
      IF_DEBUG_NUDGE(strncat_s(notes, "zero ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
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
      IF_DEBUG_NUDGE(strncat_s(notes, "damp ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
         a -= corr;
      m_sum -= corr;

      // Advance the correction time slightly, but not all the
      // way to the present - we want to allow another forced
      // correction soon if necessary to get things back to
      // neutral quickly.
      m_tCorr = frameTime - 40000;
   }

   IF_DEBUG_NUDGE(
      if (a != 0.f || aIn != 0.f)
         dbg(*m_axis == 'x' ? "%f,%f, , ,%s\n" : " , ,%f,%f,%s\n",
         aIn, a, notes);)
}

// debug output
IF_DEBUG_NUDGE(void NudgeFilter::dbg(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   static FILE *fp = 0;
   if (fp == 0) fp = fopen("c:\\joystick.csv", "w");
   vfprintf(fp, fmt, args);
   va_end(args);
})
