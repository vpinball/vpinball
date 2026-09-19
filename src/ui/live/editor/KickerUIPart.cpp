#include "core/stdafx.h"

#include "KickerUIPart.h"

namespace VPX::EditorUI
{

KickerUIPart::KickerUIPart(Kicker* kicker)
   : EditableUIPart(kicker)
{
}

KickerUIPart::~KickerUIPart() { }

KickerUIPart::TransformMask KickerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void KickerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D offset(pos.x - m_part->m_d.m_vCenter.x, pos.y - m_part->m_d.m_vCenter.y);
   m_part->Translate(offset);
}

void KickerUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   if (m_part->m_d.m_kickertype == KickerType::KickerInvisible)
      ctx.DrawHitObjects(m_part);
   else
      ctx.DrawWireframe(m_part);
}

void KickerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Kicker"s, m_part);

   if (props.BeginSection("Visuals"s))
   {
      props.MaterialCombo<Kicker>(
         m_part, "Material"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_szMaterial; }, //
         [](Kicker* kicker, const string& v) { kicker->m_d.m_szMaterial = v; });
      props.Combo<Kicker>(
         m_part, "Shape"s, vector<string> { "Invisible"s, "Hole"s, "Cup"s, "Hole Simple"s, "Williams"s, "Gottlieb"s, "Cup 2"s }, //
         [this](const Kicker* kicker) { return kicker->m_d.m_kickertype; }, //
         [this](Kicker* kicker, int v) { kicker->m_d.m_kickertype = static_cast<KickerType>(v); });
      props.InputFloat<Kicker>(
         m_part, "Radius"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_radius; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Kicker>(
         m_part, "Orientation"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_orientation; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_orientation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Kicker>(
         m_part, "Position"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_vCenter; }, //
         [](Kicker* kicker, const Vertex2D& v) { kicker->Translate(Vertex2D(v.x - kicker->m_d.m_vCenter.x, v.y - kicker->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Kicker>(
         m_part, "Surface"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_szSurface; }, //
         [](Kicker* kicker, const string& v) { kicker->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Kicker>(
         m_part, "Enabled"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_enabled; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_enabled = v; });
      props.Checkbox<Kicker>(
         m_part, "Fall Through"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_fallThrough; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_fallThrough = v; });
      props.Checkbox<Kicker>(
         m_part, "Legacy (approx. physics)"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_legacyMode; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_legacyMode = v; });
      props.InputFloat<Kicker>(
         m_part, "Scatter angle"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_scatter; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Kicker>(
         m_part, "Hit Accuracy"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_hitAccuracy; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_hitAccuracy = v; }, PropertyPane::Unit::Percent, 2);
      props.InputFloat<Kicker>(
         m_part, "Hit Height"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_hit_height; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_hit_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
