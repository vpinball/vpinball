#pragma once

extern HINSTANCE g_hinst;
extern VPinball *g_pvp;
extern Player *g_pplayer; // Game currently being played
extern HACCEL g_haccel; // Accelerator keys
extern bool g_fKeepUndoRecords;

void ShowError(const char * const sz);
void ExitApp();
