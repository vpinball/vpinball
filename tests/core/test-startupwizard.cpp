// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "ui/live/ingameui/StartupWizardModel.h"

#include "doctest.h"

namespace
{

vector<VPX::Window::DisplayConfig> MakeDisplays(int count, bool portrait = false)
{
   vector<VPX::Window::DisplayConfig> displays;
   for (int i = 0; i < count; ++i)
   {
      VPX::Window::DisplayConfig display {};
      display.display = (SDL_DisplayID)(i + 1);
      display.displayId = std::format("Display{} [{}, 0]", i, i * 1920);
      display.displayName = std::format("Display {}", i + 1);
      display.isPrimary = i == 0;
      display.videomode = portrait ? VPX::Window::VideoMode { 1080, 1920, 1.f, 32, 60.f } : VPX::Window::VideoMode { 1920, 1080, 1.f, 32, 60.f };
      display.left = i * 1920;
      display.top = 0;
      displays.push_back(display);
   }
   return displays;
}

}

using VPX::InGameUI::StartupWizardModel;

TEST_CASE("StartupWizardModel")
{
   SUBCASE("initial mode follows the runtime and main display orientation")
   {
      Settings settings;
      settings.SetPlayer_BGSet(1, false); // A cabinet setting is not an automatic machine-mode indication
      CHECK(StartupWizardModel(settings, MakeDisplays(1), false).GetMainMode() == StartupWizardModel::MainMode::Desktop);
      CHECK(StartupWizardModel(settings, MakeDisplays(1, true), false).GetMainMode() == StartupWizardModel::MainMode::SingleScreen);
      CHECK(StartupWizardModel(settings, MakeDisplays(1), false, true).GetMainMode() == StartupWizardModel::MainMode::VR);
   }

   SUBCASE("desktop preset embeds ancillary outputs")
   {
      Settings settings;
      StartupWizardModel model(settings, MakeDisplays(1), true);
      model.SetMainMode(StartupWizardModel::MainMode::Desktop);

      CHECK(model.GetDraft().GetPlayer_BGSet() == 2);
      CHECK(model.GetDraft().GetWindow_Mode(VPXWindowId::VPXWINDOW_Backglass) == VPX::RenderOutput::OM_EMBEDDED);
      CHECK(model.GetDraft().GetWindow_Mode(VPXWindowId::VPXWINDOW_ScoreView) == VPX::RenderOutput::OM_EMBEDDED);
      CHECK(model.GetDraft().GetWindow_Mode(VPXWindowId::VPXWINDOW_Topper) == VPX::RenderOutput::OM_DISABLED);
      CHECK(model.GetDraft().GetWindow_Display(VPXWindowId::VPXWINDOW_Playfield) == model.GetDisplays()[0].displayId);
   }

   SUBCASE("cabinet preset assigns ancillary outputs to ancillary displays")
   {
      Settings settings;
      settings.SetPlayer_BGSet(0, false);
      StartupWizardModel model(settings, MakeDisplays(2), true);
      model.SetMainMode(StartupWizardModel::MainMode::Cabinet);

      CHECK(model.GetDraft().GetPlayer_BGSet() == 1);
      CHECK(model.GetDraft().GetWindow_Mode(VPXWindowId::VPXWINDOW_Backglass) == VPX::RenderOutput::OM_WINDOW);
      CHECK(model.GetDraft().GetWindow_Display(VPXWindowId::VPXWINDOW_Backglass) == model.GetDisplays()[1].displayId);
      CHECK(model.GetDraft().GetWindow_Mode(VPXWindowId::VPXWINDOW_ScoreView) == VPX::RenderOutput::OM_DISABLED);
   }

   SUBCASE("at least one display stays active")
   {
      Settings settings;
      StartupWizardModel model(settings, MakeDisplays(1), true);
      CHECK_FALSE(model.SetDisplayActive(0, false));
      CHECK(model.IsDisplayActive(0));
   }

   SUBCASE("finish persists only at the end")
   {
      Settings settings;
      settings.SetPlayer_BGSet(0, false);
      const string initialPlayfieldDisplay = settings.GetWindow_Display(VPXWindowId::VPXWINDOW_Playfield);
      StartupWizardModel model(settings, MakeDisplays(1), true);
      model.SetMainMode(StartupWizardModel::MainMode::Desktop);
      model.GetDraft().SetFrontend_TableFolders("D:/Tables"s, false);

      CHECK(settings.GetWindow_Display(VPXWindowId::VPXWINDOW_Playfield) == initialPlayfieldDisplay);
      CHECK(settings.GetFrontend_TableFolders().empty());

      model.Finish(settings);
      CHECK(model.IsFinished());
      CHECK(settings.GetPlayer_BGSet() == 2);
      CHECK(settings.GetPlayer_StartupWizardVersion() == StartupWizardModel::kStartupWizardVersion);
      CHECK(settings.GetFrontend_TableFolders() == "D:/Tables");
      CHECK_FALSE(StartupWizardModel::ShouldAutoOpen(settings));
   }
}
