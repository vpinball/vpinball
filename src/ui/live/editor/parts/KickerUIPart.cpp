#include "core/stdafx.h"

#include "KickerUIPart.h"

#include "../BlueprintRenderContext.h"
#include "math/matrix.h"
#include "utils/color.h"

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

void KickerUIPart::RenderBlueprint(const BlueprintRenderContext& ctx)
{
   if (ctx.GetPass() == BlueprintRenderContext::Pass::Fill)
      return;

   ctx.SetBorderColor(RGB(0, 0, 0), false, 0);
   ctx.SetFillColor(-1);
   ctx.SetObject(this);

   // Draw Arrow to display (default) orientation
   {
      const float radangle = ANGTORAD(m_part->m_d.m_orientation);
      constexpr float halflength = 50.0f;

      ctx.SetLineColor(RGB(255, 0, 0), false, 1);

      Vertex2D tmp;
      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         constexpr float len1 = halflength * 0.5f;
         tmp.x = m_part->m_d.m_vCenter.x + sn * len1;
         tmp.y = m_part->m_d.m_vCenter.y - cs * len1;
      }

      ctx.Line(tmp.x, tmp.y, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
      constexpr float len2 = halflength * 0.25f;
      {
         const float arrowang = radangle + 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         ctx.Line(tmp.x, tmp.y, m_part->m_d.m_vCenter.x + sn * len2, m_part->m_d.m_vCenter.y - cs * len2);
      }
      {
         const float arrowang = radangle - 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         ctx.Line(tmp.x, tmp.y, m_part->m_d.m_vCenter.x + sn * len2, m_part->m_d.m_vCenter.y - cs * len2);
      }
   }

   ctx.Ellipse(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_d.m_radius);
   ctx.Ellipse(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_d.m_radius * 0.75f);
   ctx.Ellipse(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_d.m_radius * 0.5f);
   ctx.Ellipse(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_d.m_radius * 0.25f);
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
         [](Kicker* kicker, const string& v) { kicker->m_d.m_szSurface = v; }, "Surface this part rests on (defines its base height)");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Kicker>(
         m_part, "Enabled"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_enabled; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_enabled = v; }, "Capture balls entering this kicker");
      props.Checkbox<Kicker>(
         m_part, "Fall Through"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_fallThrough; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_fallThrough = v; }, "Let the ball fall through the playfield instead of capturing it");
      props.Checkbox<Kicker>(
         m_part, "Legacy (approx. physics)"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_legacyMode; }, //
         [](Kicker* kicker, bool v) { kicker->m_d.m_legacyMode = v; }, "Use the simplified legacy kicker physics instead of the accurate one");
      props.InputFloat<Kicker>(
         m_part, "Scatter angle"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_scatter; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2, "Random angle added to the ball's bounce direction");
      props.InputFloat<Kicker>(
         m_part, "Hit Accuracy"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_hitAccuracy; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_hitAccuracy = v; }, PropertyPane::Unit::Percent, 2,
         "Portion of the kicker's radius within which a ball is captured (a ball hitting the edge is not)");
      props.InputFloat<Kicker>(
         m_part, "Hit Height"s, //
         [](const Kicker* kicker) { return kicker->m_d.m_hit_height; }, //
         [](Kicker* kicker, float v) { kicker->m_d.m_hit_height = v; }, PropertyPane::Unit::VPLength, 1, "Height of the collision volume above the table surface");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
