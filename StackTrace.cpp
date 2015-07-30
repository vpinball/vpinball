#include "stdafx.h"
#include "StackTrace.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <cstdlib>
#include <strsafe.h>

#pragma comment(lib, "dbghelp.lib")

namespace
{
	void GetFileFromPath(const char* path, char* file, int fileNameSize)
	{
		char ext[_MAX_EXT] = { 0 };
		_splitpath_s(path, 0, 0, 0, 0, file, fileNameSize, ext, _MAX_EXT);
		strncat_s(file, fileNameSize, ext, _MAX_EXT);
	}

	void InitStackFrameFromContext(PCONTEXT context, STACKFRAME64& stackFrame)
	{
#ifdef _WIN64
		stackFrame.AddrPC.Offset	= context->Rip;
		stackFrame.AddrFrame.Offset = context->Rbp;
		stackFrame.AddrStack.Offset = context->Rsp;
#else
		stackFrame.AddrPC.Offset	= context->Eip;
		stackFrame.AddrFrame.Offset = context->Ebp;
		stackFrame.AddrStack.Offset = context->Esp;
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

namespace rde
{
bool StackTrace::InitSymbols()
{
	static bool ls_initialized(false);
	if (!ls_initialized)
	{
		DWORD options = SYMOPT_FAIL_CRITICAL_ERRORS |
						SYMOPT_DEFERRED_LOADS |
						SYMOPT_LOAD_LINES |
						SYMOPT_UNDNAME;
		SymSetOptions(options);
		const char* dir = NULL;
		if (!SymInitialize(GetCurrentProcess(), dir, options & SYMOPT_DEFERRED_LOADS))
		{
			OutputDebugString("Cannot initialize symbol engine");
			return false;
		}

		ls_initialized = true;
	}
	return true;
}

int StackTrace::GetCallStack(Address* callStack, int maxDepth, int entriesToSkip)
{
	PCONTEXT pContext(0);
	HMODULE hKernel32Dll = GetModuleHandle("kernel32.dll");
	void (WINAPI *pRtlCaptureContext)(PCONTEXT);
	*(void**)&pRtlCaptureContext = GetProcAddress(hKernel32Dll, "RtlCaptureContext");
	CONTEXT context;
	if (pRtlCaptureContext)
	{
		memset(&context, 0, sizeof(context));
		context.ContextFlags = CONTEXT_FULL;
		pRtlCaptureContext(&context);
		pContext = &context;
	}
	// +1 -> skip over "us"
	return GetCallStack(pContext, callStack,  maxDepth, entriesToSkip + 1);
}

int StackTrace::GetCallStack(void* vcontext, Address* callStack, int maxDepth, 
							 int entriesToSkip)
{
#ifndef _WIN64
	uintptr_t* ebpReg;
	uintptr_t espReg;
	__asm mov [ebpReg], ebp
	__asm mov [espReg], esp
#else
	uintptr_t ebpReg[2];
	uintptr_t espReg;
    CONTEXT Context;
	RtlCaptureContext(&Context);
	ebpReg[1] = Context.Rip;
	ebpReg[0] = Context.Rbp;
	espReg = Context.Rsp;
#endif

	InitSymbols();

	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(stackFrame));

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
	while (::StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, 
		&stackFrame, context, 0, SymFunctionTableAccess64, SymGetModuleBase64, NULL) &&
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
#ifndef _WIN64
	__asm mov [ebpReg], ebp
#else
    CONTEXT Context;
	RtlCaptureContext(&Context);
	ebpReg = Context.Rbp;
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
	IMAGEHLP_MODULE64 moduleInfo;
	ZeroMemory(&moduleInfo, sizeof(moduleInfo));
	moduleInfo.SizeOfStruct = sizeof(moduleInfo);
	const HANDLE hCurrentProcess = GetCurrentProcess();
	if (SymGetModuleInfo64(hCurrentProcess, address64, &moduleInfo))
	{
		char moduleName[_MAX_PATH + 1];
		GetFileFromPath(moduleInfo.ImageName, moduleName, _MAX_PATH);
		const int moduleLen = (int)strlen(moduleName);
		strncpy_s(symbol, maxSymbolLen, moduleName, _TRUNCATE);
		symbol += moduleLen;
		charsAdded += moduleLen;
		maxSymbolLen -= moduleLen;
	}
	if (maxSymbolLen <= 0)
		return charsAdded;

	// Symbol name
	ULONG64 symbolBuffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) +
			sizeof(ULONG64) - 1) / sizeof(ULONG64)] = { 0 };
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
	IMAGEHLP_LINE64 lineInfo;
	ZeroMemory(&lineInfo, sizeof(lineInfo));
	lineInfo.SizeOfStruct = sizeof(lineInfo);
	if (SymGetLineFromAddr64(hCurrentProcess, address64, &displacementLine, &lineInfo))
	{
		char fileName[_MAX_PATH + 1];
		GetFileFromPath(lineInfo.FileName, fileName, _MAX_PATH);
		int fileLineChars(0);
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

void StackTrace::GetCallStack(void* vcontext, bool includeArguments, 
							  char* symbol, int maxSymbolLen)
{
	PCONTEXT context = (PCONTEXT)vcontext;
	if (context == 0)
		return;

	InitSymbols();

	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(stackFrame));

	InitStackFrameFromContext(context, stackFrame);
	stackFrame.AddrPC.Mode		= AddrModeFlat;
	stackFrame.AddrFrame.Mode	= AddrModeFlat;
	stackFrame.AddrStack.Mode	= AddrModeFlat;

	while (maxSymbolLen > 0 &&
		::StackWalk64(IMAGE_FILE_MACHINE_I386,
			::GetCurrentProcess(), ::GetCurrentThread(), &stackFrame,
			context, NULL, /*Internal_ReadProcessMemory,*/
			SymFunctionTableAccess64, SymGetModuleBase64, NULL) != FALSE &&
		stackFrame.AddrFrame.Offset != 0)
	{
		Address addr = reinterpret_cast<Address>(stackFrame.AddrPC.Offset);
		int charsAdded = GetSymbolInfo(addr, symbol, maxSymbolLen);
		maxSymbolLen -= charsAdded;
		symbol += charsAdded;
		if (maxSymbolLen > 0 && includeArguments)
		{
			charsAdded = _snprintf_s(symbol, maxSymbolLen, _TRUNCATE, 
				" (0x%08llX 0x%08llX 0x%08llX 0x%08llx)\n", stackFrame.Params[0],
				stackFrame.Params[1], stackFrame.Params[2], stackFrame.Params[3]);
			maxSymbolLen -= charsAdded;
			symbol += charsAdded;
		}
	}
}

}
