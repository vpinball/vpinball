#include "StdAfx.h"

HANDLE g_hWorkerStarted;

size_t hangsnooptimerid;
int lasthangsnoopvalue;

VOID CALLBACK HangSnoopProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
   const int newvalue = g_pplayer->m_LastKnownGoodCounter;
   if (!g_pplayer->m_fPause && newvalue == lasthangsnoopvalue && !g_pplayer->m_ModalRefCount)
   {
      // Nothing happened since the last time - we are probably hung
      EXCEPINFO eiInterrupt;
      ZeroMemory(&eiInterrupt, sizeof(eiInterrupt));
      LocalString ls(IDS_HANG);
      WCHAR *wzError = MakeWide(ls.m_szbuffer);
      eiInterrupt.bstrDescription = SysAllocString(wzError);
      //eiInterrupt.scode = E_NOTIMPL;
      eiInterrupt.wCode = 2345;
      delete[] wzError;
      /*const HRESULT hr =*/ g_pplayer->m_ptable->m_pcv->m_pScript->InterruptScriptThread(SCRIPTTHREADID_BASE/*SCRIPTTHREADID_ALL*/,
         &eiInterrupt, /*SCRIPTINTERRUPT_DEBUG*/ SCRIPTINTERRUPT_RAISEEXCEPTION);
   }
   lasthangsnoopvalue = newvalue;
}

unsigned int WINAPI VPWorkerThreadStart(void *param)
{
   MSG msg;

   PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE); // Create message queue for this thread
   SetEvent(g_hWorkerStarted); // Tell the world we have a message queue to talk to

   for (;;)
   {
      GetMessage(&msg, NULL, 0, 0);

      switch (msg.message)
      {
      case COMPLETE_AUTOSAVE:
      {
         HANDLE hEvent = (HANDLE)msg.wParam;
         CompleteAutoSave(hEvent, msg.lParam);
      }
      break;

      case HANG_SNOOP_START:
      {
         lasthangsnoopvalue = -1;
         hangsnooptimerid = SetTimer(NULL, 0, 1000, (TIMERPROC)HangSnoopProc);
         HANDLE hEvent = (HANDLE)msg.wParam;
         CloseHandle(hEvent);
         //HangSnoopProc(NULL, 0, 0, 0);
      }
      break;

      case HANG_SNOOP_STOP:
      {
         KillTimer(NULL, hangsnooptimerid);
         HANDLE hEvent = (HANDLE)msg.wParam;
         CloseHandle(hEvent);
      }
      break;

      default:
      {
         DispatchMessage(&msg);
      }
      break;
      }
   }

   return 0;
}

void CompleteAutoSave(HANDLE hEvent, LPARAM lParam)
{
   AutoSavePackage *pasp = (AutoSavePackage *)lParam;

   FastIStorage *pstgroot = pasp->pstg;

   IStorage *pstgDisk;

   WCHAR *wzSaveName = L"AutoSave";
   WCHAR *wzSaveExtension = L".vpx";
   WCHAR wzSuffix[32];
   _itow_s(pasp->tableindex, wzSuffix, sizeof(wzSuffix) / sizeof(WCHAR), 10);
   const int maxLen = MAX_PATH + 32 + lstrlenW(wzSaveName) + lstrlenW(wzSaveExtension) + 1;
   WCHAR * const wzT = new WCHAR[maxLen];

   WideStrNCopy(g_pvp->m_wzMyPath, wzT, maxLen);
   WideStrCat(wzSaveName, wzT);
   WideStrCat(wzSuffix, wzT);
   WideStrCat(wzSaveExtension, wzT);

   //MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

   STGOPTIONS stg;
   stg.usVersion = 1;
   stg.reserved = 0;
   stg.ulSectorSize = 4096;

   HRESULT hr;
   if (SUCCEEDED(hr = StgCreateStorageEx(wzT, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
      STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgDisk)))
   {
      pstgroot->CopyTo(0, NULL, NULL, pstgDisk);
      hr = pstgDisk->Commit(STGC_DEFAULT);
      pstgDisk->Release();
   }

   pstgroot->Release();

   SetEvent(hEvent);

   PostMessage(pasp->HwndTable, DONE_AUTOSAVE, (WPARAM)hEvent, hr);

   delete[] wzT;
   delete pasp;
}
