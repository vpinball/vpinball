#include "StdAfx.h"

HINSTANCE g_hinst;
VPinball *g_pvp;
Player *g_pplayer;
HACCEL g_haccel;
BOOL g_fKeepUndoRecords = fTrue;

void ShowError(const char * const sz)
{
   MessageBox(NULL, sz, "Error", MB_OK | MB_ICONEXCLAMATION);
}

void ExitApp()
{
   // Quit nicely.
   if (g_pvp)
   {
      g_pvp->Quit();
   }
   else
   {
      exit(0);
   }
}
