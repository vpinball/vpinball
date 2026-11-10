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
      , m_name(rom.name)
   {
   }
   ~Rom() = default;

   PSC_IMPLEMENT_REFCOUNT()

   const string& GetName() const { return m_name; }
   int32_t GetState() const { return m_rom.state; }
   const string& GetStateDescription() const
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
   // 32-bit integers for scripts, as in VPinMAME's IRom (a CRC32 above 0x7FFFFFFF reads as negative)
   int32_t GetLength() const { return static_cast<int32_t>(m_rom.length); }
   int32_t GetExpLength() const { return static_cast<int32_t>(m_rom.expLength); }
   int32_t GetChecksum() const { return static_cast<int32_t>(m_rom.checksum); }
   int32_t GetExpChecksum() const { return static_cast<int32_t>(m_rom.expChecksum); }
   int32_t GetFlags() const { return static_cast<int32_t>(m_rom.flags); }

private:
   PinmameRom m_rom;
   const string m_gameName;
   const string m_name;
};

}
