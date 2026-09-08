// license:GPLv3+

#pragma once

// Put the calling thread's FPU into flush-to-zero mode.
//
// Each denormal operation costs about an order of
// magnitude more than a normal one on x86-64, and far worse on cores that trap denormals to
// software support code (phones, SBCs).
//
// This is thread local state on x86 (MXCSR) and on ARM (FPCR/FPSCR), so it has to be set on every
// thread doing float work. Do not rely on inheriting it from the creating thread: Linux copies the
// parent's FPU state on clone, but Windows starts every thread from the default MXCSR (0x1F80).
//
// x86:
//   FTZ (MXCSR bit 15): flushes denormal *results* to zero
//   DAZ (MXCSR bit  6): additionally treats denormal *inputs* as zero
// ARM has no such split: FPCR/FPSCR.FZ already covers operands as well as results.

#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1) || defined(__SSE__) || defined(__SSE2__)
   #define DENORMALS_VIA_MXCSR 1
   #include <xmmintrin.h>
   #include <cstring>
   #if defined(_MSC_VER)
      #include <intrin.h>
   #endif
#elif defined(_MSC_VER) && (defined(_M_ARM64) || defined(_M_ARM))
   #define DENORMALS_VIA_CONTROLFP 1
   #include <float.h>
#endif

#if defined(DENORMALS_VIA_MXCSR)
// Does this CPU implement MXCSR.DAZ? DAZ is optional even with SSE2, and setting a reserved MXCSR
// bit faults, so it has to be probed. A zero MXCSR_MASK means the legacy default of 0xFFBF, which
// has DAZ (bit 6) clear.
inline bool denormals_daz_supported()
{
#if defined(_MSC_VER)
   __declspec(align(16)) unsigned char buf[512];
#elif defined(__GNUC__) || defined(__clang__)
   unsigned char buf[512] __attribute__((aligned(16)));
#else
   return false; // no portable way to run FXSAVE here, so skip DAZ and keep FTZ
#endif
#if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
   std::memset(buf, 0, sizeof(buf));
   #if defined(_MSC_VER)
   _fxsave(buf);
   #else
   __asm__ __volatile__("fxsave %0" : "=m"(*(char(*)[512])buf));
   #endif
   unsigned int mask;
   std::memcpy(&mask, buf + 28, sizeof(mask)); // MXCSR_MASK
   if (mask == 0u)
      mask = 0xFFBFu;
   return (mask & 0x0040u) != 0u;
#endif
}
#endif

inline void set_denormals_flush_to_zero()
{
#if defined(DENORMALS_VIA_MXCSR)
   unsigned int csr = _mm_getcsr() | 0x8000u; // FTZ, always available with SSE
   if (denormals_daz_supported())
      csr |= 0x0040u;                         // DAZ, only where implemented
   _mm_setcsr(csr);

#elif defined(DENORMALS_VIA_CONTROLFP)
   // ARM under MSVC, which has no inline asm: FZ covers operands and results alike, and there is no
   // DAZ bit to probe for
   unsigned int old;
   _controlfp_s(&old, _DN_FLUSH, _MCW_DN);

#elif defined(__aarch64__)
   unsigned long long fpcr;
   __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
   fpcr |= (1ull << 24); // FPCR.FZ, operands and results both
   __asm__ __volatile__("msr fpcr, %0" : : "r"(fpcr));

// __ARM_FP (ACLE) is the test for 'VFP instructions are usable', and is absent under
// -mfloat-abi=soft. __VFP_FP__ is NOT usable for this, as clang defines it even for soft-float,
// where these two instructions then fail to assemble. __ARM_PCS_VFP is only a fallback for
// toolchains predating __ARM_FP: it means hard-float ABI, so it is narrower (it misses softfp) but
// never wrong.
#elif defined(__arm__) && (defined(__ARM_FP) || defined(__ARM_PCS_VFP))
   unsigned int fpscr;
   __asm__ __volatile__("vmrs %0, fpscr" : "=r"(fpscr));
   fpscr |= (1u << 24); // FPSCR.FZ, present since VFPv2, so a Pi Zero too
   __asm__ __volatile__("vmsr fpscr, %0" : : "r"(fpscr));

#else
   #pragma message("Warning: No CPU float ignore denorm implemented")
   // nothing safe to set here, everything still works, just slower once values decay to zero
#endif
}

// Same, but for threads we do not create ourselves (the audio callbacks): cheap enough to call on
// every invocation, since the FPU mode only has to be applied once per thread.
inline void set_denormals_flush_to_zero_once()
{
   static thread_local bool done = false;
   if (!done)
   {
      done = true;
      set_denormals_flush_to_zero();
   }
}
