// license:GPLv3+

#include "Roms.h"

namespace PinMAME
{

Roms::Roms(const string& gameName)
   : m_gameName(gameName)
{
   PinmameGetGameRoms(
      m_gameName.c_str(),
      [](PinmameRom* pRom, void* const pUserData)
      {
         Roms* const roms = static_cast<Roms*>(pUserData);
         roms->m_roms.push_back(new Rom(roms->m_gameName, *pRom));
      },
      this);
}

Roms::~Roms()
{
   for (Rom* const rom : m_roms)
      rom->Release();
}

// VPinMAME's CRoms::get_State aggregation: 0 while a ROM hasn't been audited yet, 1 when all are good, else 2
int32_t Roms::GetState() const
{
   int32_t state = 1;
   for (const Rom* const rom : m_roms)
   {
      const long subState = rom->GetState();
      if (subState == PINMAME_ROM_STATE_UNKNOWN)
         return 0;
      if (subState != PINMAME_ROM_STATE_OK)
         state = 2;
   }
   return state;
}

string Roms::GetStateDescription() const
{
   switch (GetState())
   {
   case 1: return "Ok"s;
   case 2: return "Bad"s;
   default: return "Unknown"s;
   }
}

Rom* Roms::GetItem(const int index)
{
   if ((index < 0) || (index >= static_cast<int>(m_roms.size())))
      return nullptr;
   Rom* const rom = m_roms[index];
   rom->AddRef();
   return rom;
}

Rom* Roms::GetItem(const string& name)
{
   const string key = string_to_lower(name);
   for (Rom* const rom : m_roms)
      if (string_to_lower(rom->GetName()) == key)
      {
         rom->AddRef();
         return rom;
      }
   return nullptr;
}

}
