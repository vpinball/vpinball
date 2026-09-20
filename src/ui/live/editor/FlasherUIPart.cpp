#include "core/stdafx.h"
#include "FlasherUIPart.h"

#include "utils/color.h"


namespace VPX::EditorUI
{

FlasherUIPart::FlasherUIPart(Flasher* flasher)
   : EditableUIPart(flasher, { &Data::m_isVisible })
{
}

FlasherUIPart::TransformMask FlasherUIPart::GetTransform(Matrix3D& transform)
{
   const Vertex2D center = m_part->GetCenter();
   if (m_part->m_desktopBackdrop)
   {
      // Backdrop flashers are flat, in the 2D backdrop XY plane (only in plane rotation applies)
      transform = Matrix3D::MatrixRotateZ(ANGTORAD(m_part->m_d.m_rotZ)) * Matrix3D::MatrixTranslate(center.x, center.y, 0.f);
      return static_cast<TransformMask>(TM_TransAny | TM_RotZ);
   }
   const Matrix3D trans = Matrix3D::MatrixTranslate(center.x, center.y, m_part->m_d.m_height);
   const Matrix3D rotx = Matrix3D::MatrixRotateX(ANGTORAD(m_part->m_d.m_rotX));
   const Matrix3D roty = Matrix3D::MatrixRotateY(ANGTORAD(m_part->m_d.m_rotY));
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(m_part->m_d.m_rotZ));
   transform = rotz * roty * rotx * trans;
   return static_cast<TransformMask>(TM_TransAny | TM_RotAny);
}

void FlasherUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center = m_part->GetCenter();
   m_part->m_curve.TranslatePoints(Vertex2D { pos.x - center.x, pos.y - center.y });
   if (m_part->m_desktopBackdrop)
   {
      m_part->put_RotZ(rot.z);
      return;
   }
   m_part->put_Height(pos.z);
   m_part->put_RotX(rot.x);
   m_part->put_RotY(rot.y);
   m_part->put_RotZ(rot.z);
}

void FlasherUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void FlasherUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flasher"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      const FlasherData::RenderMode renderMode = props.GetEditedPart<Flasher>(m_part)->m_d.m_renderMode;
      props.Checkbox<Flasher>(
         m_part, "Visible"s, //
         [this](const Flasher* flasher) { return GetVisibility(flasher); }, //
         [this](Flasher* flasher, bool v) { SetVisibility(flasher, v); });
      props.Combo<Flasher>(
         m_part, "Render Mode"s, vector { "Flasher"s, "DMD"s, "Display"s, "Alpha.Seg."s, "Ext.Renderer."s }, //
         [](const Flasher* flasher) { return flasher->m_d.m_renderMode; }, //
         [](Flasher* flasher, int v) { flasher->m_d.m_renderMode = static_cast<FlasherData::RenderMode>(v); });
      if (renderMode != FlasherData::EXT_RENDER)
      {
         props.InputRGB<Flasher>(
            m_part, "Color"s, //
            [](const Flasher* flasher) { return convertColor(flasher->m_d.m_color); }, //
            [](Flasher* flasher, const vec3& v) { flasher->m_d.m_color = convertColorRGB(v); });
      }
      props.InputFloat<Flasher>(
         m_part, "Depth bias"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_depthBias; }, //
         [](Flasher* flasher, float v) { flasher->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);

      if (renderMode == FlasherData::FLASHER)
      {
         props.Combo<Flasher>(
            m_part, "Image Mode"s, vector { "World"s, "Wrap"s }, //
            [](const Flasher* flasher) { return static_cast<int>(flasher->m_d.m_imagealignment); }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_imagealignment = static_cast<RampImageAlignment>(v); });
         props.ImageCombo<Flasher>(
            m_part, "Image A"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.ImageCombo<Flasher>(
            m_part, "Image B"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageB; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageB = v; });
         props.Combo<Flasher>(
            m_part, "Mix"s, vector { "None"s, "Additive"s, "Overlay"s, "Multiply"s, "Screen"s }, //
            [](const Flasher* flasher) { return static_cast<int>(flasher->m_d.m_filter); }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_filter = static_cast<Filters>(v); });
         props.InputInt<Flasher>(
            m_part, "Mix Factor (%)"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_filterAmount; }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_filterAmount = v; });
      }
      else if (renderMode == FlasherData::EXT_RENDER)
      {
         props.Combo<Flasher>(
            m_part, "Renderer"s, vector { "Backglass"s, "Score View"s, "Topper"s }, //
            [](const Flasher* flasher) { return flasher->m_d.m_renderStyle - 1; }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v + 1; });
      }
      else
      {
         if (renderMode == FlasherData::DMD)
         {
            props.Combo<Flasher>(
               m_part, "Render Style"s, vector { "Legacy VPX"s, "Neon Plasma"s, "Red LED"s, "Green LED"s, "Yellow LED"s, "Generic Plasma"s, "Generic LED"s }, //
               [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
               [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         }
         else if (renderMode == FlasherData::DISPLAY)
         {
            props.Combo<Flasher>(
               m_part, "Render Style"s, vector { "Pixelated"s, "Smoothed"s, "CRT"s }, //
               [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
               [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         }
         else if (renderMode == FlasherData::ALPHASEG)
         {
            vector<string> styles;
            for (int i2 = 0; i2 < 5; i2++)
            {
               const string family = i2 == 0 ? "Generic: "s : i2 == 1 ? "Gottlieb: "s : i2 == 2 ? "Williams: "s : i2 == 3 ? "Bally: "s : "Atari: "s;
               styles.push_back(family + "Neon Plasma");
               styles.push_back(family + "Blue VFD");
               styles.push_back(family + "Green VFD");
               styles.push_back(family + "Red LED");
               styles.push_back(family + "Green LED");
               styles.push_back(family + "Yellow LED");
               styles.push_back(family + "Generic Plasma");
               styles.push_back(family + "Generic LED");
            }
            props.Combo<Flasher>(
               m_part, "Render Style"s, styles, //
               [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
               [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         }
         props.InputString<Flasher>(
            m_part, "Source"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_imageSrcLink; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_imageSrcLink = v; });
         props.ImageCombo<Flasher>(
            m_part, "Glass"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.InputFloat<Flasher>(
            m_part, "Glass Roughness"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassRoughness; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassRoughness = v; }, PropertyPane::Unit::None, 0);
         props.InputRGB<Flasher>(
            m_part, "Glass Ambient", //
            [](const Flasher* flasher) { return convertColor(flasher->m_d.m_glassAmbient); }, //
            [](Flasher* flasher, const vec3& v) { flasher->m_d.m_glassAmbient = convertColorRGB(v); });
         props.InputFloat<Flasher>(
            m_part, "Glass Pad Left"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadLeft; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadLeft = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_part, "Glass Pad Right"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadRight; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadRight = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_part, "Glass Pad Top"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadTop; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadTop = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_part, "Glass Pad Bottom"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadBottom; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadBottom = v; }, PropertyPane::Unit::Percent, 1);
      }
      props.EndSection();
   }

   if (props.BeginSection("Transparency"s))
   {
      props.InputInt<Flasher>(
         m_part, "Opacity (%)"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_alpha; }, //
         [](Flasher* flasher, int v) { flasher->m_d.m_alpha = v; });
      props.LightmapCombo<Flasher>(
         m_part, "Lightmap"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_szLightmap; }, //
         [](Flasher* flasher, const string& v) { flasher->m_d.m_szLightmap = v; });
      // Alpha Segment (which always uses max blending) and external rendering ignore both of these, see Flasher::Render
      const Flasher *const edited = props.GetEditedPart<Flasher>(m_part);
      const FlasherData::RenderMode renderMode = edited->m_d.m_renderMode;
      if (renderMode != FlasherData::ALPHASEG && renderMode != FlasherData::EXT_RENDER)
      {
         // The 2 additive modes add the very same light and only differ in what they do to what is behind the flasher:
         // 'amplify' brightens it (the historical behavior), 'absorb' darkens it, to fake a Fresnel like reflection
         vector<string> addBlendModes { "Off"s, "On, amplify"s };
         if (edited->CanAbsorbBlend())
            addBlendModes.push_back("On, absorb"s);
         // Clamped so that absorb shows as amplify where it is not honored, which is also how it renders there
         const int lastMode = static_cast<int>(addBlendModes.size()) - 1;
         props.Combo<Flasher>(
            m_part, "Additive Blend"s, addBlendModes, //
            [lastMode](const Flasher* flasher) { return clamp(flasher->m_d.m_addBlend, 0, lastMode); }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_addBlend = v; });
         props.InputFloat<Flasher>(
            m_part, "Modulate"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_modulate_vs_add; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_modulate_vs_add = v; }, PropertyPane::Unit::Percent, 1);
      }
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat3<Flasher>(
         m_part, "Position"s, //
         [](const Flasher* flasher)
         {
            const Vertex2D center = flasher->GetCenter();
            return vec3(center.x, center.y, flasher->m_d.m_height);
         }, //
         [](Flasher* flasher, const vec3& v)
         {
            const Vertex2D center = flasher->GetCenter();
            flasher->m_curve.TranslatePoints(Vertex2D { v.x - center.x, v.y - center.y });
            flasher->put_Height(v.z);
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Flasher>(
         m_part, "Rotation"s, //
         [](const Flasher* flasher) { return vec3(flasher->m_d.m_rotX, flasher->m_d.m_rotY, flasher->m_d.m_rotZ); }, //
         [](Flasher* flasher, const vec3& v)
         {
            flasher->m_d.m_rotX = v.x;
            flasher->m_d.m_rotY = v.y;
            flasher->m_d.m_rotZ = v.z;
         },
         PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Editor"s))
   {
      props.Checkbox<Flasher>(
         m_part, "Show in Editor"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_displayTexture; }, //
         [](Flasher* flasher, bool v) { flasher->m_d.m_displayTexture = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
