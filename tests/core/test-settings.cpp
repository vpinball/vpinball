// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/Settings.h"

#include "doctest.h"

TEST_CASE("Settings")
{
   SUBCASE("defaults, set and reset")
   {
      Settings settings;
      CHECK(settings.GetPlayer_PlayfieldWidth() == Settings::GetPlayer_PlayfieldWidth_Default());
      CHECK_FALSE(settings.IsModified());

      settings.SetPlayer_PlayfieldWidth(1920, false);
      CHECK(settings.GetPlayer_PlayfieldWidth() == 1920);
      CHECK(settings.IsModified());

      settings.ResetPlayer_PlayfieldWidth();
      CHECK(settings.GetPlayer_PlayfieldWidth() == Settings::GetPlayer_PlayfieldWidth_Default());
   }

   SUBCASE("typed accessors")
   {
      Settings settings;

      settings.SetPlayer_DisableAO(true, false);
      CHECK(settings.GetPlayer_DisableAO() == true);

      settings.SetPlayer_BallTrailStrength(0.75f, false);
      CHECK(settings.GetPlayer_BallTrailStrength() == doctest::Approx(0.75f));

      settings.SetPlayer_BallImage("ball.png"s, false);
      CHECK(settings.GetPlayer_BallImage() == "ball.png");

      const int askToTurnOn = Settings::GetPlayerVR_AskToTurnOn_Default() == 0 ? 1 : 0;
      settings.SetPlayerVR_AskToTurnOn(askToTurnOn, false);
      CHECK(settings.GetPlayerVR_AskToTurnOn() == askToTurnOn);
   }

   SUBCASE("generic accessors match typed accessors")
   {
      Settings settings;
      settings.SetPlayer_PlayfieldWidth(1920, false);
      settings.SetPlayer_DisableAO(true, false);
      CHECK(settings.GetInt(Settings::m_propPlayer_PlayfieldWidth) == 1920);
      CHECK(settings.GetBool(Settings::m_propPlayer_DisableAO) == true);
      CHECK(settings.GetInt(Settings::m_propPlayer_DisableAO) == 1);
   }

   SUBCASE("child settings are table overrides of their parent")
   {
      Settings parent;
      Settings child(&parent);

      parent.SetPlayer_PlayfieldWidth(1920, false);
      CHECK(child.GetPlayer_PlayfieldWidth() == 1920); // child inherits parent's value

      child.SetPlayer_PlayfieldWidth(800, true); // table override only affects the child
      CHECK(child.GetPlayer_PlayfieldWidth() == 800);
      CHECK(parent.GetPlayer_PlayfieldWidth() == 1920);

      child.SetPlayer_PlayfieldWidth(640, false); // not an override: writes to the parent
      CHECK(parent.GetPlayer_PlayfieldWidth() == 640);
      CHECK(child.GetPlayer_PlayfieldWidth() == 640);

      child.SetPlayer_DisableAO(true, true);
      CHECK(child.GetPlayer_DisableAO() == true);
      CHECK(parent.GetPlayer_DisableAO() == Settings::GetPlayer_DisableAO_Default());
   }

   SUBCASE("ini file save/load round-trip")
   {
      const std::filesystem::path iniPath = GetTestTmpDir() / "settings.ini";

      {
         Settings settings;
         settings.SetIniPath(iniPath);
         CHECK(settings.GetIniPath() == iniPath);
         settings.SetPlayer_PlayfieldWidth(1234, false);
         settings.SetPlayer_BallImage("custom-ball.png"s, false);
         settings.Save();
      }
      REQUIRE(FileExists(iniPath));

      Settings loaded;
      loaded.SetIniPath(iniPath);
      CHECK(loaded.Load(false));
      CHECK(loaded.GetPlayer_PlayfieldWidth() == 1234);
      CHECK(loaded.GetPlayer_BallImage() == "custom-ball.png");
   }

   SUBCASE("load fails on missing ini file")
   {
      Settings settings;
      settings.SetIniPath(GetTestTmpDir() / "no-such-settings-file.ini");
      CHECK_FALSE(settings.Load(false));
   }

   SUBCASE("load copies values from another settings store")
   {
      Settings source;
      source.SetPlayer_PlayfieldWidth(1024, false);
      source.SetPlayer_BallTrailStrength(0.9f, false);

      Settings settings;
      settings.Load(source);
      CHECK(settings.GetPlayer_PlayfieldWidth() == 1024);
      CHECK(settings.GetPlayer_BallTrailStrength() == doctest::Approx(0.9f));
   }
}
