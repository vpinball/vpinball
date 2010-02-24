#define COMPLETE_AUTOSAVE		WM_USER+100
#define COMPLETE_SAVE			WM_USER+101
#define HANG_SNOOP_START		WM_USER+102
#define HANG_SNOOP_STOP			WM_USER+103
#define CREATE_PROGRESS_WINDOW	WM_USER+104
#define DESTROY_PROGRESS_WINDOW	WM_USER+105

#define DONE_AUTOSAVE			WM_USER+200
#define DONE_SAVE				WM_USER+201

extern HANDLE g_hWorkerStarted;

// window for progress meter.  Stored as global so it can be
// shared across threads so one thread can draw it while the
// other is busy and never idles
extern HANDLE g_hProgressWindowStarted;
//extern HWND g_hwndProgress;

class FastIStorage;

class AutoSavePackage
	{
public:
	FastIStorage *pstg;
	int tableindex;
	HWND HwndTable;
	};

DWORD WINAPI VPWorkerThreadStart(void *param);

void CompleteAutoSave(HANDLE hEvent, LPARAM lParam);
