// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "input/ScanCodes.h"

#include "doctest.h"

TEST_CASE("Scan code conversion")
{
   SUBCASE("SDL scancode to DirectInput key")
   {
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_A) == 0x1E);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_Z) == 0x2C);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_1) == 0x02);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_0) == 0x0B);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_RETURN) == 0x1C);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_ESCAPE) == 0x01);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_SPACE) == 0x39);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_F1) == 0x3B);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_LSHIFT) == 0x2A);
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_UP) == 0xC8);
   }

   SUBCASE("invalid SDL scancodes map to zero")
   {
      CHECK(GetDirectInputKeyFromSDLScancode(SDL_SCANCODE_UNKNOWN) == 0);
      CHECK(GetDirectInputKeyFromSDLScancode(static_cast<SDL_Scancode>(-1)) == 0);
      CHECK(GetDirectInputKeyFromSDLScancode(static_cast<SDL_Scancode>(512)) == 0);
   }

   SUBCASE("DirectInput key to SDL scancode")
   {
      CHECK(GetSDLScancodeFromDirectInputKey(0x1E) == SDL_SCANCODE_A);
      CHECK(GetSDLScancodeFromDirectInputKey(0x2C) == SDL_SCANCODE_Z);
      CHECK(GetSDLScancodeFromDirectInputKey(0x39) == SDL_SCANCODE_SPACE);
      CHECK(GetSDLScancodeFromDirectInputKey(0x1C) == SDL_SCANCODE_RETURN);
      CHECK(GetSDLScancodeFromDirectInputKey(0x3B) == SDL_SCANCODE_F1);
   }

   SUBCASE("invalid DirectInput keys map to unknown")
   {
      CHECK(GetSDLScancodeFromDirectInputKey(0xFF) == SDL_SCANCODE_UNKNOWN);
      CHECK(GetSDLScancodeFromDirectInputKey(0xEF) == SDL_SCANCODE_UNKNOWN);
      CHECK(GetSDLScancodeFromDirectInputKey(0x00) == SDL_SCANCODE_UNKNOWN);
   }

   SUBCASE("SDL to DirectInput and back is consistent")
   {
      for (int i = 0; i < 512; ++i)
      {
         const SDL_Scancode scancode = static_cast<SDL_Scancode>(i);
         const unsigned char dik = GetDirectInputKeyFromSDLScancode(scancode);
         if (dik == 0)
            continue;
         // Several scancodes may share the same DIK value, so the reverse lookup
         // returns the first match which must map back to the same DIK.
         const SDL_Scancode back = GetSDLScancodeFromDirectInputKey(dik);
         CHECK(back != SDL_SCANCODE_UNKNOWN);
         CHECK(GetDirectInputKeyFromSDLScancode(back) == dik);
      }
   }

#ifdef WIN32
   SUBCASE("Win32 virtual key to SDL scancode")
   {
      // MapVirtualKey depends on the active keyboard layout: only use keys
      // whose position is identical on all standard layouts (no letters).
      // Arrow keys are skipped: Windows does not set the extended-key bit for
      // them on all systems, which makes this lookup unreliable.
      CHECK(GetSDLScancodeFromWin32VirtualKey(VK_RETURN) == SDL_SCANCODE_RETURN);
      CHECK(GetSDLScancodeFromWin32VirtualKey(VK_SPACE) == SDL_SCANCODE_SPACE);
      CHECK(GetSDLScancodeFromWin32VirtualKey(VK_ESCAPE) == SDL_SCANCODE_ESCAPE);
      CHECK(GetSDLScancodeFromWin32VirtualKey(VK_F1) == SDL_SCANCODE_F1);
      CHECK(GetSDLScancodeFromWin32VirtualKey(VK_LSHIFT) == SDL_SCANCODE_LSHIFT);
   }

   SUBCASE("SDL scancode to Win32 virtual key")
   {
      CHECK(GetWin32VirtualKeyFromSDLScancode(SDL_SCANCODE_RETURN) == VK_RETURN);
      CHECK(GetWin32VirtualKeyFromSDLScancode(SDL_SCANCODE_F1) == VK_F1);
      CHECK(GetWin32VirtualKeyFromSDLScancode(SDL_SCANCODE_ESCAPE) == VK_ESCAPE);
   }
#endif
}
