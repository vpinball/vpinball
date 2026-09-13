#include "core/stdafx.h"

#include "CrashHandler.h"
#include "StackTrace.h"
#include "core/vpversion.h"

#if defined(__APPLE__)

#include <csignal>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ucontext.h>
#include <dlfcn.h>

// macOS in-process crash reporter. A fatal signal (SIGSEGV/SIGBUS/SIGILL/
// SIGFPE/SIGABRT) is caught with sigaction and the report is written from the
// signal handler using only write(2)/open/close and stack buffers. Symbol
// resolution (CoreSymbolication + dladdr) may allocate, a pragmatic risk for a
// best-effort in-process reporter.
//
// The report starts at the faulting frame, not our handler. We take a normal
// backtrace() (reliable full walk), drop the leading frames through _sigtramp
// (the kernel signal trampoline, above which are only our handler frames), and
// prepend the ucontext PC as frame 0 since backtrace() omits a leaf that
// faulted without making a call. This mirrors the Windows path starting from
// the exception CONTEXT.

#include <execinfo.h>

namespace
{
   string s_reportFileName = "crash.txt"s;
   string s_miniDumpFileName = "crash.dmp"s; // unused on macOS, kept for API parity

   volatile sig_atomic_t s_inHandler = 0;
   char s_altStack[SIGSTKSZ > 65536 ? SIGSTKSZ : 65536];

   void WriteStr(int fd, const char* s)
   {
      if (s != nullptr)
         (void)!write(fd, s, strlen(s));
   }

   const char* SignalName(int sig)
   {
      switch (sig)
      {
      case SIGSEGV: return "SIGSEGV";
      case SIGBUS:  return "SIGBUS";
      case SIGILL:  return "SIGILL";
      case SIGFPE:  return "SIGFPE";
      case SIGABRT: return "SIGABRT";
      case SIGTRAP: return "SIGTRAP";
      default:      return "signal";
      }
   }

   void WriteHeader(int fd)
   {
      WriteStr(fd, "Crash report VPX - " VP_VERSION_STRING_FULL_LITERAL "\n============\n");
   }

   void WriteExceptionInfo(int fd, int sig, siginfo_t* si)
   {
      char line[MAXSTRING];
      const void* faultAddr = (si != nullptr) ? si->si_addr : nullptr;
      uint64_t tid = 0;
      pthread_threadid_np(nullptr, &tid);
      snprintf(line, sizeof(line), "Reason: %s (signo %d, code %d) at %p\nThread ID: 0x%llx\n\n",
         SignalName(sig), sig, (si != nullptr) ? si->si_code : 0, faultAddr, (unsigned long long)tid);
      WriteStr(fd, line);
   }

   // Build a fault-first stack: frame 0 is the ucontext PC (the exact crashing
   // instruction, which backtrace() drops for a leaf), then the backtrace()
   // frames from just past _sigtramp (dropping our handler and the trampoline).
   // The first post-trampoline frame is skipped when it maps to the same
   // function as the PC, to avoid a duplicate for a mid-function fault.
   int CaptureFromContext(void* uctx, void** out, int max)
   {
      if (max <= 0)
         return 0;

      void* raw[256];
      const int n = backtrace(raw, (int)std::size(raw));

      int start = 0;
      for (int i = 0; i < n; ++i)
      {
         Dl_info di;
         if (dladdr(raw[i], &di) && di.dli_sname != nullptr && strcmp(di.dli_sname, "_sigtramp") == 0)
         {
            start = i + 1;
            break;
         }
      }

      int w = 0;
      const void* pc = nullptr;
      if (uctx != nullptr)
      {
         const ucontext_t* uc = (const ucontext_t*)uctx;
#if defined(__aarch64__)
         pc = (const void*)uc->uc_mcontext->__ss.__pc;
#else
         pc = (const void*)uc->uc_mcontext->__ss.__rip;
#endif
         if (pc != nullptr && w < max)
            out[w++] = (void*)pc;
      }

      // Drop the first captured frame if it is in the same function as the PC.
      if (pc != nullptr && start < n)
      {
         Dl_info a, b;
         if (dladdr(pc, &a) && dladdr(raw[start], &b) && a.dli_saddr == b.dli_saddr)
            ++start;
      }

      for (int i = start; i < n && w < max; ++i)
         out[w++] = raw[i];
      return w;
   }

