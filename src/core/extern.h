// license:GPLv3+

#pragma once

extern class VPApp *g_app;
extern class VPinball *g_pvp;
extern class Player *g_pplayer; // Game currently being played

#ifdef __STANDALONE__
class Standalone;
extern Standalone *g_pStandalone;
#endif
