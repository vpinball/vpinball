#include "core/stdafx.h"
#include "worker.h"

#include "core/VPApp.h"
#include "ui/win/resource.h"
#include "utils/fileio.h"

#include "pole/pole.h"


HANDLE g_hWorkerStarted;

static size_t hangSnoopTimerID;
static int lastHangSnoopValue;

VOID CALLBACK HangSnoopProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
   const int newvalue = g_pplayer->m_lastKnownGoodCounter;
   if (newvalue == lastHangSnoopValue && !g_pplayer->m_modalRefCount && g_pplayer->IsPlaying(false))
   {
      // Nothing happened since the last time - we are probably hung
      EXCEPINFO eiInterrupt = {};
      eiInterrupt.bstrDescription = MakeWideBSTR(LocalStringW(IDS_HANG).m_buffer);
      //eiInterrupt.scode = E_NOTIMPL;
      eiInterrupt.wCode = 2345;
      g_pplayer->m_scriptInterpreter->Stop(g_pplayer->m_ptable, true);
   }
   lastHangSnoopValue = newvalue;
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
         lastHangSnoopValue = -1;
         hangSnoopTimerID = SetTimer(nullptr, 0, 1000, (TIMERPROC)HangSnoopProc);
         const HANDLE hEvent = (HANDLE)msg.wParam;
         CloseHandle(hEvent);
         //HangSnoopProc(nullptr, 0, 0, 0);
      }
      break;

      case HANG_SNOOP_STOP:
      {
         KillTimer(nullptr, hangSnoopTimerID);
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

   InMemStructuredStorage * const pstgroot = pasp->pstg;

   const std::filesystem::path fn = g_app->m_fileLocator.GetTablePath(pasp->table, FileLocator::TableSubFolder::AutoSave, true) / std::format("AutoSave{}.vpx", pasp->tableindex);

   HRESULT hr = E_FAIL;
   POLE::Storage storage(fn.string().c_str());
   if (storage.open(true, true) && storage.result() == POLE::Storage::Ok)
   {
      hr = pstgroot->WriteToStorage(storage) ? S_OK : E_FAIL;
      storage.close();
   }

   delete pstgroot;

   SetEvent(hEvent);

   PostMessage(pasp->hwndtable, DONE_AUTOSAVE, (WPARAM)hEvent, hr);

   delete pasp;
}
