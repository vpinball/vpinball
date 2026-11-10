// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"

#include <bit>

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
      // States are single bits (UNKNOWN = 0, OK = 0x01 ... NEED_DUMP = 0x80), so index by bit position + 1.
      // PINMAME_ROM_STATE is defined in PinMAME's libpinmame.h: adapt this array if states change there
      static_assert(PINMAME_ROM_STATE_UNKNOWN == 0 && PINMAME_ROM_STATE_OK == 0x01 && PINMAME_ROM_STATE_NEED_DUMP == 0x80, "PINMAME_ROM_STATE changed: adapt GetStateDescription");
      static const string descriptions[] = { "Unknown"s, "OK"s, "Need redump"s, "Not found"s, "Not available"s, "Bad checksum"s, "Memory error"s, "Length mismatch"s, "Need dump"s };
      const unsigned int state = static_cast<unsigned int>(m_rom.state);
      if (!std::has_single_bit(state) || state > PINMAME_ROM_STATE_NEED_DUMP)
         return descriptions[0];
      return descriptions[std::countr_zero(state) + 1];
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
