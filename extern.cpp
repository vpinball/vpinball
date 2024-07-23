#include "stdafx.h"

VPinball *g_pvp = nullptr;
Player *g_pplayer = nullptr;
HACCEL g_haccel;

#ifdef __STANDALONE__
Standalone *g_pStandalone = nullptr;
#endif