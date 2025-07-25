// license:GPLv3+

#pragma once

#ifdef DEBUG_NUDGE
# define IF_DEBUG_NUDGE(code) code
#else
# define IF_DEBUG_NUDGE(code)
#endif

class NudgeFilter final
{
public:
   NudgeFilter(const char * axis);

   // adjust an acceleration sample
   void sample(float &a, const uint64_t frameTime);

private:
   // debug output
   IF_DEBUG_NUDGE(void dbg(const char *fmt, ...);)
   IF_DEBUG_NUDGE(virtual const char *axis() const = 0;)

   IF_DEBUG_NUDGE(const char *m_axis;)

   // running total of samples
   float m_sum = 0.f;

   // previous sample
   float m_prv = 0.f;

   // timestamp of last zero crossing in the raw acceleration data
   uint64_t m_tzc = 0;

   // timestamp of last correction inserted into the data
   uint64_t m_tCorr = 0;

   // timestamp of last motion == start of rest
   uint64_t m_tMotion = 0;
};
