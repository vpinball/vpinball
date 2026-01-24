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
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_light->m_d.m_visible = true;
      ctx.DrawWireframe(m_light);
   }

   m_light->m_d.m_visible = isUIVisible && m_visible;
}

void LightUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Light"s, m_light);

   if (props.BeginSection("Light Settings"s))
   {
      props.Combo<Light>(
         m_light, "Shape"s, vector<string> { "None"s, "Linear"s, "Incandescent"s }, //
         [](const Light* light) { return static_cast<int>(light->m_d.m_fader); }, //
         [](Light* light, int v) { light->m_d.m_fader = static_cast<Fader>(v); });
      props.InputFloat<Light>(
         m_light, "Intensity"s, //
         [](const Light* light) { return light->m_d.m_intensity; }, //
         [](Light* light, float v) { light->m_d.m_intensity = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Light>(
         m_light, "Fade Up (ms)"s, //
         [](const Light* light) { return light->m_d.m_fadeSpeedUp > 0.1f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / light->m_d.m_fadeSpeedUp : 100000.0f; }, //
         [](Light* light, float v) { light->m_d.m_fadeSpeedUp = v > 0.001f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / v : 100000.0f; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Light>(
         m_light, "Fade Down (ms)"s, //
         [](const Light* light) { return light->m_d.m_fadeSpeedDown > 0.1f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / light->m_d.m_fadeSpeedDown : 100000.0f; }, //
         [](Light* light, float v) { light->m_d.m_fadeSpeedDown = v > 0.001f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / v : 100000.0f; }, PropertyPane::Unit::None, 1);
      props.InputRGB<Light>(
         m_light, "Light Color"s, //
         [](const Light* light) { return convertColor(light->m_d.m_color2); }, //
         [](Light* light, const vec3& v) { light->m_d.m_color = convertColorRGB(v); });
      props.InputRGB<Light>(
         m_light, "Center Burst Color"s, //
         [](const Light* light) { return convertColor(light->m_d.m_color2); }, //
         [](Light* light, const vec3& v) { light->m_d.m_color = convertColorRGB(v); });
      props.InputFloat<Light>(
         m_light, "Falloff Range"s, //
         [](const Light* light) { return light->m_d.m_falloff; }, //
         [](Light* light, float v) { light->m_d.m_falloff = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Light>(
         m_light, "Falloff Power"s, //
         [](const Light* light) { return light->m_d.m_falloff_power; }, //
         [](Light* light, float v) { light->m_d.m_falloff_power = v; }, PropertyPane::Unit::None, 1);
      props.EndSection();
   }

   if (props.BeginSection("Render Mode"s))
   {
      props.Combo<Light>(
         m_light, "Type"s, vector<string> { "Hidden"s, "Classic"s, "Halo"s }, //
         [this](const Light* light)
         {
            const bool visible = light == m_light ? m_visible : light->m_d.m_visible;
            return visible ? light->m_d.m_BulbLight ? 2 : 1 : 0;
         },
         [this](Light* light, int v)
         {
            light->m_d.m_visible = v != 0;
            (light == m_light ? m_visible : light->m_d.m_visible) = v != 0;
            light->m_d.m_BulbLight = v == 2;
         });
      if (const Light* light = props.GetEditedPart<Light>(m_light); light == m_light ? m_visible : light->m_d.m_visible)
      {
         props.Checkbox<Light>(
            m_light, "Reflection Enabled"s, //
            [](const Light* light) { return light->m_d.m_reflectionEnabled; }, //
            [](Light* light, bool v) { light->m_d.m_reflectionEnabled = v; });
         props.InputFloat<Light>(
            m_light, "Depth Bias"s, //
            [](const Light* light) { return light->m_d.m_depthBias; }, //
            [](Light* light, float v) { light->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 1);
         if (light->m_d.m_BulbLight)
         {
            props.InputFloat<Light>(
               m_light, "Halo Height"s, //
               [](const Light* light) { return light->m_d.m_bulbHaloHeight; }, //
               [](Light* light, float v) { light->m_d.m_bulbHaloHeight = v; }, PropertyPane::Unit::VPLength, 1);
            props.InputFloat<Light>(
               m_light, "Modulate"s, //
               [](const Light* light) { return light->m_d.m_modulate_vs_add; }, //
               [](Light* light, float v) { light->m_d.m_modulate_vs_add = v; }, PropertyPane::Unit::Percent, 1);
            props.InputFloat<Light>(
               m_light, "Transmit"s, //
               [](const Light* light) { return light->m_d.m_transmissionScale; }, //
               [](Light* light, float v) { light->m_d.m_transmissionScale = v; }, PropertyPane::Unit::Percent, 1);
         }
         else
         {
            props.Checkbox<Light>(
               m_light, "PassThrough"s, //
               [](const Light* light) { return light->m_d.m_imageMode; }, //
               [](Light* light, bool v) { light->m_d.m_imageMode = v; });
            props.ImageCombo<Light>(
               m_light, "Image"s, //
               [](const Light* light) { return light->m_d.m_szImage; }, //
               [](Light* light, const string& v) { light->m_d.m_szImage = v; });
         }
      }
      props.EndSection();
   }

   if (props.BeginSection("Bulb"s))
   {
      props.Checkbox<Light>(
         m_light, "Show Bulb"s, //
         [](const Light* light) { return light->m_d.m_showBulbMesh; }, //
         [](Light* light, bool v) { light->m_d.m_showBulbMesh = v; });
      props.Checkbox<Light>(
         m_light, "Static Mesh"s, //
         [](const Light* light) { return light->m_d.m_staticBulbMesh; }, //
         [](Light* light, bool v) { light->m_d.m_staticBulbMesh = v; });
      props.InputFloat<Light>(
         m_light, "Radius"s, //
         [](const Light* light) { return light->m_d.m_meshRadius; }, //
         [](Light* light, float v) { light->m_d.m_meshRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Ball Reflections & Shadows"s))
   {
      props.Checkbox<Light>(
         m_light, "Show Reflection on Balls"s, //
         [](const Light* light) { return light->m_d.m_showReflectionOnBall; }, //
         [](Light* light, bool v) { light->m_d.m_showReflectionOnBall = v; });
      props.Checkbox<Light>(
         m_light, "Raytraced Ball Shadows"s, //
         [](const Light* light) { return light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS; }, //
         [](Light* light, bool v) { light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Light>(
         m_light, "Position"s, //
         [](const Light* light) { return light->m_d.m_vCenter; }, //
         [](Light* light, const Vertex2D& v) { light->Translate(Vertex2D(v.x - light->m_d.m_vCenter.x, v.y - light->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Light>(
         m_light, "Surface"s, //
         [](const Light* light) { return light->m_d.m_szSurface; }, //
         [](Light* light, const string& v) { light->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("States"s))
   {
      props.Checkbox<Light>(
         m_light, "Blinking Light"s, //
         [](const Light* light) { return light->m_d.m_state == 2; }, //
         [](Light* light, bool v)
         {
            if (v)
               light->m_d.m_state = 2.f;
            else if (light->m_d.m_state == 2.f)
               light->m_d.m_state = 1.f;
         });
      if (const Light* light = props.GetEditedPart<Light>(m_light); light->m_d.m_state == 2.f)
      {
         props.InputString<Light>(
            m_light, "Blink Pattern"s, //
            [](const Light* light) { return light->m_d.m_rgblinkpattern; }, //
            [](Light* light, const string& v) { light->m_d.m_rgblinkpattern = v; });
         props.InputInt<Light>(
            m_light, "Blink Interval (ms)"s, //
            [](const Light* light) { return light->m_d.m_blinkinterval; }, //
            [](Light* light, int v) { light->m_d.m_blinkinterval = v; });
      }
      else
      {
         props.InputFloat<Light>(
            m_light, "State"s, //
            [](const Light* light) { return light->m_d.m_state; }, //
            [](Light* light, float v)
            {
               light->m_d.m_state = v;
               light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
            },
            PropertyPane::Unit::Percent, 1);
      }
      props.EndSection();
   }

   if (m_light->GetPTable()->m_liveBaseTable && props.BeginSection("Transient state"s))
   {
      props.InputFloat<Light>(
         m_light, "Intensity Scale"s, //
         [](const Light* light) { return light->m_d.m_intensity_scale; }, //
         [](Light* light, float v) { light->m_d.m_intensity_scale = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Light>(
         m_light, "Current Intensity"s, //
         [](const Light* light) { return light->m_currentIntensity; }, //
         [](Light* light, float v) { light->m_currentIntensity = v; }, PropertyPane::Unit::None, 1);
      props.InputFloat<Light>(
         m_light, "Filament Temperature"s, //
         [](const Light* light) { return static_cast<float>(light->m_currentFilamentTemperature); }, //
         [](Light* light, float v) { light->m_currentFilamentTemperature = static_cast<double>(v); }, PropertyPane::Unit::None, 1);
      props.EndSection();
   }

   props.TimerSection<Light>(m_light, [](Light* obj) { return &(obj->m_d.m_tdr); });
}

}
