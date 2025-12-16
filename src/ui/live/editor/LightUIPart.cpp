#pragma once

#include "core/stdafx.h"

#include "LightUIPart.h"

namespace VPX::EditorUI
{

LightUIPart::LightUIPart(Light* light)
   : m_light(light)
   , m_visible(light->m_d.m_visible)
{
}

LightUIPart::~LightUIPart() { m_light->m_d.m_visible = m_visible; }

LightUIPart::TransformMask LightUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_light->GetPTable()->GetSurfaceHeight(m_light->m_d.m_szSurface, m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y, height + m_light->m_d.m_height);
   return TM_TransAny;
}

void LightUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const float pz = pos.z - m_light->GetPTable()->GetSurfaceHeight(m_light->m_d.m_szSurface, m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y);
   m_light->m_d.m_bulbHaloHeight = m_light->m_d.m_bulbHaloHeight + (pz - m_light->m_d.m_height);
   m_light->m_d.m_vCenter.x = pos.x;
   m_light->m_d.m_vCenter.y = pos.y;
   m_light->m_d.m_height = pz;
}

void LightUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_light->m_d.m_visible;

   const bool isUIVisible = m_light->IsVisible(m_light);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.GetViewMode() != ViewMode::PreviewCam)))
   {
      m_light->m_d.m_visible = true;
      // FIXME ctx.DrawWireframe(m_light);
      //ctx.DrawHitObjects(m_light);
   }

   m_light->m_d.m_visible = isUIVisible && m_visible;
}

void LightUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Light", m_light);
   
   /*
   Light *const light = (is_live ? live_light : startup_light);
   if (light && ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_intensity = [startup_light, live_light, light](bool is_live, float prev, float v)
      {
         if (prev > 0.1f && v > 0.1f)
         {
            const float fade_up_ms = prev / light->m_d.m_fadeSpeedUp;
            light->m_d.m_fadeSpeedUp = fade_up_ms < 0.1f ? 100000.0f : v / fade_up_ms;
            const float fade_down_ms = prev / light->m_d.m_fadeSpeedDown;
            light->m_d.m_fadeSpeedDown = fade_down_ms < 0.1f ? 100000.0f : v / fade_down_ms;
         }
         startup_light->m_currentIntensity = startup_light->m_d.m_intensity * startup_light->m_d.m_intensity_scale * startup_light->m_inPlayState;
         live_light->m_currentIntensity = live_light->m_d.m_intensity * live_light->m_d.m_intensity_scale * live_light->m_inPlayState;
      };
      float startup_fadeup = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedUp) : 0.f;
      float live_fadeup = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedUp) : 0.f;
      auto upd_fade_up = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedUp = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v;  };
      float startup_fadedown = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedDown) : 0.f;
      float live_fadedown = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedDown) : 0.f;
      auto upd_fade_down = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedDown = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v; };
      bool startup_shadow = startup_light ? (startup_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      bool live_shadow = live_light ? (live_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      auto upd_shadow = [light](bool is_live, bool prev, bool v) { light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE; };

      PropSeparator("Light Settings");
      PropFloat("Intensity", startup_light, is_live, startup_light ? &(startup_light->m_d.m_intensity) : nullptr, live_light ? &(live_light->m_d.m_intensity) : nullptr, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_intensity);
      static const string faders[] = { "None"s, "Linear"s, "Incandescent"s };
      PropCombo("Fader", startup_light, is_live, startup_light ? (int *)&(startup_light->m_d.m_fader) : nullptr, live_light ? (int *)&(live_light->m_d.m_fader) : nullptr, std::size(faders), faders);
      PropFloat("Fade Up (ms)", startup_light, is_live, startup_light ? &startup_fadeup : nullptr, live_light ? &live_fadeup : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_up);
      PropFloat("Fade Down (ms)", startup_light, is_live, startup_light ? &startup_fadedown : nullptr, live_light ? &live_fadedown : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_down);
      PropRGB("Light Color", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color) : nullptr, live_light ? &(live_light->m_d.m_color) : nullptr);
      PropRGB("Center Burst", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color2) : nullptr, live_light ? &(live_light->m_d.m_color2) : nullptr);
      PropFloat("Falloff Range", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff) : nullptr, live_light ? &(live_light->m_d.m_falloff) : nullptr, 10.f, 100.f, "%.0f");
      PropFloat("Falloff Power", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff_power) : nullptr, live_light ? &(live_light->m_d.m_falloff_power) : nullptr, 0.1f, 0.5f, "%.2f");

      PropSeparator("Render Mode");
      static const string modes[] = { "Hidden"s, "Classic"s, "Halo"s };
      int startup_mode = startup_light ? startup_light->m_d.m_visible ? startup_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      int live_mode = live_light ? live_light->m_d.m_visible ? live_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      auto upd_mode = [light](bool is_live, bool prev, int v) { light->m_d.m_visible = (v != 0); light->m_d.m_BulbLight = (v != 1); };
      PropCombo("Type", startup_light, is_live, startup_mode >= 0 ? &startup_mode : nullptr, live_mode >= 0 ? &live_mode : nullptr, std::size(modes), modes, upd_mode);
      if (!light->m_d.m_visible)
      {
      }
      else if (light->m_d.m_BulbLight)
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropFloat("Halo Height", startup_light, is_live, startup_light ? &(startup_light->m_d.m_bulbHaloHeight) : nullptr, live_light ? &(live_light->m_d.m_bulbHaloHeight) : nullptr, 1.f, 5.f, "%.1f");
         PropFloat("Modulate", startup_light, is_live, startup_light ? &(startup_light->m_d.m_modulate_vs_add) : nullptr, live_light ? &(live_light->m_d.m_modulate_vs_add) : nullptr, 0.1f, 0.5f, "%.1f");
         PropFloat("Transmission", startup_light, is_live, startup_light ? &(startup_light->m_d.m_transmissionScale) : nullptr, live_light ? &(live_light->m_d.m_transmissionScale) : nullptr, 0.1f, 0.5f, "%.1f");
      }
      else
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropCheckbox("PassThrough", startup_light, is_live, startup_light ? &(startup_light->m_d.m_imageMode) : nullptr, live_light ? &(live_light->m_d.m_imageMode) : nullptr);
         PropImageCombo("Image", startup_light, is_live, startup_light ? &(startup_light->m_d.m_szImage) : nullptr, live_light ? &(live_light->m_d.m_szImage) : nullptr, m_table);
      }

      PropSeparator("Bulb");
      PropCheckbox("Render bulb", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_showBulbMesh) : nullptr);
      PropCheckbox("Static rendering", startup_light, is_live, startup_light ? &(startup_light->m_d.m_staticBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_staticBulbMesh) : nullptr);
      PropFloat("Bulb Size", startup_light, is_live, startup_light ? &(startup_light->m_d.m_meshRadius) : nullptr, live_light ? &(live_light->m_d.m_meshRadius) : nullptr, 1.0f, 5.0f, "%.0f");

      PropSeparator("Ball reflections & Shadows");
      PropCheckbox("Show Reflection on Balls", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showReflectionOnBall) : nullptr, live_light ? &(live_light->m_d.m_showReflectionOnBall) : nullptr);
      PropCheckbox("Raytraced ball shadows", startup_light, is_live, startup_light ? &startup_shadow : nullptr, live_light ? &live_shadow : nullptr, upd_shadow);

      PropSeparator("Position");
      // FIXME This allows to edit the center but does not update dragpoint coordinates accordingly => add a callback and use Translate
      // FIXME we also need to save dragpoint change when saving x/y to startup table as well as center pos => add a save callback and copy to startup table
      PropFloat("X", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.x) : nullptr, live_light ? &(live_light->m_d.m_vCenter.x) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Y", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.y) : nullptr, live_light ? &(live_light->m_d.m_vCenter.y) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Z", startup_light, is_live, startup_light ? &(startup_light->m_d.m_height) : nullptr, live_light ? &(live_light->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");

      ImGui::EndTable();
   }
   if (light && ImGui::CollapsingHeader("States", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_inplaystate = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light * const light = (is_live ? live_light : startup_light);
         light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
      };
      PropFloat("State", startup_light, is_live, startup_light ? &(startup_light->m_d.m_state) : nullptr, live_light ? &(live_light->m_d.m_state) : nullptr, 0.1f, 0.5f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_inplaystate);
      // Missing blink pattern
      PropInt("Blink interval", startup_light, is_live, startup_light ? &(startup_light->m_d.m_blinkinterval) : nullptr, live_light ? &(live_light->m_d.m_blinkinterval) : nullptr);
      ImGui::EndTable();
   }
   PROP_TIMER(is_live, startup_light, live_light)
   if (is_live && ImGui::CollapsingHeader("Live state", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PROP_TABLE_SETUP
      ImGui::BeginDisabled();
      ImGui::TableNextColumn();
      ImGui::InputFloat("Intensity", &live_light->m_currentIntensity);
      ImGui::TableNextColumn();
      ImGui::Button(ICON_SAVE "##t2");
      if (live_light->m_d.m_fader == FADER_INCANDESCENT)
      {
         ImGui::TableNextColumn();
         float temperature = (float)live_light->m_currentFilamentTemperature;
         ImGui::InputFloat("Filament Temperature", &temperature);
         ImGui::TableNextColumn();
         ImGui::Button(ICON_SAVE "##t1");
      }
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
   
   */
   if (props.BeginSection(PropertyPane::Section::Visual))
   {
      props.EndSection();
   }
   
   props.TimerSection<Light>(m_light, [](Light* obj) { return &(obj->m_d.m_tdr); });
}

}