   void WriteCallStack(int fd, void* const* frames, int n)
   {
      WriteStr(fd, "Call stack\n==========\n");

      // One line per frame: our own frames resolve to symbol + file:line via
      // CoreSymbolication, system frames to module + symbol via dladdr, and
      // anything unresolved to the bare address (all handled by GetSymbolInfo).
      for (int i = 0; i < n; ++i)
      {
         char sym[MAXSTRING];
         const int c = rde::StackTrace::GetSymbolInfo(frames[i], sym, sizeof(sym) - 1);
         if (c > 0)
         {
            sym[c] = '\0';
            WriteStr(fd, sym);
            WriteStr(fd, "\n");
         }
      }
      WriteStr(fd, "\n");
   }

   void WriteReport(int fd, int sig, siginfo_t* si, void* const* frames, int n)
   {
      WriteHeader(fd);
      WriteExceptionInfo(fd, sig, si);
      WriteCallStack(fd, frames, n);
   }

   void CrashSignalHandler(int sig, siginfo_t* si, void* uctx)
   {
      if (s_inHandler)
      {
         signal(sig, SIG_DFL);
         raise(sig);
         return;
      }
      s_inHandler = 1;

      // Capture a fault-first stack (drops this handler and the trampoline).
      void* frames[256];
      const int n = CaptureFromContext(uctx, frames, (int)std::size(frames));

      const int fd = open(s_reportFileName.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (fd >= 0)
      {
         WriteReport(fd, sig, si, frames, n);
         close(fd);
      }

      // Restore default disposition and re-raise so the OS still terminates
      // (and produces its own crash report / core if configured).
      signal(sig, SIG_DFL);
      raise(sig);
   }

   void InstallHandlers()
   {
      stack_t ss = {};
      ss.ss_sp = s_altStack;
      ss.ss_size = sizeof(s_altStack);
      ss.ss_flags = 0;
      sigaltstack(&ss, nullptr);

      struct sigaction sa = {};
      sa.sa_sigaction = CrashSignalHandler;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_SIGINFO | SA_ONSTACK;

      const int signals[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT };
      for (int s : signals)
         sigaction(s, &sa, nullptr);
   }
} // namespace

namespace rde
{
   void CrashHandler::Init()
   {
      // Pre-warm symbol resolution on the main thread (mirrors the MinGW arm).
      rde::StackTrace::InitSymbols();
      InstallHandlers();
   }

   void CrashHandler::SetMiniDumpFileName(const string& name)
   {
      s_miniDumpFileName = name;
   }

   void CrashHandler::SetCrashReportFileName(const string& name)
   {
      s_reportFileName = name;
   }
}

#else // !__APPLE__

#include "BlackBox.h"
#include "MemoryStatus.h"
#include <cstdio>
#include <cstdlib>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <cassert>

namespace
{
   static string s_miniDumpFileName = "crash.dmp"s;
   static string s_reportFileName = "crash.txt"s;

   void WriteProcessName(FILE* f)
   {
      fprintf(f, "Process: ");
      const string buffer = GetExecutablePath();
      const char* lastSeparatorPos = strrchr(buffer.c_str(), PATH_SEPARATOR_CHAR);
      if (lastSeparatorPos != nullptr)
         fprintf(f, "%s", lastSeparatorPos + 1); // +1 -> skip over separator
      else
         fprintf(f, "%s", buffer.c_str());
   }

   typedef HRESULT(STDAPICALLTYPE *pRGV)(LPOSVERSIONINFOEXW osi);
   static pRGV mRtlGetVersion = nullptr;

   void WriteSystemInfo(FILE* f)
   {
      if (mRtlGetVersion == nullptr)
         mRtlGetVersion = (pRGV)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "RtlGetVersion"); // apparently the only really reliable solution to get the OS version (as of Win10 1803)

      DWORD major, minor, build;
      BYTE product;
      if (mRtlGetVersion != nullptr) // Windows 10 1803 and above
      {
         OSVERSIONINFOEXW osInfo;
         osInfo.dwOSVersionInfoSize = sizeof(osInfo);
         mRtlGetVersion(&osInfo);

         major = osInfo.dwMajorVersion;
         minor = osInfo.dwMinorVersion;
         build = osInfo.dwBuildNumber;
         product = osInfo.wProductType;
      }
      else
      {
         OSVERSIONINFOEX sysInfo = {};
         sysInfo.dwOSVersionInfoSize = sizeof(sysInfo);
         #pragma warning(disable: 4996) // warning C4996: 'GetVersionExA': is unadvised but needed for Windows before 8.1 (we support 7)
         ::GetVersionEx((OSVERSIONINFO*)&sysInfo);

         major = sysInfo.dwMajorVersion;
         minor = sysInfo.dwMinorVersion;
         build = sysInfo.dwBuildNumber;
         product = sysInfo.wProductType;
      }

