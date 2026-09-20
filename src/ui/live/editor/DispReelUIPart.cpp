#include "core/stdafx.h"

#include "DispReelUIPart.h"

#include "utils/color.h"

namespace VPX::EditorUI
{

DispReelUIPart::DispReelUIPart(DispReel* dispreel)
   : EditableUIPart(dispreel, { &Data::m_visible })
{
}

DispReelUIPart::TransformMask DispReelUIPart::GetTransform(Matrix3D& transform)
{
   // DispReels are flat 2D parts in the backdrop XY plane
   transform = Matrix3D::MatrixTranslate(0.5f * (m_part->m_d.m_v1.x + m_part->m_d.m_v2.x), 0.5f * (m_part->m_d.m_v1.y + m_part->m_d.m_v2.y), 0.f);
   return static_cast<TransformMask>(TM_TransX | TM_TransY);
}

void DispReelUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D center(0.5f * (m_part->m_d.m_v1.x + m_part->m_d.m_v2.x), 0.5f * (m_part->m_d.m_v1.y + m_part->m_d.m_v2.y));
   m_part->Translate(Vertex2D(pos.x - center.x, pos.y - center.y));
}

void DispReelUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawHitObjects(m_part); // Draw the UI picking quad (reels have no wireframe to display)
}

void DispReelUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("DispReel"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<DispReel>(
         m_part, "Visible"s, //
         [this](const DispReel* dispReel) { return GetVisibility(dispReel); }, //
         [this](DispReel* dispReel, bool v) { SetVisibility(dispReel, v); });
      props.Checkbox<DispReel>(
         m_part, "Background Transparent"s, //
         [](const DispReel* dispReel) { return dispReel->m_d.m_transparent; }, //
         [](DispReel* dispReel, bool v) { dispReel->m_d.m_transparent = v; });
      props.InputRGB<DispReel>(
         m_part, "Background Color"s, //
         [](const DispReel* dispReel) { return convertColor(dispReel->m_d.m_backcolor); }, //
         [](DispReel* dispReel, const vec3& v) { dispReel->m_d.m_backcolor = convertColorRGB(v); });
      props.InputInt<DispReel>(
         m_part, "Single Digit Range (0 -> n)"s, //
         [](const DispReel* dispReel) { return dispReel->GetRange(); }, //
         [](DispReel* dispReel, int v) { dispReel->SetRange(v); });
      props.ImageCombo<DispReel>(
         m_part, "Image"s, //
         [](const DispReel* dispReel) { return dispReel->m_d.m_szImage; }, //
         [](DispReel* dispReel, const string& v) { dispReel->m_d.m_szImage = v; });

      props.Separator("Image Grid"s);
      props.Checkbox<DispReel>(
         m_part, "Use Image Grid"s, //
         [](const DispReel* dispReel) { return dispReel->m_d.m_useImageGrid; }, //
         [](DispReel* dispReel, bool v) { dispReel->m_d.m_useImageGrid = v; });
      props.InputInt<DispReel>(
         m_part, "Images Per Row"s, //
         [](const DispReel* dispReel) { return dispReel->GetImagesPerGridRow(); }, //
         [](DispReel* dispReel, int v) { dispReel->SetImagesPerGridRow(v); });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<DispReel>(
         m_part, "Position"s, //
         [](const DispReel* dispReel) { return Vertex2D(dispReel->GetX(), dispReel->GetY()); }, //
         [](DispReel* dispReel, const Vertex2D& v)
         {
            dispReel->SetX(v.x);
            dispReel->SetY(v.y);
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputInt<DispReel>(
         m_part, "Reels"s, //
         [](const DispReel* dispReel) { return dispReel->GetReels(); }, //
         [](DispReel* dispReel, int v) { dispReel->SetReels(v); });
      props.InputFloat<DispReel>(
         m_part, "Reel Width"s, //
         [](const DispReel* dispReel) { return dispReel->GetWidth(); }, //
         [](DispReel* dispReel, float v) { dispReel->SetWidth(v); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<DispReel>(
         m_part, "Reel Height"s, //
         [](const DispReel* dispReel) { return dispReel->GetHeight(); }, //
         [](DispReel* dispReel, float v) { dispReel->SetHeight(v); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<DispReel>(
         m_part, "Reel Spacing"s, //
         [](const DispReel* dispReel) { return dispReel->GetSpacing(); }, //
         [](DispReel* dispReel, float v) { dispReel->SetSpacing(v); }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (props.BeginSection("State"s))
   {
      props.InputInt<DispReel>(
         m_part, "Motor Steps"s, //
         [](const DispReel* dispReel) { return dispReel->GetMotorSteps(); }, //
         [](DispReel* dispReel, int v) { dispReel->SetMotorSteps(v); });
      props.InputInt<DispReel>(
         m_part, "Update Interval (ms)"s, //
         [](const DispReel* dispReel) { return dispReel->GetUpdateInterval(); }, //
         [](DispReel* dispReel, int v) { dispReel->SetUpdateInterval(v); });
      props.SoundCombo<DispReel>(
         m_part, "Sound"s, //
         [](const DispReel* dispReel) { return dispReel->m_d.m_szSound; }, //
         [](DispReel* dispReel, const string& v) { dispReel->m_d.m_szSound = v; });
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
