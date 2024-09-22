// license:GPLv3+

#pragma once

extern VPinball *g_pvp;
extern class Player *g_pplayer; // Game currently being played
extern HACCEL g_haccel; // Accelerator keys

#ifdef __STANDALONE__
class Standalone;
extern Standalone *g_pStandalone;
#endif
