// license:GPLv3+

#pragma once

// Messages of the AltSound plugin

#define ALTSOUNDPI_NAMESPACE "AltSound"

// Sent by the PinMAME plugin before it starts a game, to know which sound command stream the AltSound pack of the game was
// made for. The AltSound plugin answers if it is loaded; without an answer, there is no pack. Message data is an AltSoundPackInfoMsg.
#define ALTSOUNDPI_GET_PACK_INFO_MSG "GetPackInfo:1"

typedef struct AltSoundPackInfoMsg
{
   int version;            // Always 1 (to allow upgrading this message in later revisions)
   const char* gameName;   // Request: game name, that is to say the ROM or alias name given to PinMAME
   int found;              // Response: 1 if AltSound will use a pack for this game, 0 (initial value) if not
   int newSoundCommands;   // Response: 'new_sound_commands' of the pack's altsound.ini. 0 for the sound command stream
                           // and the pairing of the bytes older packs were made with, 1 for the real sound commands
} AltSoundPackInfoMsg;
