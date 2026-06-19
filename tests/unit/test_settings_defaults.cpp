// license:GPLv3+

#include "core/stdafx.h"

#include <doctest/doctest.h>

#include "core/Settings.h"

// The settings property registry is populated at static initialization time, so a freshly
// constructed Settings exposes the compiled-in default configuration without loading or
// generating any ini file (and without needing a display, unlike Load/UpdateDefaults).
TEST_CASE("Settings exposes the compiled-in default configuration")
{
   const Settings settings;

   CHECK(settings.GetEditor_EnableLog() == true);
   CHECK(settings.GetEditor_DisableHash() == false);
   CHECK(settings.GetPlayer_MusicVolume() == 100);
   CHECK(settings.GetPlayer_SoundVolume() == 100);
   CHECK(settings.GetPlayer_PlayMusic() == true);

   // A freshly constructed instance agrees with the static default accessors.
   CHECK(settings.GetPlayer_MusicVolume() == Settings::GetPlayer_MusicVolume_Default());
   CHECK(settings.GetPlayer_PlayfieldWidth() == Settings::GetPlayer_PlayfieldWidth_Default());
}
