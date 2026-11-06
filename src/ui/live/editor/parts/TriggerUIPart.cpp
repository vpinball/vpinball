#include "core/stdafx.h"

#include "TriggerUIPart.h"

#include "../BlueprintRenderContext.h"
#include "math/matrix.h"
#include "utils/color.h"

namespace VPX::EditorUI
{

TriggerUIPart::TriggerUIPart(Trigger* trigger)
   : EditableUIPart(trigger, { &Data::m_visible })
{
}

TriggerUIPart::TransformMask TriggerUIPart::GetTransform(Matrix3D& transform)
{
   const float height = m_part->GetPTable()->GetSurfaceHeight(m_part->m_d.m_szSurface, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y);
   return GetCurveTransform(transform, height);
}

void TriggerUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) { SetCurveTransform(pos, scale, rot); }

void TriggerUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawHitObjects(m_part);
}

void TriggerUIPart::RenderBlueprint(const BlueprintRenderContext& ctx)
{
   if (ctx.GetPass() == BlueprintRenderContext::Pass::Fill)
   {
      ctx.SetBorderColor(-1, false, 0);
      ctx.SetObject(this);

      if (m_part->m_d.m_shape != TriggerStar && m_part->m_d.m_shape != TriggerButton)
      {
         ctx.SetFillColor(m_part->GetPTable()->RenderSolid() ? RGB(200, 220, 200) : -1);

         vector<RenderVertex> vvertex;
         m_part->m_curve.GetRgVertex(vvertex);

         ctx.Polygon(vvertex);
      }
      else
      {
         ctx.SetFillColor(-1);
         ctx.Ellipse(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y, m_part->m_d.m_radius);
      }
   }
   else
   {
      ctx.SetLineColor(RGB(0, 0, 0), false, 0);
      ctx.SetObject(this);
      ctx.SetFillColor(-1);

      if (m_part->m_d.m_shape != TriggerStar && m_part->m_d.m_shape != TriggerButton)
      {
         vector<RenderVertex> vvertex;
         m_part->m_curve.GetRgVertex(vvertex);

         ctx.SetObject(nullptr);
         ctx.SetBorderColor(RGB(0, 180, 0), false, 1);

         ctx.Polygon(vvertex);

         bool drawDragpoints = (ctx.m_partSelected || (this == ctx.m_pointEditPart) || g_settingsService.GetAppSettings().GetEditor_ShowDragPoints());
         // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
         if (!drawDragpoints)
         {
            // if any of the dragpoints of this object are selected then draw all the dragpoints
            for (const auto& pdp : m_part->m_curve.GetPoints())
            {
               if (ctx.IsPointSelected(pdp.get()))
               {
                  drawDragpoints = true;
                  break;
               }
            }
         }

         if (drawDragpoints)
         {
            for (const auto& pdp : m_part->m_curve.GetPoints())
            {
               ctx.SetFillColor(-1);
               ctx.SetBorderColor((ctx.m_hotPoint == pdp.get()) ? RGB(0, 255, 0) : RGB(0, 180, 0), false, 0);
               ctx.SetObject(this, pdp.get());

               ctx.Ellipse2(pdp->GetX(), pdp->GetY(), 8);
            }
         }
      }
      else
      {
         ctx.SetObject(nullptr);
         ctx.SetBorderColor(RGB(0, 180, 0), false, 1);

         ctx.Line(m_part->m_d.m_vCenter.x - m_part->m_d.m_radius, m_part->m_d.m_vCenter.y, m_part->m_d.m_vCenter.x + m_part->m_d.m_radius, m_part->m_d.m_vCenter.y);
         ctx.Line(m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y - m_part->m_d.m_radius, m_part->m_d.m_vCenter.x, m_part->m_d.m_vCenter.y + m_part->m_d.m_radius);

         static const float sp4 = (float)sin(M_PI / 4.0);
         const float r2 = m_part->m_d.m_radius * sp4;

         ctx.Line(m_part->m_d.m_vCenter.x - r2, m_part->m_d.m_vCenter.y - r2, m_part->m_d.m_vCenter.x + r2, m_part->m_d.m_vCenter.y + r2);
         ctx.Line(m_part->m_d.m_vCenter.x - r2, m_part->m_d.m_vCenter.y + r2, m_part->m_d.m_vCenter.x + r2, m_part->m_d.m_vCenter.y - r2);
      }

      if (m_part->m_d.m_shape == TriggerWireA || m_part->m_d.m_shape == TriggerWireB || m_part->m_d.m_shape == TriggerWireC || m_part->m_d.m_shape == TriggerWireD
         || m_part->m_d.m_shape == TriggerInder)
      {
         vector<Vertex2D> outline;
         m_part->GetWireOutline(outline);
         if (!outline.empty())
            ctx.Polyline(outline.data(), (int)outline.size());
      }
   }
}

void TriggerUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Trigger"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      props.Checkbox<Trigger>(
         m_part, "Visible"s, //
         [this](const Trigger* trigger) { return GetVisibility(trigger); }, //
         [this](Trigger* trigger, bool v) { SetVisibility(trigger, v); });
      props.Checkbox<Trigger>(
         m_part, "Reflection Enabled"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_reflectionEnabled; }, //
         [](Trigger* trigger, bool v) { trigger->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
      props.Combo<Trigger>(
         m_part, "Shape"s, vector<string> { "None"s, "Wire A"s, "Star"s, "Wire B"s, "Button"s, "Wire C"s, "Wire D"s, "Inder"s },
         [](const Trigger* trigger) { return trigger->m_d.m_shape; }, //
         [](Trigger* trigger, int v) { trigger->m_d.m_shape = static_cast<TriggerShape>(v); });
      props.InputFloat<Trigger>(
         m_part, "Wire Thickness"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_wireThickness; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_wireThickness = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Trigger>(
         m_part, "Star Radius"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_radius; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_radius = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat<Trigger>(
         m_part, "Rotation"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_rotation; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_rotation = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<Trigger>(
         m_part, "Anim Speed"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_animSpeed; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_animSpeed = v; }, PropertyPane::Unit::None, 1, "Speed of the shape animation played when the trigger fires");
      props.MaterialCombo<Trigger>(
         m_part, "Material"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_szMaterial; }, //
         [](Trigger* trigger, const string& v) { trigger->m_d.m_szMaterial = v; });
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.Checkbox<Trigger>(
         m_part, "Enabled"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_enabled; }, //
         [](Trigger* trigger, bool v) { trigger->m_d.m_enabled = v; }, "Fire Hit/Unhit events when the ball passes over the trigger");
      ImGui::BeginDisabled(!props.GetEditedPart<Trigger>(m_part)->m_d.m_enabled);
      props.InputResURI<Trigger>(
         m_part, "State Link"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_stateSrcLink; }, //
         [](Trigger* trigger, const string& v) { trigger->m_d.m_stateSrcLink = v; }, //
         ResURIKind::State, "URI linking this trigger's state to a shared state source (plugin or external controller)", DisplayFamily::Any, CTLPI_STATE_TYPE_SWITCH, true, -1);
      ImGui::EndDisabled();
      props.InputFloat<Trigger>(
         m_part, "Hit Height"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_hit_height; }, //
         [](Trigger* trigger, float v) { trigger->m_d.m_hit_height = v; }, PropertyPane::Unit::VPLength, 1, "Height of the collision volume above the table surface");
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat2<Trigger>(
         m_part, "Position"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_vCenter; }, //
         [](Trigger* trigger, const Vertex2D& v) { trigger->Translate(Vertex2D(v.x - trigger->m_d.m_vCenter.x, v.y - trigger->m_d.m_vCenter.y)); }, PropertyPane::Unit::VPLength, 1);
      props.SurfaceCombo<Trigger>(
         m_part, "Surface"s, //
         [](const Trigger* trigger) { return trigger->m_d.m_szSurface; }, //
         [](Trigger* trigger, const string& v) { trigger->m_d.m_szSurface = v; }, "Surface this part rests on (defines its base height)");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
