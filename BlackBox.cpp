#include "stdafx.h"
#include "BlackBox.h"
#include <cstdarg>
#include <cstdio>
#include <intrin.h>		// rdtsc
#include <algorithm>	// sort

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// Very slim version, just to make it compile & run w/o RDE.
template<typename T>
class ScopedPtr
{
public:
   ScopedPtr() : m_ptr(0) {}
   ~ScopedPtr()
   {
      delete[] m_ptr;
      m_ptr = 0;
   }
   void Reset(T* ptr)
   {
      assert(m_ptr == 0);
      assert(ptr != 0);
      m_ptr = ptr;
   }
   T* Get() const			{ return m_ptr; }
   bool operator!() const	{ return !m_ptr; }
private:
   T*	m_ptr;
};

namespace rde
{
   namespace Interlocked
   {
      inline long CompareAndSwap(volatile long* ptr, long comparand,
         long exchange)
      {
         // Arguments order swapped on purpose (Win32 uses xchg/comp, we use comp/xchg).
         return _InterlockedCompareExchange(ptr, exchange, comparand);
      }
   }
}
#define RDE_GET_CURRENT_THREAD_ID	::GetCurrentThreadId

namespace
{
   // This will output thread name next to each message, but may cause slow downs.
   // RDE core required
#define BB_LOG_THREAD_NAME	0
#if BB_LOG_THREAD_NAME && RDE_BLACKBOX_STANDALONE
#	error "RDE core library required for thread name logging support!"
#endif

   typedef char BlackBoxMessage[80];
   struct BlackBoxEntry
   {
      BlackBoxEntry() : threadId(0), ticks(0) {}
      bool operator<(const BlackBoxEntry& rhs) const
      {
         return ticks < rhs.ticks;
      }

      BlackBoxMessage	message;
      int				threadId;
      __int64		ticks;
   };

   const int	kMaxMessages = 128;
   ScopedPtr<BlackBoxEntry>							s_messages;

   volatile long	s_topMessageIndex(0);
   volatile long	s_enabled(1);
   volatile long	s_dirty(0);
   volatile long	s_overflow(0);

   void AddMessageInternal(const char* msg, long i)
   {
      BlackBoxEntry* messages = s_messages.Get();
#if BB_LOG_THREAD_NAME
      const char* threadName = rde::Thread::GetCurrentThreadName();
      messages[i].message[0] = '\0';
      if (threadName != 0)
      {
         strcpy_s(messages[i].message, threadName);
         strcat_s(messages[i].message, ": ");
      }
      strcat_s(messages[i].message, msg);
      messages[i].ticks = __rdtsc();
#else
      strcpy_s(messages[i].message, msg);
      messages[i].threadId = RDE_GET_CURRENT_THREAD_ID();
      messages[i].ticks = __rdtsc();
#endif
      s_dirty = 1;
   }
}

namespace rde
{
   void BlackBox::AddMessage(const char* msg)
   {
      // @note	Potential race on application start, not very dangerous and
      //			rather unlikely.
      if (!s_messages)
         s_messages.Reset(new BlackBoxEntry[kMaxMessages]);

      long top;
      while (true)
      {
         top = s_topMessageIndex;
         const long newTop = (top + 1) & (kMaxMessages - 1);
         if (newTop < top)
            s_overflow = 1;
         if (Interlocked::CompareAndSwap(&s_topMessageIndex, top, newTop) == top)
            break;
      }
      ::AddMessageInternal(msg, top);
   }

   void BlackBox::AddMessagef(const char* fmt, ...)
   {
      if (s_enabled)
      {
         va_list args;
         va_start(args, fmt);
         char buff[sizeof(BlackBoxMessage)];
         memset(buff, 0, sizeof(buff));
         _vsnprintf_s(buff, sizeof(buff) - 1, fmt, args);
         va_end(args);
         AddMessage(buff);
      }
   }

   long BlackBox::GetNumMessages()
   {
      return s_overflow ? kMaxMessages : s_topMessageIndex;
   }

   const char* BlackBox::GetBoxMessage(long index, int& threadId)
   {
      if (s_dirty)
      {
         std::sort(s_messages.Get(), s_messages.Get() + GetNumMessages());
         s_dirty = 0;
      }
      const BlackBoxEntry& be = s_messages.Get()[index];
      threadId = be.threadId;
      return be.message;
   }
}
