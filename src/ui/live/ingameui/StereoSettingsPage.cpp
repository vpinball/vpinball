// license:GPLv3+

#include "core/stdafx.h"

#include "StereoSettingsPage.h"
#include "renderer/Anaglyph.h"

namespace VPX::InGameUI
{

StereoSettingsPage::StereoSettingsPage()
   : InGameUIPage("Stereo Settings"s, ""s, SaveMode::Both)
{
}

void StereoSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_staticPrepassDisabled = false;
   BuildPage();
}

void StereoSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void StereoSettingsPage::Render(float elapsedS)
{
   if (IsAnaglyphStereoMode(m_editedStereoMode) && m_calibrationStep >= 0)
      UpdateAnaglyphCalibrationModal();
   else
      InGameUIPage::Render(elapsedS);
}

VPX::Properties::PropertyRegistry::PropId StereoSettingsPage::GetCalibratedProperty() const
{
   const int glassesIndex = m_editedStereoMode - STEREO_ANAGLYPH_1;
   switch (m_calibrationStep)
   {
   case 0: return Settings::m_propPlayer_AnaglyphLeftRed[glassesIndex];
   case 1: return Settings::m_propPlayer_AnaglyphLeftGreen[glassesIndex];
   case 2: return Settings::m_propPlayer_AnaglyphLeftBlue[glassesIndex];
   case 3: return Settings::m_propPlayer_AnaglyphRightRed[glassesIndex];
   case 4: return Settings::m_propPlayer_AnaglyphRightGreen[glassesIndex];
   case 5: return Settings::m_propPlayer_AnaglyphRightBlue[glassesIndex];
   default: assert(false); break;
   }
   return {};
}

void StereoSettingsPage::SelectNextItem()
{
   if (IsAnaglyphStereoMode(m_editedStereoMode) && m_calibrationStep >= 0)
   {
      m_calibrationStep++;
      if (m_calibrationStep > 5)
         m_calibrationStep = -1;
   }
   else
      InGameUIPage::SelectNextItem();
}

void StereoSettingsPage::SelectPrevItem()
{
   if (IsAnaglyphStereoMode(m_editedStereoMode) && m_calibrationStep >= 0)
      m_calibrationStep--;
   else
      InGameUIPage::SelectPrevItem();
}

void StereoSettingsPage::AdjustItem(float direction, bool isInitialPress)
{
   if (IsAnaglyphStereoMode(m_editedStereoMode) && m_calibrationStep >= 0)
   {
      float calibrationBrightness = m_player->m_ptable->m_settings.GetFloat(GetCalibratedProperty());
      calibrationBrightness = clamp(calibrationBrightness - direction * 0.01f, 0.f, 1.f);
      m_player->m_ptable->m_settings.Set(GetCalibratedProperty(), calibrationBrightness, false);
   }
   else
      InGameUIPage::AdjustItem(direction, isInitialPress);
}

void StereoSettingsPage::OnPointOfViewChanged()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->UpdateStereoShaderState();
   m_player->m_renderer->InitLayout();
}

