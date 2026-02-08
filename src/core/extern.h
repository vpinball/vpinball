// license:GPLv3+

#pragma once

// Main application, never null
extern class VPApp *g_app;

// Player, only defined when playing/live editing a table
extern class Player *g_pplayer;

// Win32 editor, only defined when used on Windows Platforms
extern class WinEditor *g_pvp;
