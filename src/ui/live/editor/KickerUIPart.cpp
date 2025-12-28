#include "core/stdafx.h"

#include "KickerUIPart.h"

namespace VPX::EditorUI
{

KickerUIPart::KickerUIPart(Kicker* kicker)
   : m_kicker(kicker)
   , m_kickerType(kicker->m_d.m_kickertype)
{
}

KickerUIPart::~KickerUIPart() { m_kicker->m_d.m_kickertype = m_kickerType; }

KickerUIPart::TransformMask KickerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_kicker->GetPTable()->GetSurfaceHeight(m_kicker->m_d.m_szSurface, m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y);
   transform = Matrix3D::MatrixTranslate(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, height);
   return TM_TransAny;
}

void KickerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   const Vertex2D offset(pos.x - m_kicker->m_d.m_vCenter.x, pos.y - m_kicker->m_d.m_vCenter.y);
   m_kicker->Translate(offset);
}

void KickerUIPart::Render(const EditorRenderContext& ctx)
{
   if (ctx.NeedsLiveTableSync())
      m_kickerType = m_kicker->m_d.m_kickertype;

   const bool isUIVisible = m_kicker->IsVisible(m_kicker);
   if (isUIVisible && (ctx.IsSelected() || m_kickerType == KickerType::KickerInvisible))
   {
      m_kicker->m_d.m_visible = true;
      //ctx.DrawWireframe(m_kicker);
      ctx.DrawHitObjects(m_kicker);
   }

   m_kicker->m_d.m_visible = isUIVisible && (m_kickerType != KickerType::KickerInvisible);
}

void KickerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Kicker", m_kicker);
   
   if (props.BeginSection("Visual"s))
   {
      props.MaterialCombo<Kicker>(
         m_kicker, "Material"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_szMaterial; }, //
         [](Kicker* kicker, const string& v) { kicker->m_d.m_szMaterial = v; });
      props.Combo<Kicker>(
         m_kicker, "Shape", vector<string> { "Invisible"s, "Hole"s, "Cup"s, "Hole Simple"s, "Williams"s, "Gottlieb"s, "Cup 2"s }, //
         [this](const Kicker* kicker) { return kicker == m_kicker ? m_kickerType : kicker->m_d.m_kickertype; }, //
         [this](Kicker* kicker, int v) { kicker->m_d.m_kickertype = static_cast<KickerType>(v); (kicker == m_kicker ? m_kickerType : kicker->m_d.m_kickertype) = static_cast<KickerType>(v); });
      props.InputFloat<Kicker>(
         m_kicker, "Radius"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_radius; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Kicker>(
         m_kicker, "Orientation"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_orientation; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_orientation = v; }, PropertyPane::Unit::Degree, 1);
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat<Kicker>(
         m_kicker, "X", //
         [](const Kicker* kicker) { return kicker->m_d.m_vCenter.x; }, //
         [](Kicker* kicker, float v) { kicker->Translate(Vertex2D(v - kicker->m_d.m_vCenter.x, 0.f)); }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Kicker>(
         m_kicker, "Y", //
         [](const Kicker* kicker) { return kicker->m_d.m_vCenter.y; }, //
         [](Kicker* kicker, float v) { kicker->Translate(Vertex2D(0.f, v - kicker->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Kicker>(
         m_kicker, "Surface", //
         [](const Kicker* kicker) { return kicker->m_d.m_szSurface; }, //
         [](Kicker* kicker, const string& v) { kicker->m_d.m_szSurface = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Kicker>(
         m_kicker, "Enabled", //
         [](const Kicker* kicker) { return kicker->m_d.m_enabled; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_enabled = v; });
      props.Checkbox<Kicker>(
         m_kicker, "Fall Through", //
         [](const Kicker* kicker) { return kicker->m_d.m_fallThrough; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_fallThrough = v; });
      props.Checkbox<Kicker>(
         m_kicker, "Legacy (approx. physics)", //
         [](const Kicker* kicker) { return kicker->m_d.m_legacyMode; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_legacyMode = v; });
      props.InputFloat<Kicker>(
         m_kicker, "Scatter angle", //
         [](const Kicker* kicker) { return kicker->m_d.m_scatter; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Kicker>(
         m_kicker, "Hit Accuracy", //
         [](const Kicker* kicker) { return kicker->m_d.m_hitAccuracy; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_hitAccuracy = v; }, PropertyPane::Unit::Percent, 2);
      props.InputFloat<Kicker>(
         m_kicker, "Hit Height", //
         [](const Kicker* kicker) { return kicker->m_d.m_hit_height; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_scatter = v; }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   props.TimerSection<Kicker>(m_kicker, [](Kicker* obj) { return &(obj->m_d.m_tdr); });
}

}