      // We're mainly interested in rough info and latest systems
      fprintf(f, "System: ");
      if (major == 10)
      {
         fprintf(f, "Windows 10 (%lu.%lu %lu%s)\n", major, minor, build, VER_NT_WORKSTATION ? "" : " Server");
      }
      else if (major == 6 && minor == 2)
      {
         if (product != VER_NT_WORKSTATION)
            fprintf(f, "Windows Server 2012 (or above)\n"); //!! as otherwise the manifest should be adapted to target 8.1 or 10
         else
            fprintf(f, "Windows 8 (or above)\n"); //!! as otherwise the manifest should be adapted to target 8.1 or 10
      }
      else if (major == 6 && minor == 1)
      {
         if (product != VER_NT_WORKSTATION)
            fprintf(f, "Windows Server 2008 R2\n");
         else
            fprintf(f, "Windows 7\n");
      }
      else if (major == 6 && minor == 0)
      {
         if (product != VER_NT_WORKSTATION)
            fprintf(f, "Windows Server 2008\n");
         else
            fprintf(f, "Windows Vista\n");
      }
      else if (major == 5)
      {
         if (minor == 2)
            fprintf(f, "Windows Server 2003\n");
         else if (minor == 1)
            fprintf(f, "Windows XP\n");
         else if (minor == 0)
            fprintf(f, "Windows 2000\n");
      }
      else
         fprintf(f, "Unknown Windows version - %lu.%lu (%lu)\n", major, minor, build);
   }

   void WriteProcessorInfo(FILE* f)
   {
      SYSTEM_INFO sysInfo = {};
      GetSystemInfo(&sysInfo);
      fprintf(f, "Number of CPUs: %lu\nProcessor type: %lu\n",
         sysInfo.dwNumberOfProcessors, sysInfo.dwProcessorType);
   }

   void WriteDateTime(FILE* f)
   {
      SYSTEMTIME st;
      ::GetLocalTime(&st);
      fprintf(f, "Date/time: %u/%u/%u, %02u:%02u:%02u:%u\n",
         st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute,
         st.wSecond, st.wMilliseconds);
   }

   void WriteHeader(FILE* f)
   {
      fprintf(f, "Crash report VPX - %s\n============\n", VP_VERSION_STRING_FULL_LITERAL);
   }

   const char* GetExceptionString(DWORD exc)
   {
#define EXC_CASE(EXC)	case EXCEPTION_##EXC : return "EXCEPTION_" #EXC
      switch (exc)
      {
         EXC_CASE(ACCESS_VIOLATION);
         EXC_CASE(DATATYPE_MISALIGNMENT);
         EXC_CASE(BREAKPOINT);
         EXC_CASE(SINGLE_STEP);
         EXC_CASE(ARRAY_BOUNDS_EXCEEDED);
         EXC_CASE(FLT_DENORMAL_OPERAND);
         EXC_CASE(FLT_DIVIDE_BY_ZERO);
         EXC_CASE(FLT_INEXACT_RESULT);
         EXC_CASE(FLT_INVALID_OPERATION);
         EXC_CASE(FLT_OVERFLOW);
         EXC_CASE(FLT_STACK_CHECK);
         EXC_CASE(FLT_UNDERFLOW);
         EXC_CASE(INT_DIVIDE_BY_ZERO);
         EXC_CASE(INT_OVERFLOW);
         EXC_CASE(PRIV_INSTRUCTION);
         EXC_CASE(IN_PAGE_ERROR);
         EXC_CASE(ILLEGAL_INSTRUCTION);
         EXC_CASE(NONCONTINUABLE_EXCEPTION);
         EXC_CASE(STACK_OVERFLOW);
         EXC_CASE(INVALID_DISPOSITION);
         EXC_CASE(GUARD_PAGE);
         EXC_CASE(INVALID_HANDLE);
      default:
         return "UNKNOWN";
      }
#undef EXC_CASE
   }

