#include "core/stdafx.h"
#include "StackTrace.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <cstdlib>
#include <strsafe.h>

#if defined(_MSC_VER)
#pragma comment(lib, "dbghelp.lib")
#endif

#if defined(__MINGW32__)
#include <backtrace.h>
#include <cxxabi.h>
#include <cstdio>
#include <cstring>
#endif

namespace
{
#ifdef _WIN64
	constexpr DWORD kStackWalkMachine = IMAGE_FILE_MACHINE_AMD64;
#else
	constexpr DWORD kStackWalkMachine = IMAGE_FILE_MACHINE_I386;
#endif

#if defined(_MSC_VER)
	void GetFileFromPath(const char* path, char* file, int fileNameSize)
	{
		char ext[_MAX_EXT] = {};
		_splitpath_s(path, nullptr, 0, nullptr, 0, file, fileNameSize, ext, _MAX_EXT);
		strncat_s(file, fileNameSize, ext, _MAX_EXT);
	}
#endif

	void InitStackFrameFromContext(PCONTEXT context, STACKFRAME64& stackFrame)
	{
#if defined(_M_ARM64)
#pragma message ( "Warning: No CPU stack debug implemented yet" )
#else
#ifdef _WIN64
		stackFrame.AddrPC.Offset	= context->Rip;
		stackFrame.AddrFrame.Offset = context->Rbp;
		stackFrame.AddrStack.Offset = context->Rsp;
#else
		stackFrame.AddrPC.Offset	= context->Eip;
		stackFrame.AddrFrame.Offset = context->Ebp;
		stackFrame.AddrStack.Offset = context->Esp;
#endif
#endif
	}

	void** GetNextStackFrame(void** prevSP)
	{
		void** newSP = (void**)(*prevSP);
		if (newSP == prevSP)
			return 0;
		// Difference between stack pointers has to be sane.
		if (newSP > prevSP && ((uintptr_t)newSP - (uintptr_t)prevSP) > 1000000)
			return 0;
		if ((uintptr_t)newSP & (sizeof(void*) - 1))
			return 0;

		return newSP;
	}
} // namespace

#if defined(__MINGW32__)
namespace
{
	struct backtrace_state* g_backtraceState = nullptr;

	void BacktraceErrorCallback(void* /*data*/, const char* msg, int errnum)
	{
		char line[MAXSTRING];
		snprintf(line, sizeof(line), "libbacktrace error: %s (%d)\n", msg ? msg : "?", errnum);
		OutputDebugString(line);
	}

	void GetBaseName(const char* path, char* out, size_t outSize)
	{
		const char* base = path;
		for (const char* p = path; *p; ++p)
			if (*p == '/' || *p == '\\')
				base = p + 1;
		snprintf(out, outSize, "%s", base);
	}
}
#endif

