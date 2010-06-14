#pragma once

extern HINSTANCE g_hinst;
extern HINSTANCE g_hinstres;
extern ITypeLib *g_ptlMain;
extern VPinball *g_pvp;
extern Player *g_pplayer; // Game currently being played
extern HACCEL g_haccel; // Accelerator keys
extern WMAudioCreateReaderFunc g_AudioCreateReaderFunc;
extern BOOL g_fKeepUndoRecords;

#ifdef GLOBALLOG
extern FILE *logfile;
#endif

#ifdef HITLOG
extern BOOL g_fWriteHitDeleteLog; // For debugging VP 7 crash
#endif

extern ATOM atom;

void ShowError(char *sz);void ShowErrorID(int id);
void ExitApp();
