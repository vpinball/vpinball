#pragma once

#define COMPLETE_AUTOSAVE		(WM_USER+100)

#define DONE_AUTOSAVE			(WM_USER+200)

extern HANDLE g_hWorkerStarted;

class InMemStructuredStorage;

class AutoSavePackage final
{
public:
   InMemStructuredStorage *pstg;
   int tableindex;
   class PinTable *table;
   HWND hwndtable;
};

unsigned int WINAPI VPWorkerThreadStart(void *param);

void CompleteAutoSave(HANDLE hEvent, LPARAM lParam);
