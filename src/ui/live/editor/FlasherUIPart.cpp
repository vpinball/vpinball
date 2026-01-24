#include "core/stdafx.h"

#include "FlasherUIPart.h"

namespace VPX::EditorUI
{

FlasherUIPart::FlasherUIPart(Flasher* flasher)
   : m_flasher(flasher)
   , m_visible(flasher->m_d.m_isVisible)
{
}

FlasherUIPart::~FlasherUIPart() { m_flasher->m_d.m_isVisible = m_visible; }

FlasherUIPart::TransformMask FlasherUIPart::GetTransform(Matrix3D& transform)
{
   const Matrix3D trans = Matrix3D::MatrixTranslate(m_flasher->m_d.m_vCenter.x, m_flasher->m_d.m_vCenter.y, m_flasher->m_d.m_height);
   const Matrix3D rotx = Matrix3D::MatrixRotateX(ANGTORAD(m_flasher->m_d.m_rotX));
   const Matrix3D roty = Matrix3D::MatrixRotateY(ANGTORAD(m_flasher->m_d.m_rotY));
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(m_flasher->m_d.m_rotZ));
   transform = rotz * roty * rotx * trans;
   return static_cast<TransformMask>(TM_TransAny | TM_RotAny);
}

void FlasherUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const float px = m_flasher->m_d.m_vCenter.x, py = m_flasher->m_d.m_vCenter.y;
   m_flasher->TranslatePoints(Vertex2D { pos.x - px, pos.y - py });
   m_flasher->GetPTable()->m_undo.Undo(true);
   m_flasher->put_Height(pos.z);
   m_flasher->put_RotX(rot.x);
   m_flasher->put_RotY(rot.y);
   m_flasher->put_RotZ(rot.z);
}

void FlasherUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_visible = m_flasher->m_d.m_isVisible;

   const bool isUIVisible = m_flasher->IsVisible(m_flasher);
   if (isUIVisible && (ctx.IsSelected() || (!m_visible && ctx.IsShowInvisible())))
   {
      m_flasher->m_d.m_isVisible = true;
      ctx.DrawWireframe(m_flasher);
   }

   m_flasher->m_d.m_isVisible = isUIVisible && m_visible;
}

void FlasherUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Flasher"s, m_flasher);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Flasher>(
         m_flasher, "Visible"s, //
         [this](const Flasher* flasher) { return flasher == m_flasher ? m_visible : flasher->m_d.m_isVisible; }, //
         [this](Flasher* flasher, bool v)
         {
            flasher->m_d.m_isVisible = v;
            (flasher == m_flasher ? m_visible : flasher->m_d.m_isVisible) = v;
         });
      props.Combo<Flasher>(
         m_flasher, "Render Mode"s, vector { "Flasher"s, "DMD"s, "Display"s, "Alpha.Seg."s }, //
         [](const Flasher* flasher) { return flasher->m_d.m_renderMode; }, //
         [](Flasher* flasher, int v) { flasher->m_d.m_renderMode = static_cast<FlasherData::RenderMode>(v); });
      props.InputRGB<Flasher>(
         m_flasher, "Color"s, //
         [](const Flasher* flasher) { return convertColor(flasher->m_d.m_color); }, //
         [](Flasher* flasher, const vec3& v) { flasher->m_d.m_color = convertColorRGB(v); });
      props.InputFloat<Flasher>(
         m_flasher, "Depth bias"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_depthBias; }, //
         [](Flasher* flasher, float v) { flasher->m_d.m_depthBias = v; }, PropertyPane::Unit::None, 0);

      if (const FlasherData::RenderMode renderMode = props.GetEditedPart<Flasher>(m_flasher)->m_d.m_renderMode; renderMode == FlasherData::FLASHER)
      {
         props.Combo<Flasher>(
            m_flasher, "Image Mode"s, vector { "World"s, "Wrap"s }, //
            [](const Flasher* flasher) { return static_cast<int>(flasher->m_d.m_imagealignment); }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_imagealignment = static_cast<RampImageAlignment>(v); });
         props.ImageCombo<Flasher>(
            m_flasher, "Image A"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.ImageCombo<Flasher>(
            m_flasher, "Image B"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageB; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageB = v; });
         props.Combo<Flasher>(
            m_flasher, "Mix"s, vector { "None"s, "Additive"s, "Overlay"s, "Multiply"s, "Screen"s }, //
            [](const Flasher* flasher) { return static_cast<int>(flasher->m_d.m_filter); }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_filter = static_cast<Filters>(v); });
         props.InputInt<Flasher>(
            m_flasher, "Mix Factor (%)"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_filterAmount; }, //
            [](Flasher* flasher, int v) { flasher->m_d.m_filterAmount = v; });
      }
      else
      {
         if (renderMode == FlasherData::DMD)
         {
            props.Combo<Flasher>(
               m_flasher, "Render Style"s, vector { "Legacy VPX"s, "Neon Plasma"s, "Red LED"s, "Green LED"s, "Yellow LED"s, "Generic Plasma"s, "Generic LED"s }, //
               [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
               [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         }
         else if (renderMode == FlasherData::DISPLAY)
         {
            props.Combo<Flasher>(
               m_flasher, "Render Style"s, vector { "Pixelated"s, "Smoothed"s, "CRT"s }, //
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
               m_flasher, "Render Style"s, styles, //
               [](const Flasher* flasher) { return flasher->m_d.m_renderStyle; }, //
               [](Flasher* flasher, int v) { flasher->m_d.m_renderStyle = v; });
         }
         props.InputString<Flasher>(
            m_flasher, "Source"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_imageSrcLink; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_imageSrcLink = v; });
         props.ImageCombo<Flasher>(
            m_flasher, "Glass"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_szImageA; }, //
            [](Flasher* flasher, const string& v) { flasher->m_d.m_szImageA = v; });
         props.InputFloat<Flasher>(
            m_flasher, "Glass Roughness"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassRoughness; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassRoughness = v; }, PropertyPane::Unit::None, 0);
         props.InputRGB<Flasher>(
            m_flasher, "Glass Ambient", //
            [](const Flasher* flasher) { return convertColor(flasher->m_d.m_glassAmbient); }, //
            [](Flasher* flasher, const vec3& v) { flasher->m_d.m_glassAmbient = convertColorRGB(v); });
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Left"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadLeft; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadLeft = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Right"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadRight; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadRight = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Top"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadTop; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadTop = v; }, PropertyPane::Unit::Percent, 1);
         props.InputFloat<Flasher>(
            m_flasher, "Glass Pad Bottom"s, //
            [](const Flasher* flasher) { return flasher->m_d.m_glassPadBottom; }, //
            [](Flasher* flasher, float v) { flasher->m_d.m_glassPadBottom = v; }, PropertyPane::Unit::Percent, 1);
      }
      props.EndSection();
   }

   if (props.BeginSection("Transparency"s))
   {
      props.InputInt<Flasher>(
         m_flasher, "Opacity (%)"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_alpha; }, //
         [](Flasher* flasher, int v) { flasher->m_d.m_alpha = v; });
      props.LightmapCombo<Flasher>(
         m_flasher, "Lightmap"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_szLightmap; }, //
         [](Flasher* flasher, const string& v) { flasher->m_d.m_szLightmap = v; });
      props.Checkbox<Flasher>(
         m_flasher, "Additive Blend"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_addBlend; }, //
         [](Flasher* flasher, bool v) { flasher->m_d.m_addBlend = v; });
      props.InputFloat<Flasher>(
         m_flasher, "Modulate"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_modulate_vs_add; }, //
         [](Flasher* flasher, float v) { flasher->m_d.m_modulate_vs_add = v; }, PropertyPane::Unit::Percent, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat3<Flasher>(
         m_flasher, "Position"s, //
         [](const Flasher* flasher) { return vec3(flasher->m_d.m_vCenter.x, flasher->m_d.m_vCenter.y, flasher->m_d.m_height); }, //
         [](Flasher* flasher, const vec3& v)
         {
            const float px = flasher->m_d.m_vCenter.x;
            const float py = flasher->m_d.m_vCenter.y;
            flasher->TranslatePoints(Vertex2D { v.x - px, v.y - py });
            flasher->put_Height(v.z);
            flasher->GetPTable()->m_undo.Undo(true);
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Flasher>(
         m_flasher, "Rotation"s, //
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
         m_flasher, "Disable Wireframe"s, //
         [](const Flasher* flasher) { return flasher->m_d.m_displayTexture; }, //
         [](Flasher* flasher, bool v) { flasher->m_d.m_displayTexture = v; });
      props.EndSection();
   }

   props.TimerSection<Flasher>(m_flasher, [](Flasher* obj) { return &(obj->m_d.m_tdr); });
}

}
