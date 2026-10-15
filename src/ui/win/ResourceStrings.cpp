// license:GPLv3+

#include "core/stdafx.h"
#include "core/def.h"


// Win32 implementations of the LocalString resource loaders, reading the string table of
// src/ui/win/vpinball.rc. The standalone build has no resources and uses the compile-time
// string maps in def.cpp instead.

LocalString::LocalString(const int resid)
{
   m_szbuffer[0] = '\0';
   if (resid > 0)
   {
      // Note that with the char version of LoadString one cannot get a pointer to the internal buffer directly
      /*const int cchar =*/LoadString(GetModuleHandle(nullptr), resid, m_szbuffer, sizeof(m_szbuffer));
      m_szbuffer[std::size(m_szbuffer) - 1] = '\0'; // in case of truncation
   }
}

LocalStringW::LocalStringW(const int resid)
{
   if (resid > 0)
   {
      LPWSTR strPtr = nullptr;
      const int len = LoadStringW(GetModuleHandle(nullptr), resid, reinterpret_cast<LPWSTR>(&strPtr), 0);
      if (len > 0 && strPtr)
         m_buffer = wstring(strPtr, len);
   }
}