   void WriteExceptionInfo(FILE* f, const EXCEPTION_POINTERS* exceptionPtrs)
   {
      WriteProcessName(f);
      fprintf(f, "\n");

      fprintf(f, "Reason: 0x%X - %s", exceptionPtrs->ExceptionRecord->ExceptionCode,
         GetExceptionString(exceptionPtrs->ExceptionRecord->ExceptionCode));
#if defined(_M_ARM64)
#pragma message ( "Warning: No CPU exception debug output implemented yet" )
#else
      fprintf(f, " at %04X:%p\n", exceptionPtrs->ContextRecord->SegCs,
         exceptionPtrs->ExceptionRecord->ExceptionAddress);
#endif

      if (exceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
      {
         fprintf(f, "Attempt to %s 0x%08X\n",
            (exceptionPtrs->ExceptionRecord->ExceptionInformation[0] == 1 ?
            "write to" : "read from"), (unsigned int) exceptionPtrs->ExceptionRecord->ExceptionInformation[1]);
      }
      const DWORD threadId = ::GetCurrentThreadId();
      fprintf(f, "Thread ID: 0x%X [%lu]\n\n", threadId, threadId);
   }

   void WriteEnvironmentInfo(FILE* f)
   {
      fprintf(f, "Environment\n===========\n");
      WriteDateTime(f);
      WriteProcessorInfo(f);
      WriteSystemInfo(f);
      fprintf(f, "\n");
   }

   bool WriteMiniDump(EXCEPTION_POINTERS* exceptionPtrs, const string& filename)
   {
      const HANDLE hDump = ::CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0,
         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
      if (hDump != INVALID_HANDLE_VALUE)
      {
         MINIDUMP_EXCEPTION_INFORMATION dumpInfo = {};
         dumpInfo.ClientPointers = TRUE;
         dumpInfo.ExceptionPointers = exceptionPtrs;
         dumpInfo.ThreadId = ::GetCurrentThreadId();

         constexpr MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);

         const BOOL success = ::MiniDumpWriteDump(
            ::GetCurrentProcess(), ::GetCurrentProcessId(), hDump,
            dumpType, &dumpInfo, 0, 0);
         ::CloseHandle(hDump);
         return success == TRUE;
      }
      return false;
   }

   void WriteBlackBoxMessages(FILE* f)
   {
      const int numMessages = rde::BlackBox::GetNumMessages();
      if (numMessages <= 0)
         return;

      fprintf(f, "BlackBox messages\n=================\n");
      for (int i = 0; i < numMessages; ++i)
      {
         int threadId;
         const char* message = rde::BlackBox::GetBoxMessage(i, threadId);
         fprintf(f, "%d: Thread 0x%X: %s\n", i, threadId, message);
      }
   }

   void WriteMemoryStatus(FILE* f, const rde::MemoryStatus& status)
   {
      fprintf(f, "Memory status\n=============\n");
      fprintf(f, "Total Reserved: %uK (%uM) bytes\n", (unsigned int)status.totalReserved >> 10, (unsigned int)status.totalReserved >> 20);
      fprintf(f, "Total Commited: %uK (%uM) bytes\n", (unsigned int)status.totalCommited >> 10, (unsigned int)status.totalCommited >> 20);
      fprintf(f, "Total Free: %uK (%uM) bytes\n", (unsigned int)status.totalFree >> 10, (unsigned int)status.totalFree >> 20);
      fprintf(f, "Largest Free: %uK (%uM) bytes\n\n", (unsigned int)status.largestFree >> 10, (unsigned int)status.largestFree >> 20);
   }

