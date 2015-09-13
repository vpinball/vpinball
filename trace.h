#pragma once

//#define ENABLE_TRACE

////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_TRACE

#include <d3d9.h>

#define _STR(x)    # x
#define STR(x)     _STR(x)
#define _WIDEN(x)  L ## x
#define WIDEN(x)   _WIDEN(x)

#define TRACE_FUNCTION()  ScopedTracer(WIDEN(__FUNCTION__))

class ScopedTracer
{
public:
   ScopedTracer(const WCHAR *name)
   {
      D3DPERF_BeginEvent(RGB(0,255,0), name);
      OutputDebugStringW(name);
   }

   ~ScopedTracer()
   {
      D3DPERF_EndEvent();
   }
};

#else

#define TRACE_FUNCTION()

#endif
