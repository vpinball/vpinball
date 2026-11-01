#include "core/stdafx.h"
#include "LightUIPart.h"

#include "math/matrix.h"
#include "utils/color.h"


namespace VPX::EditorUI
{

LightUIPart::LightUIPart(Light* light)
   : EditableUIPart(light, { &Data::m_visible })
{
}

LightUIPart::TransformMask LightUIPart::GetTransform(Matrix3D& transform)
{
   // Backdrop lights are flat, in the 2D backdrop XY plane
   if (m_part->m_desktopBackdrop)
      return GetCurveTransform(transform, 0.f);
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   return GetCurveTransform(transform, height + m_part->m_d.m_height);
}

void LightUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   if (!m_part->m_desktopBackdrop)
   {
      const float pz = pos.z - m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, pos.x, pos.y);
      m_part->m_d.m_bulbHaloHeight = m_part->m_d.m_bulbHaloHeight + (pz - m_part->m_d.m_height);
      m_part->m_d.m_height = pz;
   }
   SetCurveTransform(pos, scale, rot);
}

void LightUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);

   // Bulb center overlay, same as the win32 editor: falloff ellipse, center crosshair, bulb mesh circle
   const ImU32 color = ctx.GetColor(ctx.IsSelected());
   const Vertex3Ds xAxis(1.f, 0.f, 0.f);
   const Vertex3Ds yAxis(0.f, 1.f, 0.f);
   const Vertex2D& c = m_part->m_d.m_vCenter;
   const float z = m_part->m_surfaceHeight;
   ctx.DrawCircle(Vertex3Ds(c.x, c.y, z), xAxis, yAxis, m_part->m_d.m_falloff, color);
   ctx.DrawLine(Vertex3Ds(c.x - 10.f, c.y, z), Vertex3Ds(c.x + 10.f, c.y, z), color);
   ctx.DrawLine(Vertex3Ds(c.x, c.y - 10.f, z), Vertex3Ds(c.x, c.y + 10.f, z), color);
   if (m_part->m_d.m_showBulbMesh)
      ctx.DrawCircle(Vertex3Ds(c.x, c.y, z), xAxis, yAxis, m_part->m_d.m_meshRadius * 0.5f, color);
}

void LightUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Light"s, m_part);

   UpdateCurveSection(props);
   UpdateCenterSection(props, &Data::m_vCenter);

   if (props.BeginSection("Light Settings"s))
   {
      props.InputFloat<Light>(
         m_part, "Intensity"s, //
         [](const Light* light) { return light->m_d.m_intensity; }, //
         [](Light* light, float v)
         {
            light->m_d.m_intensity = v;
            light->m_currentIntensity = light->m_d.m_intensity * light->m_d.m_intensity_scale * light->m_d.m_state;
         },
         PropertyPane::Unit::None, 1);
      props.Combo<Light>(
         m_part, "Fader"s, vector<string> { "None"s, "Linear"s, "Incandescent"s }, //
         [](const Light* light) { return static_cast<int>(light->m_d.m_fader); }, //
         [](Light* light, int v) { light->m_d.m_fader = static_cast<Fader>(v); }, "How the light interpolates between states: instantly, linearly, or emulating an incandescent filament");
      props.InputFloat<Light>(
         m_part, "Fade Up (ms)"s, //
         [](const Light* light) { return light->m_d.m_fadeSpeedUp > 0.1f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / light->m_d.m_fadeSpeedUp : 100000.0f; }, //
         [](Light* light, float v) { light->m_d.m_fadeSpeedUp = v > 0.001f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / v : 100000.0f; }, PropertyPane::Unit::None, 1,
         "Time for the light to ramp from off to its full intensity");
      props.InputFloat<Light>(
         m_part, "Fade Down (ms)"s, //
         [](const Light* light) { return light->m_d.m_fadeSpeedDown > 0.1f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / light->m_d.m_fadeSpeedDown : 100000.0f; }, //
         [](Light* light, float v) { light->m_d.m_fadeSpeedDown = v > 0.001f ? light->m_d.m_intensity * light->m_d.m_intensity_scale / v : 100000.0f; }, PropertyPane::Unit::None, 1,
         "Time for the light to fade from its full intensity to off");
      props.InputRGB<Light>(
         m_part, "Center Color"s, //
         [](const Light* light) { return convertColor(light->m_d.m_color2); }, //
         [](Light* light, const vec3& v) { light->m_d.m_color2 = convertColorRGB(v); }, "Color at the center of the light shape");
      props.InputRGB<Light>(
         m_part, "Edge Color"s, //
         [](const Light* light) { return convertColor(light->m_d.m_color); }, //
         [](Light* light, const vec3& v) { light->m_d.m_color = convertColorRGB(v); }, "Color at the edge of the light's range");
      props.InputFloat<Light>(
         m_part, "Light Range"s, //
         [](const Light* light) { return light->m_d.m_falloff; }, //
         [](Light* light, float v) { light->m_d.m_falloff = v; }, PropertyPane::Unit::VPLength, 1, "Radius reached by the light on the playfield");
      props.InputFloat<Light>(
         m_part, "Falloff Power"s, //
         [](const Light* light) { return light->m_d.m_falloff_power; }, //
         [](Light* light, float v) { light->m_d.m_falloff_power = v; }, PropertyPane::Unit::None, 1,
         "Exponent of the intensity falloff toward the edge of the light's range (higher = tighter center)");
      props.EndSection();
   }

   if (props.BeginSection("Render Mode"s))
   {
      props.Combo<Light>(
         m_part, "Type"s, vector<string> { "Hidden"s, "Classic"s, "Halo"s }, //
         [this](const Light* light) { return GetVisibility(light) ? light->m_d.m_BulbLight ? 2 : 1 : 0; },
         [this](Light* light, int v)
         {
            SetVisibility(light, v != 0);
            light->m_d.m_BulbLight = v == 2;
         },
         "Hidden lights the scene without being drawn; Classic draws the flat light shape; Halo renders a 3D bulb");
      if (const Light* light = props.GetEditedPart<Light>(m_part); GetVisibility(light))
      {
         props.Checkbox<Light>(
            m_part, "Reflection Enabled"s, //
            [](const Light* light) { return light->m_d.m_reflectionEnabled; }, //
            [](Light* light, bool v) { light->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
         props.InputFloat<Light>(
            m_part, "Depth Bias"s, //
            [](const Light* light) { return light->m_d.m_depthBias; }, //
            [](Light* light, float v) { light->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 1, "Depth offset applied at render time to fix flickering between overlapping parts");
         if (light->m_d.m_BulbLight)
         {
            props.InputFloat<Light>(
               m_part, "Halo Height"s, //
               [](const Light* light) { return light->m_d.m_bulbHaloHeight; }, //
               [](Light* light, float v) { light->m_d.m_bulbHaloHeight = v; }, PropertyPane::Unit::VPLength, 1, "Height above the surface at which the bulb halo is drawn");
            props.InputFloat<Light>(
               m_part, "Modulate"s, //
               [](const Light* light) { return light->m_d.m_modulate_vs_add; }, //
               [](Light* light, float v) { light->m_d.m_modulate_vs_add = v; }, PropertyPane::Unit::Percent, 1,
               "Blend between adding light over the surface (0%) and modulating its color (100%)");
            props.InputFloat<Light>(
               m_part, "Transmit"s, //
               [](const Light* light) { return light->m_d.m_transmissionScale; }, //
               [](Light* light, float v) { light->m_d.m_transmissionScale = v; }, PropertyPane::Unit::Percent, 1, "Amount of light transmitted to the back side of the surface below");
         }
         else
         {
            props.Checkbox<Light>(
               m_part, "PassThrough"s, //
               [](const Light* light) { return light->m_d.m_imageMode; }, //
               [](Light* light, bool v) { light->m_d.m_imageMode = v; }, "Draw the image unlit instead of rendering it as a lit light");
            props.ImageCombo<Light>(
               m_part, "Image"s, //
               [](const Light* light) { return light->m_d.m_szImage; }, //
               [](Light* light, const string& v) { light->m_d.m_szImage = v; });
         }
      }
      props.EndSection();
   }

   if (props.BeginSection("Bulb"s))
   {
      props.Checkbox<Light>(
         m_part, "Show Bulb"s, //
         [](const Light* light) { return light->m_d.m_showBulbMesh; }, //
         [](Light* light, bool v) { light->m_d.m_showBulbMesh = v; });
      props.Checkbox<Light>(
         m_part, "Static Mesh"s, //
         [](const Light* light) { return light->m_d.m_staticBulbMesh; }, //
         [](Light* light, bool v) { light->m_d.m_staticBulbMesh = v; }, "Bake the bulb mesh into the static render pass (faster, but it cannot change at runtime)");
      props.InputFloat<Light>(
         m_part, "Radius"s, //
         [](const Light* light) { return light->m_d.m_meshRadius; }, //
         [](Light* light, float v) { light->m_d.m_meshRadius = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("Ball Reflections & Shadows"s))
   {
      props.Checkbox<Light>(
         m_part, "Show Reflection on Balls"s, //
         [](const Light* light) { return light->m_d.m_showReflectionOnBall; }, //
         [](Light* light, bool v) { light->m_d.m_showReflectionOnBall = v; });
      props.Checkbox<Light>(
         m_part, "Raytraced Ball Shadows"s, //
         [](const Light* light) { return light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS; }, //
         [](Light* light, bool v) { light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE; }, "Cast a real-time shadow of the ball from this light");
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Light>(
         m_part, "Position"s, //
         [](const Light* light) { return light->m_d.m_vCenter; }, //
         [](Light* light, const Vertex2D& v) { light->Translate(Vertex2D(v.x - light->m_d.m_vCenter.x, v.y - light->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Light>(
         m_part, "Height"s, //
         [](const Light* light) { return light->m_d.m_height; }, //
         [](Light* light, float v) { light->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Light>(
         m_part, "Surface"s, //
         [](const Light* light) { return light->m_d.m_szSurface; }, //
         [](Light* light, const string& v) { light->m_d.m_szSurface = v; }, "Surface this part rests on (defines its base height)");
      props.EndSection();
   }

   if (props.BeginSection("States"s))
   {
      props.Checkbox<Light>(
         m_part, "Blinking Light"s, //
         [](const Light* light) { return light->m_d.m_state == 2; }, //
         [](Light* light, bool v)
         {
            if (v)
               light->m_d.m_state = 2.f;
            else if (light->m_d.m_state == 2.f)
               light->m_d.m_state = 1.f;
         });
      if (const Light* light = props.GetEditedPart<Light>(m_part); light->m_d.m_state == 2.f)
      {
         props.InputString<Light>(
            m_part, "Blink Pattern"s, //
            [](const Light* light) { return light->m_d.m_rgblinkpattern; }, //
            [](Light* light, const string& v) { light->m_d.m_rgblinkpattern = v; }, "On/off steps played in a loop, one character per blink interval ('1' = on, e.g. '10' toggles)");
         props.InputInt<Light>(
            m_part, "Blink Interval (ms)"s, //
            [](const Light* light) { return light->m_d.m_blinkinterval; }, //
            [](Light* light, int v) { light->m_d.m_blinkinterval = v; });
      }
      else
      {
         props.InputFloat<Light>(
            m_part, "State"s, //
            [](const Light* light) { return light->m_d.m_state; }, //
            [](Light* light, float v)
            {
               light->m_d.m_state = v;
               light->m_currentIntensity = light->m_d.m_intensity * light->m_d.m_intensity_scale * light->m_d.m_state;
               light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
            },
            PropertyPane::Unit::Percent, 1, "Current light state: 0 = off, 1 = on, fractional values dim the light");
      }
      props.InputResURI<Light>(
         m_part, "State Link"s, //
         [](const Light* light) { return light->m_d.m_stateSrcLink; }, //
         [](Light* light, const string& v) { light->m_d.m_stateSrcLink = v; }, ResURIKind::State, "URI linking this light's state to a shared state source (plugin or external controller)");
      props.EndSection();
   }

   if (m_part->GetPTable()->m_liveBaseTable && props.BeginSection("Transient state"s))
   {
      props.InputFloat<Light>(
         m_part, "Intensity Scale"s, //
         [](const Light* light) { return light->m_d.m_intensity_scale; }, //
         [](Light* light, float v) { light->m_d.m_intensity_scale = v; }, PropertyPane::Unit::None, 1, "Runtime multiplier applied to the intensity (transient state, not saved)");
      props.InputFloat<Light>(
         m_part, "Current Intensity"s, //
         [](const Light* light) { return light->m_currentIntensity; }, //
         [](Light* light, float v) { light->m_currentIntensity = v; }, PropertyPane::Unit::None, 1, "Intensity currently emitted by the light (transient state, not saved)");
      props.InputFloat<Light>(
         m_part, "Filament Temperature"s, //
         [](const Light* light) { return static_cast<float>(light->m_currentFilamentTemperature); }, //
         [](Light* light, float v) { light->m_currentFilamentTemperature = static_cast<double>(v); }, PropertyPane::Unit::None, 1,
         "Simulated filament temperature driving the incandescent fader (transient state, not saved)");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
