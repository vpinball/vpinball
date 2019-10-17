#pragma once

extern HINSTANCE g_hinst;
extern VPinball *g_pvp;
extern Player *g_pplayer; // Game currently being played
extern HACCEL g_haccel; // Accelerator keys
extern bool g_keepUndoRecords;

__forceinline void ShowError(const char * const sz)
{
   MessageBox(g_pvp->m_hwnd, sz, "Error", MB_OK | MB_ICONEXCLAMATION);
}

inline void ExitApp()
{
   // Quit nicely.
   if (g_pvp)
      g_pvp->Quit();
   else
      exit(0);
}
