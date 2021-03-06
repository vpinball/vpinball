#include "stdafx.h"
#include "CrashHandler.h"
#include "BlackBox.h"
#include "MemoryStatus.h"
#include "StackTrace.h"
#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <cassert>
#include "svn_version.h"

namespace
{
   char s_miniDumpFileName[MAX_PATH] = "crash.dmp";
   char s_reportFileName[MAX_PATH] = "crash.txt";

   void WriteProcessName(FILE* f)
   {
      fprintf(f, "Process: ");
      char buffer[MAX_PATH + 1];
      HMODULE hModule = NULL;
      GetModuleFileName(hModule, buffer, MAX_PATH);
      const char* lastSeparatorPos = strrchr(buffer, '\\');
      if (lastSeparatorPos != 0)
         fprintf(f, "%s", lastSeparatorPos + 1); // +1 -> skip over separator
      else
         fprintf(f, "%s", buffer);
   }

   typedef HRESULT(STDAPICALLTYPE *pRGV)(LPOSVERSIONINFOEXW osi);
   static pRGV mRtlGetVersion = NULL;

   void WriteSystemInfo(FILE* f)
   {
      if (mRtlGetVersion == NULL)
         mRtlGetVersion = (pRGV)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "RtlGetVersion"); // apparently the only really reliable solution to get the OS version (as of Win10 1803)

      DWORD major, minor, build;
      BYTE product;
      if (mRtlGetVersion != NULL) // Windows 10 1803 and above
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
         OSVERSIONINFOEX sysInfo;
         memset(&sysInfo, 0, sizeof(sysInfo));
         sysInfo.dwOSVersionInfoSize = sizeof(sysInfo);
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
      SYSTEM_INFO sysInfo;
      memset(&sysInfo, 0, sizeof(sysInfo));
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
      fprintf(f, "Crash report VPX rev%i\n============\n",SVN_REVISION);
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

   void WriteExceptionInfo(FILE* f, EXCEPTION_POINTERS* exceptionPtrs)
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
            "write to" : "read from"), exceptionPtrs->ExceptionRecord->ExceptionInformation[1]);
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

   bool WriteMiniDump(EXCEPTION_POINTERS* exceptionPtrs, const char* fileName)
   {
      HANDLE hDump = ::CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, 0,
         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
      if (hDump != INVALID_HANDLE_VALUE)
      {
         MINIDUMP_EXCEPTION_INFORMATION dumpInfo = { 0 };
         dumpInfo.ClientPointers = TRUE;
         dumpInfo.ExceptionPointers = exceptionPtrs;
         dumpInfo.ThreadId = ::GetCurrentThreadId();

         MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
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
      fprintf(f, "Total Reserved: %uK (%uM) bytes\n", status.totalReserved >> 10,
         status.totalReserved >> 20);
      fprintf(f, "Total Commited: %uK (%uM) bytes\n", status.totalCommited >> 10,
         status.totalCommited >> 20);
      fprintf(f, "Total Free: %uK (%uM) bytes\n", status.totalFree >> 10,
         status.totalFree >> 20);
      fprintf(f, "Largest Free: %uK (%uM) bytes\n\n", status.largestFree >> 10,
         status.largestFree >> 20);
   }

   void WriteRegisters(FILE* f, EXCEPTION_POINTERS* exceptionPtrs)
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
         ctx->Rax, ctx->Rbx, ctx->Rcx, ctx->Rdx, ctx->Rsi, ctx->Rdi,
         ctx->Rbp, ctx->Rsp, ctx->Rip, ctx->EFlags, ctx->SegCs,
         ctx->SegDs, ctx->SegSs, ctx->SegEs, ctx->SegFs, ctx->SegGs);
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
      char callStack[2048];
      memset(callStack, 0, sizeof(callStack));
      rde::StackTrace::GetCallStack(context, true, callStack, sizeof(callStack) - 1);
      fprintf(f, "Call stack\n==========\n%s\n", callStack);
   }

   volatile unsigned long s_inFilter = 0;

   LONG __stdcall MyExceptionFilter(EXCEPTION_POINTERS* exceptionPtrs)
   {
      LONG returnCode = EXCEPTION_CONTINUE_SEARCH;

      // Ignore multiple calls.
      if (s_inFilter != 0)
         return EXCEPTION_CONTINUE_EXECUTION;
      s_inFilter = 1;

      // Cannot really do much in case of stack overflow, it'll probably bomb soon 
      // anyway.
      if (exceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
      {
         OutputDebugString("*** FATAL ERROR: EXCEPTION_STACK_OVERFLOW detected!");
      }
      const bool miniDumpOK = WriteMiniDump(exceptionPtrs, s_miniDumpFileName);

      FILE* f;
      if ((fopen_s(&f, s_reportFileName, "wt") == 0) && f)
	  {
		  WriteHeader(f);
		  WriteExceptionInfo(f, exceptionPtrs);
		  WriteCallStack(f, exceptionPtrs->ContextRecord);

		  WriteEnvironmentInfo(f);
		  rde::MemoryStatus memStatus = rde::MemoryStatus::GetCurrent();
		  WriteMemoryStatus(f, memStatus);
		  WriteRegisters(f, exceptionPtrs);
		  WriteBlackBoxMessages(f);

		  fprintf(f, (miniDumpOK ? "\nMini dump saved successfully.\n" : "\nFailed to save minidump.\n"));
		  ::fclose(f);
	  }

      return returnCode;
   }
} // namespace

namespace rde
{
   void CrashHandler::Init()
   {
      SetUnhandledExceptionFilter(MyExceptionFilter);
   }

   void CrashHandler::SetMiniDumpFileName(const char* name)
   {
      strncpy_s(s_miniDumpFileName, name, sizeof(s_miniDumpFileName)-1);
   }

   void CrashHandler::SetCrashReportFileName(const char* name)
   {
      strncpy_s(s_reportFileName, name, sizeof(s_reportFileName)-1);
   }
}
