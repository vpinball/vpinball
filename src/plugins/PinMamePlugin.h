// license:GPLv3+

#pragma once

///////////////////////////////////////////////////////////////////////////////
// PinMame plugin
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines the identifiers and messages used by the PinMame controller
// plugin

#define PMPI_NAMESPACE               "PinMAME"

#define PMPI_EVT_ON_GAME_START       "OnGameStart"       // Broadcasted when emulation starts, msgData is a pointer to a PMPI_MSG_ON_GAME_START struct
#define PMPI_EVT_ON_GAME_END         "OnGameEnd"         // Broadcasted when emulation ends

struct PMPI_MSG_ON_GAME_START
{
   const char* vpmPath;
   const char* gameId;
};
