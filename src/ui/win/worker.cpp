#include "core/stdafx.h"
#include "worker.h"

#include "core/VPApp.h"
#include "utils/fileio.h"

#include "pole/pole.h"


HANDLE g_hWorkerStarted;

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

   const std::filesystem::path dir = g_app->m_fileLocator.GetTablePath(pasp->table, FileLocator::TableSubFolder::AutoSave, true);
   const std::filesystem::path fn = dir.empty() ? dir : dir / std::format("AutoSave{}.vpx", pasp->tableindex);

   HRESULT hr = E_FAIL;
   if (!fn.empty())
   {
      POLE::Storage storage(POLE::PathToFilename(fn).c_str());
      if (storage.open(true, true, true) && storage.result() == POLE::Storage::Ok)
      {
         const bool written = pstgroot->WriteToStorage(storage);
         storage.close();
         hr = (written && !storage.hasWriteError()) ? S_OK : E_FAIL;
      }
   }

   delete pstgroot;

   SetEvent(hEvent);

   PostMessage(pasp->hwndtable, DONE_AUTOSAVE, (WPARAM)hEvent, hr);

   delete pasp;
}
