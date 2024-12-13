#include "core/stdafx.h"

HANDLE g_hWorkerStarted;

static size_t hangsnooptimerid;
static int lasthangsnoopvalue;

VOID CALLBACK HangSnoopProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
   const int newvalue = g_pplayer->m_LastKnownGoodCounter;
   if (newvalue == lasthangsnoopvalue && !g_pplayer->m_ModalRefCount && g_pplayer->IsPlaying(false))
   {
      // Nothing happened since the last time - we are probably hung
      EXCEPINFO eiInterrupt = {};
      const LocalString ls(IDS_HANG);
      const WCHAR * const wzError = MakeWide(ls.m_szbuffer);
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

   PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE); // Create message queue for this thread
   SetEvent(g_hWorkerStarted); // Tell the world we have a message queue to talk to

   for (;;)
   {
      GetMessage(&msg, nullptr, 0, 0);

      switch (msg.message)
      {
      case COMPLETE_AUTOSAVE:
      {
         const HANDLE hEvent = (HANDLE)msg.wParam;
         CompleteAutoSave(hEvent, msg.lParam);
      }
      break;

      case HANG_SNOOP_START:
      {
         lasthangsnoopvalue = -1;
         hangsnooptimerid = SetTimer(nullptr, 0, 1000, (TIMERPROC)HangSnoopProc);
         const HANDLE hEvent = (HANDLE)msg.wParam;
         CloseHandle(hEvent);
         //HangSnoopProc(nullptr, 0, 0, 0);
      }
      break;

      case HANG_SNOOP_STOP:
      {
         KillTimer(nullptr, hangsnooptimerid);
         const HANDLE hEvent = (HANDLE)msg.wParam;
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
   const AutoSavePackage * const pasp = (AutoSavePackage *)lParam;

   FastIStorage * const pstgroot = pasp->pstg;

   const wstring wzT = g_pvp->m_wzMyPath + L"AutoSave" + std::to_wstring(pasp->tableindex) + L".vpx";

   //MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

   STGOPTIONS stg;
   stg.usVersion = 1;
   stg.reserved = 0;
   stg.ulSectorSize = 4096;

   IStorage* pstgDisk;
   HRESULT hr;
   if (SUCCEEDED(hr = StgCreateStorageEx(wzT.c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
      STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgDisk)))
   {
      pstgroot->CopyTo(0, nullptr, nullptr, pstgDisk);
      hr = pstgDisk->Commit(STGC_DEFAULT);
      pstgDisk->Release();
   }

   pstgroot->Release();

   SetEvent(hEvent);

   PostMessage(pasp->hwndtable, DONE_AUTOSAVE, (WPARAM)hEvent, hr);

   delete pasp;
}
