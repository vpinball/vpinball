// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"

namespace PinMAME
{

// A ROM file of a game's romset, mirroring VPinMAME's IRom
class Rom final
{
public:
   Rom(const string& gameName, const PinmameRom& rom)
      : m_rom(rom)
      , m_gameName(gameName)
   {
      m_name = m_rom.name;
   }
   ~Rom() = default;

   PSC_IMPLEMENT_REFCOUNT()

   string GetName() const { return m_name; }
   long GetState() const { return m_rom.state; }
   string GetStateDescription() const
   {
      switch (m_rom.state)
      {
      case PINMAME_ROM_STATE_OK: return "OK"s;
      case PINMAME_ROM_STATE_NEED_REDUMP: return "Need redump"s;
      case PINMAME_ROM_STATE_NOT_FOUND: return "Not found"s;
      case PINMAME_ROM_STATE_NOT_AVAILABLE: return "Not available"s;
      case PINMAME_ROM_STATE_BAD_CHECKSUM: return "Bad checksum"s;
      case PINMAME_ROM_STATE_MEMORY_ERROR: return "Memory error"s;
      case PINMAME_ROM_STATE_LENGTH_MISMATCH: return "Length mismatch"s;
      case PINMAME_ROM_STATE_NEED_DUMP: return "Need dump"s;
      default: return "Unknown"s;
      }
   }
   void Audit(bool fStrict = false) { PinmameAuditGameRom(m_gameName.c_str(), m_name.c_str(), fStrict ? 1 : 0, &m_rom); }
   long GetLength() const { return static_cast<long>(m_rom.length); }
   long GetExpLength() const { return static_cast<long>(m_rom.expLength); }
   long GetChecksum() const { return static_cast<long>(m_rom.checksum); }
   long GetExpChecksum() const { return static_cast<long>(m_rom.expChecksum); }
   long GetFlags() const { return static_cast<long>(m_rom.flags); }

private:
   PinmameRom m_rom;
   const string m_gameName;
   string m_name;
};

}
