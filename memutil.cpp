#include "StdAfx.h"

#ifdef DEBUG_XXX  // remove method in perference to DevPartner

#pragma comment(lib, "dbghelp.lib")
#include "dbghelp.h"

#define STACKLEVEL 5
#define PREEXTRA (STACKLEVEL*sizeof(int))

void __cdecl MemLeakAlert(void * pUserData, size_t nBytes)
{
   int st[5];
   int *pdata = (int *)pUserData;
   char szT[1024];
   for (int i=0;i<5;i++)
   {
      st[i] = pdata[i];
   }
   sprintf_s(szT,"Memory leak at 0x%.8x.\nStack trace:\n0x%.8x\n0x%.8x\n0x%.8x\n0x%.8x\n0x%.8x\n\nWould you like to debug?  (Cancel skips reporting any other leaks)",(int)pUserData+PREEXTRA,st[0],st[1],st[2],st[3],st[4]);
   const int ans = MessageBox(NULL, szT, "Memory Leak", MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON2);
   switch (ans)
   {
   case IDYES:
   {
      //DebugBreak();
      _asm int 3; // Super-secret opcode that makes us break into the debugger.
      // For some reason it works better than DebugBreak - that
      // call gives us a funny call stack which isn't very useful.
   }
   break;
   case IDNO:
      break;
   case IDCANCEL:
      // Cancel our callback - do not bring up an alert for other mem leaks
      _CrtSetDumpClient(NULL);
      break;
   }
}

DWORD CALLBACK SwGetModuleBase(HANDLE dp, DWORD ReturnAddress)
{
   IMAGEHLP_MODULE    ModuleInfo;
   if (SymGetModuleInfo(dp, ReturnAddress, &ModuleInfo))
   {
      return ModuleInfo.BaseOfImage;
   }
   return 0;
}

LPVOID CALLBACK SwFunctionTableAccess(HANDLE dp, DWORD dwPCAddr)
{
   return SymFunctionTableAccess(dp, dwPCAddr);
}

BOOL CALLBACK DoMemoryRead(HANDLE dp, DWORD addr, LPVOID buf, DWORD size, LPDWORD lpcb)
{
   return ReadProcessMemory( dp, (void *)addr, buf, size, lpcb );
}

void * operator new( unsigned int cb )
{
   const int extraspace = 5*sizeof(void*);
   void *res = _malloc_dbg(cb + extraspace, _CLIENT_BLOCK, "", 0);

   HANDLE hProcess = GetCurrentProcess();
   HANDLE hThread = GetCurrentThread();

   CONTEXT stCtx;

   stCtx.ContextFlags = CONTEXT_FULL;

   GetThreadContext(hThread, &stCtx);

   STACKFRAME stFrame;
   ZeroMemory(&stFrame, sizeof(STACKFRAME));
   stFrame.AddrPC.Offset = stCtx.Eip;
   stFrame.AddrPC.Mode = AddrModeFlat; 
   stFrame.AddrStack.Offset = stCtx.Esp; 
   stFrame.AddrStack.Mode = AddrModeFlat; 
   stFrame.AddrFrame.Offset = stCtx.Ebp; 
   stFrame.AddrFrame.Mode = AddrModeFlat; 

   for (int i=0;i<5;i++)
   {
      StackWalk(IMAGE_FILE_MACHINE_I386,
         hProcess, hThread, &stFrame, &stCtx,
         DoMemoryRead, SwFunctionTableAccess, SwGetModuleBase, NULL);

      const DWORD address = stFrame.AddrReturn.Offset;
      ((int *)res)[i] = address;
   }

   //((int *)res)[0] = allocid++;
   //((int *)res)[1] = 0xdeadbeef;

   return (void *)((int)res+extraspace);
}

void *operator new[](std::size_t s)
{
   return (operator new)(s);
}

void operator delete(void * p)
{
   if (p == NULL)
   {
      return;
   }

   const int extraspace = 5*sizeof(void*);

   _free_dbg((void *)((int)p-extraspace), _CLIENT_BLOCK);
}

void operator delete[](void *p) throw()
{
   (operator delete)(p);
}

#endif
