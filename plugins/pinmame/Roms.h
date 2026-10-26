// license:GPLv3+

#pragma once

#include "common.h"
#include "plugins/ScriptablePlugin.h"
#include "Rom.h"

namespace PinMAME
{

// The ROM files a game defines, mirroring VPinMAME's IRoms collection
class Roms final
{
public:
   Roms(const string& gameName);
   ~Roms();

   PSC_IMPLEMENT_REFCOUNT()

   void Audit(bool fStrict = false)
   {
      for (Rom* const rom : m_roms)
         rom->Audit(fStrict);
   }
   string GetStateDescription() const;
   int32_t GetState() const;
   bool GetAvailable() const { return PinmameIsGameRomsetAvailable(m_gameName.c_str()) != 0; }
   int32_t GetCount() const { return static_cast<int32_t>(m_roms.size()); }
   Rom* GetItem(int index); // nullptr when out of bounds
   Rom* GetItem(const string& name); // case insensitive, nullptr when not found

private:
   const string m_gameName;
   vector<Rom*> m_roms;
};

}