namespace rde
{
bool StackTrace::InitSymbols()
{
#if defined(_MSC_VER)
	static bool ls_initialized(false);
	if (!ls_initialized)
	{
		DWORD options = SYMOPT_FAIL_CRITICAL_ERRORS |
						SYMOPT_DEFERRED_LOADS |
						SYMOPT_LOAD_LINES |
						SYMOPT_UNDNAME;
		SymSetOptions(options);
		if (!SymInitialize(GetCurrentProcess(), SDL_GetBasePath(), options & SYMOPT_DEFERRED_LOADS))
		{
			OutputDebugString("Cannot initialize symbol engine");
			return false;
		}

		ls_initialized = true;
	}
	return true;
#elif defined(__MINGW32__)
	if (g_backtraceState == nullptr)
	{
		char exePath[MAXSTRING];
		if (GetModuleFileName(nullptr, exePath, (DWORD)std::size(exePath)) == 0)
			return false;
		g_backtraceState = backtrace_create_state(exePath, 1, BacktraceErrorCallback, nullptr);

		SymSetOptions(SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_DEFERRED_LOADS);
		SymInitialize(GetCurrentProcess(), nullptr, TRUE);
	}
	return g_backtraceState != nullptr;
#else
	return false;
#endif
}

int StackTrace::GetCallStack(Address* callStack, int maxDepth, int entriesToSkip)
{
	PCONTEXT pContext(0);
	const HMODULE hKernel32Dll = GetModuleHandle("kernel32.dll");
	typedef void(WINAPI* pRtlCaptureContext)(PCONTEXT);
	static pRtlCaptureContext RtlCaptureContext = nullptr;
	if(RtlCaptureContext == nullptr)
		RtlCaptureContext = (pRtlCaptureContext)GetProcAddress(hKernel32Dll, "RtlCaptureContext");
	CONTEXT context;
	if (RtlCaptureContext)
	{
		memset(&context, 0, sizeof(context));
		context.ContextFlags = CONTEXT_FULL;
		RtlCaptureContext(&context);
		pContext = &context;
	}
	// +1 -> skip over "us"
	return GetCallStack(pContext, callStack,  maxDepth, entriesToSkip + 1);
}

int StackTrace::GetCallStack(void* vcontext, Address* callStack, int maxDepth,
							 int entriesToSkip)
{
#if defined(_M_ARM64)
#pragma message ( "Warning: No CPU stack debug implemented yet" )
	uintptr_t ebpReg[2];
	uintptr_t espReg;
#else
#ifndef _WIN64
	uintptr_t* ebpReg;
	uintptr_t espReg;
	// clang-format off
	__asm mov [ebpReg], ebp
	__asm mov [espReg], esp
	// clang-format on
#else
	uintptr_t ebpReg[2];
	uintptr_t espReg;
	CONTEXT Context;
	RtlCaptureContext(&Context);
	ebpReg[1] = Context.Rip;
	ebpReg[0] = Context.Rbp;
	espReg = Context.Rsp;
#endif
#endif

	InitSymbols();

	STACKFRAME64 stackFrame = {};

	PCONTEXT context = (PCONTEXT)vcontext;
	if (context == 0)
	{
		stackFrame.AddrPC.Offset	= ebpReg[1];
		stackFrame.AddrFrame.Offset	= ebpReg[0];
		stackFrame.AddrStack.Offset = espReg;
	}
	else
	{
		InitStackFrameFromContext(context, stackFrame);
	}
	stackFrame.AddrPC.Mode		= AddrModeFlat;
	stackFrame.AddrFrame.Mode	= AddrModeFlat;
	stackFrame.AddrStack.Mode	= AddrModeFlat;

	HANDLE process	= GetCurrentProcess();
    HANDLE thread	= GetCurrentThread();

	int numEntries(0);
	while (::StackWalk64(kStackWalkMachine, process, thread,
		&stackFrame, context, 0, SymFunctionTableAccess64, SymGetModuleBase64, nullptr) &&
		stackFrame.AddrFrame.Offset != 0 && numEntries < maxDepth)
	{
		if (entriesToSkip > 0)
			--entriesToSkip;
		else
			callStack[numEntries++] = reinterpret_cast<Address>(stackFrame.AddrPC.Offset);
	}
	return numEntries;
}

int StackTrace::GetCallStack_Fast(Address* callStack, int maxDepth, int entriesToSkip)
{
	uintptr_t ebpReg;
#if defined(_M_ARM64)
#pragma message ( "Warning: No CPU stack debug implemented yet" )
#else
#ifndef _WIN64
	__asm mov [ebpReg], ebp
#else
	CONTEXT Context;
	RtlCaptureContext(&Context);
	ebpReg = Context.Rbp;
#endif
#endif

	void** sp = (void**)ebpReg;
	int numEntries(0);
	while (sp && numEntries < maxDepth)
	{
		if (entriesToSkip > 0)
			--entriesToSkip;
		else
			callStack[numEntries++] = sp[1];

		sp = ::GetNextStackFrame(sp);
	}
	return numEntries;
}

#if defined(__MINGW32__)
namespace
{
	struct ResolveResult
	{
		char* out;
		int remaining;
		int written;
	};