   void WriteRegisters(FILE* f, const EXCEPTION_POINTERS* exceptionPtrs)
   {
      if (IsBadReadPtr(exceptionPtrs, sizeof(EXCEPTION_POINTERS)))
         return;

      const CONTEXT* ctx = exceptionPtrs->ContextRecord;
      fprintf(f, "Registers\n=========\n");
#if defined(_M_ARM64)
#pragma message ( "Warning: No CPU state debug output implemented yet" )
#else
#ifdef _WIN64
      fprintf(f, "RAX=%08X RBX=%08X RCX=%08X RDX=%08X\n" \
         "RSI=%08X RDI=%08X RBP=%08X RSP=%08X RIP=%08X\n" \
         "FLG=%08X CS=%04X DS=%04X SS=%04X ES=%04X FS=%04X GS=%04X\n\n",
         (unsigned int)ctx->Rax, (unsigned int)ctx->Rbx, (unsigned int)ctx->Rcx, (unsigned int)ctx->Rdx, 
         (unsigned int)ctx->Rsi, (unsigned int)ctx->Rdi, (unsigned int)ctx->Rbp, (unsigned int)ctx->Rsp, (unsigned int)ctx->Rip,
         (unsigned int)ctx->EFlags, (unsigned int)ctx->SegCs, (unsigned int)ctx->SegDs, (unsigned int)ctx->SegSs, (unsigned int)ctx->SegEs, (unsigned int)ctx->SegFs, (unsigned int)ctx->SegGs);
#else
      fprintf(f, "EAX=%08X EBX=%08X ECX=%08X EDX=%08X\n" \
         "ESI=%08X EDI=%08X EBP=%08X ESP=%08X EIP=%08X\n" \
         "FLG=%08X CS=%04X DS=%04X SS=%04X ES=%04X FS=%04X GS=%04X\n\n",
         ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx, ctx->Esi, ctx->Edi,
         ctx->Ebp, ctx->Esp, ctx->Eip, ctx->EFlags, ctx->SegCs,
         ctx->SegDs, ctx->SegSs, ctx->SegEs, ctx->SegFs, ctx->SegGs);
#endif
#endif
   }

   void WriteCallStack(FILE* f, PCONTEXT context)
   {
      char callStack[2048] = {};
      rde::StackTrace::GetCallStack(context, true, callStack, sizeof(callStack) - 1);
      fprintf(f, "Call stack\n==========\n%s\n", callStack);
   }

   volatile bool s_inFilter = 0;

   LONG __stdcall MyExceptionFilter(EXCEPTION_POINTERS* exceptionPtrs)
   {
      constexpr LONG returnCode = EXCEPTION_CONTINUE_SEARCH;

      // Ignore multiple calls.
      if (s_inFilter)
         return EXCEPTION_CONTINUE_EXECUTION;
      s_inFilter = true;

      // Cannot really do much in case of stack overflow, it'll probably bomb soon 
      // anyway.
      if (exceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
      {
         OutputDebugString("*** FATAL ERROR: EXCEPTION_STACK_OVERFLOW detected!");
      }
      const bool miniDumpOK = WriteMiniDump(exceptionPtrs, s_miniDumpFileName);

      FILE* f;
      if ((fopen_s(&f, s_reportFileName.c_str(), "wt") == 0) && f)
	  {
		  WriteHeader(f);
		  WriteExceptionInfo(f, exceptionPtrs);
		  WriteCallStack(f, exceptionPtrs->ContextRecord);

		  WriteEnvironmentInfo(f);
		  const rde::MemoryStatus memStatus = rde::MemoryStatus::GetCurrent();
		  WriteMemoryStatus(f, memStatus);
		  WriteRegisters(f, exceptionPtrs);
		  WriteBlackBoxMessages(f);

		  fprintf(f, (miniDumpOK ? "\nMini dump saved successfully.\n" : "\nFailed to save minidump.\n"));
		  ::fclose(f);
	  }

      return returnCode;
   }

#if defined(CRASH_HANDLER) && defined(_MSC_VER)
   void __cdecl PureCallHandler()
   {
      ShowError("Pure Virtual Function Call");

      CONTEXT Context = {};
#ifdef _WIN64
      RtlCaptureContext(&Context);
#else
      Context.ContextFlags = CONTEXT_CONTROL;

      __asm
      {
      Label:
         mov[Context.Ebp], ebp;
         mov[Context.Esp], esp;
         mov eax, [Label];
         mov[Context.Eip], eax;
      }
#endif

      char callStack[2048] = {};
      rde::StackTrace::GetCallStack(&Context, true, callStack, sizeof(callStack) - 1);

      ShowError(callStack);
   }
#endif
} // namespace

namespace rde
{
   void CrashHandler::Init()
   {
      SetUnhandledExceptionFilter(MyExceptionFilter);
#if defined(__MINGW32__)
      // Pre-load symbols on the main thread; libbacktrace loads them lazily and that fails inside a crash on another thread.
      rde::StackTrace::InitSymbols();
#endif
#if defined(CRASH_HANDLER) && defined(_MSC_VER)
      _set_purecall_handler(PureCallHandler);
#endif
   }

   void CrashHandler::SetMiniDumpFileName(const string& name)
   {
      s_miniDumpFileName = name;
   }

   void CrashHandler::SetCrashReportFileName(const string& name)
   {
      s_reportFileName = name;
   }
}

#endif // !__APPLE__
