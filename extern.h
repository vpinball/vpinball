#pragma once

extern HINSTANCE g_hinst;
extern VPinball *g_pvp;
extern Player *g_pplayer; // Game currently being played
extern HACCEL g_haccel; // Accelerator keys
extern BOOL g_fKeepUndoRecords;

#ifdef HITLOG
extern BOOL g_fWriteHitDeleteLog; // For debugging VP 7 crash
#endif

void ShowError(char *sz);
void ExitApp();