	int BacktraceFullCallback(void* data, uintptr_t pc, const char* filename, int lineno, const char* function)
	{
		ResolveResult* r = (ResolveResult*)data;

		char nameBuf[MAXSTRING] = {};
		const char* name = function;
		if (function)
		{
			int status = 0;
			size_t len = sizeof(nameBuf);
			char* demangled = abi::__cxa_demangle(function, nameBuf, &len, &status);
			if (status == 0 && demangled)
				name = demangled;
		}

		char fileBase[MAXSTRING];
		if (filename)
			GetBaseName(filename, fileBase, sizeof(fileBase));

		int n;
		if (filename && lineno > 0)
			n = snprintf(r->out, r->remaining, " %s %s(%u)", name ? name : "<unknown>", fileBase, (unsigned)lineno);
		else
			n = snprintf(r->out, r->remaining, " %s", name ? name : "<unknown>");

		if (n > 0)
		{
			if (n > r->remaining - 1)
				n = r->remaining - 1;
			r->out += n;
			r->remaining -= n;
			r->written += n;
		}
		return 0;
	}
}

int StackTrace::GetSymbolInfo(Address address, char* symbol, int maxSymbolLen)
{
	if (!InitSymbols())
		return 0;

	int charsAdded = snprintf(symbol, maxSymbolLen, "%p", address);
	if (charsAdded < 0)
		return 0;
	if (charsAdded > maxSymbolLen - 1)
		charsAdded = maxSymbolLen - 1;

	ResolveResult r = { symbol + charsAdded, maxSymbolLen - charsAdded, 0 };
	backtrace_pcinfo(g_backtraceState, (uintptr_t)address, BacktraceFullCallback, BacktraceErrorCallback, &r);
	return charsAdded + r.written;
}
#else
int StackTrace::GetSymbolInfo(Address address, char* symbol, int maxSymbolLen)
{
	if (!InitSymbols())
		return 0;

	// Start with address.
	int charsAdded =
		_snprintf_s(symbol, maxSymbolLen, _TRUNCATE, "%p ", address);
	symbol += charsAdded;
	maxSymbolLen -= charsAdded;
	if (maxSymbolLen < 0)
		return charsAdded;

	const DWORD64 address64 = (DWORD64)address;
	// Module name
	IMAGEHLP_MODULE64 moduleInfo = {};
	moduleInfo.SizeOfStruct = sizeof(moduleInfo);
	const HANDLE hCurrentProcess = GetCurrentProcess();
	if (SymGetModuleInfo64(hCurrentProcess, address64, &moduleInfo))
	{
		char moduleName[MAXSTRING + 1];
		GetFileFromPath(moduleInfo.ImageName, moduleName, std::size(moduleName)-1);
		const int moduleLen = (int)strnlen_s(moduleName,std::size(moduleName));
		strncpy_s(symbol, maxSymbolLen, moduleName);
		symbol += moduleLen;
		charsAdded += moduleLen;
		maxSymbolLen -= moduleLen;
	}
	if (maxSymbolLen <= 0)
		return charsAdded;

	// Symbol name
	ULONG64 symbolBuffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) +
			sizeof(ULONG64) - 1) / sizeof(ULONG64)] = {};
	IMAGEHLP_SYMBOL64* symbolInfo = reinterpret_cast<IMAGEHLP_SYMBOL64*>(symbolBuffer);
	symbolInfo->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	symbolInfo->MaxNameLength = MAX_SYM_NAME;
	DWORD64 disp(0);
	if (SymGetSymFromAddr64(hCurrentProcess, address64, &disp, symbolInfo))
	{
		const int symbolChars =
			_snprintf_s(symbol, maxSymbolLen, _TRUNCATE, " %s + 0x%llX", symbolInfo->Name, disp);
		symbol += symbolChars;
		maxSymbolLen -= symbolChars;
		charsAdded += symbolChars;
	}
	if (maxSymbolLen <= 0)
		return charsAdded;

	// File + line
	DWORD displacementLine;
	IMAGEHLP_LINE64 lineInfo = {};
	lineInfo.SizeOfStruct = sizeof(lineInfo);
	if (SymGetLineFromAddr64(hCurrentProcess, address64, &displacementLine, &lineInfo))
	{
		char fileName[MAXSTRING + 1];
		GetFileFromPath(lineInfo.FileName, fileName, MAXSTRING);
		int fileLineChars;
		if (displacementLine > 0)
		{
			fileLineChars = _snprintf_s(symbol, maxSymbolLen, _TRUNCATE,
				" %s(%u+%04u byte(s))", fileName, lineInfo.LineNumber, displacementLine);
		}
		else
		{
			fileLineChars = _snprintf_s(symbol, maxSymbolLen, _TRUNCATE, 
				" %s(%u)", fileName, lineInfo.LineNumber);
		}
		symbol += fileLineChars;
		maxSymbolLen -= fileLineChars;
		charsAdded += fileLineChars;
	}
	return charsAdded;
}
#endif

void StackTrace::GetCallStack(void* vcontext, bool includeArguments,
							  char* symbol, int maxSymbolLen)
{
	const PCONTEXT context = (PCONTEXT)vcontext;
	if (context == 0)
		return;

	InitSymbols();

	STACKFRAME64 stackFrame = {};

	InitStackFrameFromContext(context, stackFrame);
	stackFrame.AddrPC.Mode		= AddrModeFlat;
	stackFrame.AddrFrame.Mode	= AddrModeFlat;
	stackFrame.AddrStack.Mode	= AddrModeFlat;

	while (maxSymbolLen > 0 &&
		::StackWalk64(kStackWalkMachine,
			::GetCurrentProcess(), ::GetCurrentThread(), &stackFrame,
			context, nullptr, /*Internal_ReadProcessMemory,*/
			SymFunctionTableAccess64, SymGetModuleBase64, nullptr) != FALSE &&
		stackFrame.AddrFrame.Offset != 0)
	{
		const Address addr = reinterpret_cast<Address>(stackFrame.AddrPC.Offset);
		int charsAdded = GetSymbolInfo(addr, symbol, maxSymbolLen);
		maxSymbolLen -= charsAdded;
		symbol += charsAdded;
#if defined(_MSC_VER)
		if (maxSymbolLen > 0 && includeArguments)
		{
			charsAdded = _snprintf_s(symbol, maxSymbolLen, _TRUNCATE,
				" (0x%08llX 0x%08llX 0x%08llX 0x%08llx)\n", stackFrame.Params[0],
				stackFrame.Params[1], stackFrame.Params[2], stackFrame.Params[3]);
			maxSymbolLen -= charsAdded;
			symbol += charsAdded;
		}
#else
		(void)includeArguments;
		if (maxSymbolLen > 1)
		{
			*symbol++ = '\n';
			*symbol = '\0';
			--maxSymbolLen;
		}
#endif
	}
}

}
