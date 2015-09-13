#pragma once
#define COMPLETE_AUTOSAVE		WM_USER+100
#define COMPLETE_SAVE			WM_USER+101
#define HANG_SNOOP_START		WM_USER+102
#define HANG_SNOOP_STOP			WM_USER+103

#define DONE_AUTOSAVE			WM_USER+200
#define DONE_SAVE				WM_USER+201

extern HANDLE g_hWorkerStarted;

class FastIStorage;

class AutoSavePackage
{
public:
   FastIStorage *pstg;
   int tableindex;
   HWND HwndTable;
};

unsigned int WINAPI VPWorkerThreadStart(void *param);

void CompleteAutoSave(HANDLE hEvent, LPARAM lParam);