void StereoSettingsPage::BuildPage()
{
   ClearItems();

   const bool stereoRT = m_player->m_renderer->m_stereo3D != STEREO_OFF;
   const bool stereoSel = m_player->m_ptable->m_settings.GetPlayer_Stereo3D() != STEREO_OFF;
   m_editedStereoMode = (stereoRT != stereoSel) ? m_player->m_ptable->m_settings.GetPlayer_Stereo3D() : m_player->m_renderer->m_stereo3D;

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   // This is a bit hacky as we can change the stereo mode at runtime if already doing stereo, but not switch it on/off
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3D, //
      [this]()
      {
         const bool stereoRT = m_player->m_renderer->m_stereo3D != STEREO_OFF;
         const bool stereoSel = m_player->m_ptable->m_settings.GetPlayer_Stereo3D() != STEREO_OFF;
         return (stereoRT != stereoSel) ? m_player->m_ptable->m_settings.GetPlayer_Stereo3D() : m_player->m_renderer->m_stereo3D;
      }, //
      [this](int, int v) {
         const bool stereoRT = m_player->m_renderer->m_stereo3D != STEREO_OFF;
         const bool stereoSel = v != STEREO_OFF;
         if (stereoRT != stereoSel)
            m_player->m_liveUI->PushNotification("Toggling stereo rendering will be applied after restarting the game", 5000);
         else
            m_player->m_renderer->m_stereo3D = (StereoMode)v;
         m_player->m_ptable->m_settings.SetPlayer_Stereo3D((StereoMode)v, false);
         OnPointOfViewChanged();
         BuildPage();
      }));

   if (m_editedStereoMode == STEREO_OFF)
      return;

   // FIXME this will conflict with rendering, so just disabled for now
   /* AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_Stereo3DEnabled, //
      [this]() { return m_player->m_renderer->m_stereo3Denabled; }, //
      [this](bool v) {
         m_player->m_renderer->m_stereo3Denabled = v;
         m_player->m_renderer->InitLayout();
         m_player->m_renderer->UpdateStereoShaderState();
         if (m_player->m_renderer->IsUsingStaticPrepass())
         {
            m_player->m_renderer->DisableStaticPrePass(true);
            m_player->m_renderer->DisableStaticPrePass(false);
         }
      })); */

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DEyeSeparation, 1.f, "%4.1f mm"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DEyeSeparation(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DEyeSeparation(v, false);
         OnPointOfViewChanged();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DBrightness, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DBrightness(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DBrightness(v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DSaturation, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DSaturation(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DSaturation(v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   if (!IsAnaglyphStereoMode(m_editedStereoMode))
      return;

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DDefocus, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DDefocus(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DDefocus(v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DLeftContrast, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DLeftContrast(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DLeftContrast(v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Stereo3DRightContrast, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_Stereo3DRightContrast(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_Stereo3DRightContrast(v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   const int glassesIndex = m_editedStereoMode - STEREO_ANAGLYPH_1;
   
   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AnaglyphFilter[glassesIndex], //
      [this, glassesIndex]() { return m_player->m_ptable->m_settings.GetPlayer_AnaglyphFilter(glassesIndex); }, //
      [this, glassesIndex](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AnaglyphFilter(glassesIndex, v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AnaglyphDynDesat[glassesIndex], 100.f, "%4.1f %%"s, //
      [this, glassesIndex]() { return m_player->m_ptable->m_settings.GetPlayer_AnaglyphDynDesat(glassesIndex); }, //
      [this, glassesIndex](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AnaglyphDynDesat(glassesIndex, v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AnaglyphDeghost[glassesIndex], 100.f, "%4.1f %%"s, //
      [this, glassesIndex]() { return m_player->m_ptable->m_settings.GetPlayer_AnaglyphDeghost(glassesIndex); }, //
      [this, glassesIndex](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AnaglyphDeghost(glassesIndex, v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   // TODO this property does not the follow the overall UI design: App/Table/Live state and is directly persisted => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_AnaglyphsRGB[glassesIndex], //
      [this, glassesIndex]() { return m_player->m_ptable->m_settings.GetPlayer_AnaglyphsRGB(glassesIndex); }, //
      [this, glassesIndex](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AnaglyphsRGB(glassesIndex, v, false);
         m_player->m_renderer->UpdateStereoShaderState();
      }));

   AddItem(std::make_unique<InGameUIItem>("Calibrate"s, "Perform calibration to adjust rendering to the characteristic of your display, using your glasses, with your eyes."s,
      [this, glassesIndex]() { m_calibrationStep = 0; }));

   Anaglyph anaglyph;
   anaglyph.LoadSetupFromRegistry(glassesIndex);
   const vec3 eyeL = anaglyph.GetLeftEyeColor(false), eyeR = anaglyph.GetRightEyeColor(false);
   Anaglyph::AnaglyphPair colors = anaglyph.GetColorPair();
   std::stringstream infoText;
   infoText << "Identified as ";
   switch (colors)
   {
   case Anaglyph::RED_CYAN: infoText << (anaglyph.IsReversedColorPair() ? "Cyan/Red" : "Red/Cyan"); break;
   case Anaglyph::GREEN_MAGENTA: infoText << (anaglyph.IsReversedColorPair() ? "Magenta/Green" : "Green/Magenta"); break;
   case Anaglyph::BLUE_AMBER: infoText << (anaglyph.IsReversedColorPair() ? "Amber/Blue" : "Blue/Amber"); break;
   }
   infoText << " - Gamma " << anaglyph.GetDisplayGamma();
   vec3 anaglyphRatio = anaglyph.GetAnaglyphRatio();
   infoText << " - Ratios R: " << (int)(100.f * anaglyphRatio.x) << "% G: " << (int)(100.f * anaglyphRatio.y) << "% B: " << (int)(100.f * anaglyphRatio.z) << '%';
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, infoText.str().c_str()));

   /* ImGui::ColorButton("LeftFilter", ImVec4(eyeL.x, eyeL.y, eyeL.z, 1.f), ImGuiColorEditFlags_NoAlpha);
   ImGui::ColorButton("RightFilter", ImVec4(eyeR.x, eyeR.y, eyeR.z, 1.f), ImGuiColorEditFlags_NoAlpha);*/
}

void StereoSettingsPage::UpdateAnaglyphCalibrationModal()
{
   const int glassesIndex = m_editedStereoMode - STEREO_ANAGLYPH_1;
   const ImGuiIO& io = ImGui::GetIO();
   ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, m_backgroundOpacity));

   ImGui::Begin("Anaglyph Calibration", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
   const ImVec2 win_size = ImGui::GetWindowSize();
   ImDrawList *draw_list = ImGui::GetWindowDrawList();
   const float s = min(win_size.x, win_size.y) / 5.f;
   const float t = 1.f * s;

   const float calibrationBrightness = m_player->m_ptable->m_settings.GetFloat(GetCalibratedProperty());

   /* Initial implementation based on MBD calibration
   draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + t, 0.5f * win_size.y + t),
      ImColor(calibrationBrightness, calibrationBrightness, calibrationBrightness));
   for (int x = 0; x < 2; x++)
   {
      for (int y = 0; y < 2; y++)
      {
         if ((x & 1) != (y & 1))
         {
            ImVec2 pos(0.5f * win_size.x - t + s * x, 0.5f * win_size.y - t + s * y);
            draw_list->AddRectFilled(pos, pos + ImVec2(s, s),ImColor((calibrationStep % 3) == 0 ? 1.f : 0.f, (calibrationStep % 3) == 1 ? 1.f : 0.f, (calibrationStep % 3) == 2 ? 1.f : 0.f));
         }
      }
   }*/

   // Perform calibration using a human face, see https://people.cs.uchicago.edu/~glk/pubs/pdf/Kindlmann-FaceBasedLuminanceMatching-VIS-2002.pdf
   static constexpr int faceLength[] = {9, 4, 7, 5, 5, 4, 4, 5, 4, 4, 4, 5, 4};
   static constexpr ImVec2 face[] = {
      ImVec2( 96.5f, 86.9f), ImVec2( 17.6f,-48.1f), ImVec2(  7.5f, -1.3f), ImVec2( 13.1f, -0.8f), ImVec2( 19.8f,  0.3f), ImVec2(22.5f,  1.6f), ImVec2(-14.2f,51.9f), ImVec2(-25.7f,14.2f), ImVec2(-16.8f,1.1f),
      ImVec2(176.9f, 38.5f), ImVec2( 37.7f, 25.7f), ImVec2( -7.8f, 33.7f), ImVec2(-44.1f, -7.5f),
      ImVec2(120.3f,105.6f), ImVec2(-14.7f, 39.8f), ImVec2( -1.3f, 13.6f), ImVec2( 16.6f,  1.3f), ImVec2( 24.6f,-35.0f), ImVec2( 0.8f,-15.5f), ImVec2( -9.1f,-5.3f),
      ImVec2(120.8f,160.4f), ImVec2( 20.8f, 11.5f), ImVec2( 68.7f,-10.4f), ImVec2(-19.2f,-27.3f), ImVec2(-45.7f, -8.8f),
      ImVec2(116.8f,171.1f), ImVec2( -3.2f, 16.0f), ImVec2(-24.9f, -0.5f), ImVec2( -2.3f,-12.5f), ImVec2(  9.7f, -6.0f),
      ImVec2(116.8f,171.1f), ImVec2( 24.9f,  0.8f), ImVec2( 12.8f, 22.2f), ImVec2(-40.9f, -6.9f),
      ImVec2(141.6f,171.9f), ImVec2( 68.7f,-10.4f), ImVec2(-20.3f, 40.6f), ImVec2(-35.5f, -8.0f),
      ImVec2(154.5f,194.0f), ImVec2( -3.5f,  8.8f), ImVec2(  3.7f, 30.2f), ImVec2( 25.1f,-10.2f), ImVec2( 10.2f,-20.8f),
      ImVec2(151.0f,202.9f), ImVec2(-23.3f, -2.4f), ImVec2(  2.9f, 43.0f), ImVec2( 24.0f,-10.4f),
      ImVec2( 89.0f,194.8f), ImVec2( 38.8f,  5.6f), ImVec2(  2.9f, 43.0f), ImVec2(-38.0f, -5.3f),
      ImVec2(191.1f,134.2f), ImVec2(  5.3f,-19.0f), ImVec2( 17.1f, -1.1f), ImVec2( -3.2f, 47.3f),
      ImVec2( 74.6f,151.9f), ImVec2(  7.9f,-23.8f), ImVec2( 26.5f, -3.0f), ImVec2(-12.9f, 42.9f), ImVec2( -9.7f,  6.0f),
      ImVec2(65.5f, 148.9f), ImVec2(  6.8f,-38.6f), ImVec2( 10.6f, -0.8f), ImVec2( -0.4f, 18.5f),
   };
   ImColor backCol(calibrationBrightness, calibrationBrightness, calibrationBrightness);
   ImColor calCol((m_calibrationStep % 3) == 0 ? 1.f : 0.f, (m_calibrationStep % 3) == 1 ? 1.f : 0.f, (m_calibrationStep % 3) == 2 ? 1.f : 0.f);
   for (int v = 0; v < 2; v++)
   {
      ImVec2 faceTrans[10], faceOffset(win_size.x * 0.5f - 0.5f * t + (float)v * t, win_size.y * 0.5f);
      draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t + (float)v * t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + (float)v * t, 0.5f * win_size.y + t), v == 0 ? backCol : calCol);
      const ImU32 col = ImGui::GetColorU32(v == 1 ? backCol.Value : calCol.Value);
      for (int i = 0, p = 0; i < 13; p += faceLength[i], i++)
      {
         ImVec2 pos(0.f, 0.f);
         for (int j = 0; j < faceLength[i]; j++)
         {
            pos = pos + face[p + j];
            faceTrans[j] = faceOffset + (pos + ImVec2(-140.f, -140.f)) * 2.f * t / 320.f;
         }
         draw_list->AddConvexPolyFilled(faceTrans, faceLength[i], col);
      }
   }

   // Face area
   //draw_list->AddRect(ImVec2((win_size.x - 2.f * t) * 0.5f, win_size.y * 0.5f - t), ImVec2((win_size.x + 2.f * t) * 0.5f, win_size.y * 0.5f + t), IM_COL32_WHITE);

   #define CENTERED_TEXT(y, t) \
      ImGui::SetCursorPos(ImVec2((win_size.x - ImGui::CalcTextSize(t).x) * 0.5f, y)); \
      ImGui::TextUnformatted(t);

   ImGui::PushFont(m_player->m_liveUI->GetOverlayFont(), m_player->m_liveUI->GetOverlayFont()->LegacySize);
   float line_height = ImGui::GetTextLineHeight();
   float y = win_size.y * 0.5f + t + line_height;
   string step_info = "Anaglyph glasses calibration step #"s.append(std::to_string(m_calibrationStep + 1)).append("/6");
   CENTERED_TEXT(y + 0 * line_height, step_info.c_str());
   step_info = (m_calibrationStep < 3 ? "Left eye's "s : "Right eye's "s)
                  .append((m_calibrationStep % 3) == 0 ? "red"s
                        : (m_calibrationStep % 3) == 1 ? "green"s
                                                       : "blue"s)
                  .append(" perceived luminance: "s)
                  .append(std::to_string((int)(calibrationBrightness * 100.f)))
                  .append(1, '%');
   CENTERED_TEXT(y + 1 * line_height, step_info.c_str());
   CENTERED_TEXT(y + 3 * line_height, m_calibrationStep < 3 ? "Close your right eye" : "Close your left eye");
   CENTERED_TEXT(y + 5 * line_height, m_calibrationStep == 0 ? "Use Left Magna to exit calibration" : "Use Left Magna to move to previous step");
   CENTERED_TEXT(y + 6 * line_height, m_calibrationStep == 5 ? "Use Right Magna to exit calibration" : "Use Right Magna to move to next step");
   CENTERED_TEXT(y + 7 * line_height, "Use Left/Right Flipper to adjust face brightness until");
   CENTERED_TEXT(y + 8 * line_height, "Your eye does not favor or focus one face over the other.");
   ImGui::PopFont();

   line_height = ImGui::GetTextLineHeight();
   y = win_size.y * 0.5f - t - 3.f * line_height;
   CENTERED_TEXT(y - line_height, "Background Opacity");
   ImGui::SetCursorPos(ImVec2((win_size.x - 1.5f * t) * 0.5f, y));
   ImGui::SetNextItemWidth(1.5f * t);
   ImGui::SliderFloat("##Background Opacity", &m_backgroundOpacity, 0.f, 1.f);

   #undef CENTERED_TEXT

   ImGui::End();
   ImGui::PopStyleColor();

   m_player->m_renderer->UpdateStereoShaderState();
}

}
