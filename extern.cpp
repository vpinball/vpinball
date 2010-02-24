#include "stdafx.h"
#include "main.h"

HINSTANCE g_hinst;
HINSTANCE g_hinstres;
ITypeLib *g_ptlMain;
VPinball *g_pvp;
Player *g_pplayer;
HACCEL g_haccel;
WMAudioCreateReaderFunc g_AudioCreateReaderFunc;
BOOL g_fKeepUndoRecords = fTrue;

bool fOldPhys = false;

#ifdef GLOBALLOG
FILE *logfile;
#endif

#ifdef HITLOG
BOOL g_fWriteHitDeleteLog = fTrue;
#endif

ATOM atom;

void ShowError(char *sz)
	{
	MessageBox(g_pvp->m_hwnd, sz, "Error", MB_OK | MB_ICONEXCLAMATION);
	}
void ShowErrorID(int id)
	{
	LocalString ls(id);
	MessageBox(g_pvp->m_hwnd, ls.m_szbuffer, "Error", MB_OK | MB_ICONEXCLAMATION);
	}
