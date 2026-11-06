#include "core/stdafx.h"

#include "RubberUIPart.h"

#include "../BlueprintRenderContext.h"
#include "math/matrix.h"
#include "utils/color.h"

namespace VPX::EditorUI
{

RubberUIPart::RubberUIPart(Rubber* rubber)
   : EditableUIPart(rubber, { &Data::m_visible })
{
}

RubberUIPart::TransformMask RubberUIPart::GetTransform(Matrix3D& transform) { return GetCurveTransform(transform, m_part->m_d.m_height); }

void RubberUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   SetCurveTransform(pos, scale, rot);
   m_part->m_d.m_height += pos.z - m_part->m_d.m_height;
}

void RubberUIPart::RenderOverlay(const EditorRenderContext& ctx)
{
   ctx.DrawWireframe(m_part);
}

void RubberUIPart::RenderBlueprint(const BlueprintRenderContext& ctx)
{
   if (ctx.GetPass() == BlueprintRenderContext::Pass::Fill)
   {
      ctx.SetLineColor(RGB(0, 0, 0), false, 0);
      if (m_part->GetPTable()->RenderSolid())
         ctx.SetFillColor(RGB(192, 192, 192));
      else
         ctx.SetFillColor(-1);
      ctx.SetBorderColor(-1, false, 0);
      ctx.SetObject(this);

      if (!m_part->m_d.m_showInEditor)
      {
         vector<Vertex2D> outline;
         m_part->GetEditorOutline(outline, nullptr, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));
         if (!outline.empty())
            ctx.Polygon(outline.data(), (int)outline.size());
      }
      else
      {
         vector<Vertex2D> edges;
         m_part->GetEditorWireframe(edges);
         if (!edges.empty())
            ctx.Lines(edges.data(), (int)(edges.size() / 2));
      }
   }
   else
   {
      ctx.SetFillColor(-1);
      ctx.SetBorderColor(RGB(0, 0, 0), false, 0);
      ctx.SetLineColor(RGB(0, 0, 0), false, 0);
      ctx.SetObject(this);
      ctx.SetObject(nullptr); // nullptr so this won't be hit-tested

      if (!m_part->m_d.m_showInEditor)
      {
         vector<Vertex2D> outline;
         vector<bool> crossFlags;
         m_part->GetEditorOutline(outline, &crossFlags, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));

         if (!outline.empty())
            ctx.Polygon(outline.data(), (int)outline.size());
         for (size_t i = 0; i < crossFlags.size(); i++)
            if (crossFlags[i])
               ctx.Line(outline[i].x, outline[i].y, outline[outline.size() - i - 1].x, outline[outline.size() - i - 1].y);
      }
      else
      {
         vector<Vertex2D> edges;
         m_part->GetEditorWireframe(edges);
         if (!edges.empty())
            ctx.Lines(edges.data(), (int)(edges.size() / 2));

         // if rotation is used don't show dragpoints
         return;
      }

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
            ctx.SetBorderColor((ctx.m_hotPoint == pdp.get()) ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
            ctx.SetObject(this, pdp.get());

            ctx.Ellipse2(pdp->GetX(), pdp->GetY(), 8);
         }
      }
   }
}

void RubberUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("Rubber"s, m_part);

   UpdateCurveSection(props);

   if (props.BeginSection("Visuals"s))
   {
      props.MaterialVisuals(m_part, "Material"s, &Data::m_szMaterial, "Image"s, &Data::m_szImage);
      props.Checkbox<Rubber>(
         m_part, "Static Rendering"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_staticRendering; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_staticRendering = v; }, "Bake this part into the static prerender pass (faster, but it cannot move or change at runtime)");
      props.Checkbox<Rubber>(
         m_part, "Visible"s, //
         [this](const Rubber* rubber) { return GetVisibility(rubber); }, //
         [this](Rubber* rubber, bool v) { SetVisibility(rubber, v); });
      props.Checkbox<Rubber>(
         m_part, "Reflection Enabled"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_reflectionEnabled; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_reflectionEnabled = v; }, "Render this part in the table's reflections");
      props.InputInt<Rubber>(
         m_part, "Thickness"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_thickness; }, //
         [](Rubber* rubber, int v) { rubber->m_d.m_thickness = v; });
      props.EndSection();
   }

   if (props.BeginSection("Position"s))
   {
      props.InputFloat<Rubber>(
         m_part, "Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_height; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_height = v; }, PropertyPane::Unit::VPLength, 1);
      props.InputFloat3<Rubber>(
         m_part, "Rotation"s, //
         [](const Rubber* rubber) { return vec3(rubber->m_d.m_rotX, rubber->m_d.m_rotY, rubber->m_d.m_rotZ); }, //
         [](Rubber* rubber, const vec3& v)
         {
            rubber->m_d.m_rotX = v.x;
            rubber->m_d.m_rotY = v.y;
            rubber->m_d.m_rotZ = v.z;
         },
         PropertyPane::Unit::Degree, 1);
      props.Checkbox<Rubber>(
         m_part, "Show in Editor"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_showInEditor; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_showInEditor = v; }, "Draw this rubber in the editor view (does not affect gameplay)");
      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {
      props.MaterialCombo<Rubber>(
         m_part, "Physics Material"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_szPhysicsMaterial; }, //
         [](Rubber* rubber, const string& v) { rubber->m_d.m_szPhysicsMaterial = v; }, "Material preset used for this part's physics values");
      props.Checkbox<Rubber>(
         m_part, "Overwrite Physics"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_overwritePhysics; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_overwritePhysics = v; }, "Use the fields below instead of the physics material's values");
      props.InputFloat<Rubber>(
         m_part, "Elasticy"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticity; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Elasticy Falloff"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_elasticityFalloff; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_elasticityFalloff = v; }, PropertyPane::Unit::None, 2, "Reduce elasticity for slower impacts (0 = constant elasticity)");
      props.InputFloat<Rubber>(
         m_part, "Friction"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_friction; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_friction = max(v, 0.f); }, PropertyPane::Unit::None, 2);
      props.InputFloat<Rubber>(
         m_part, "Scatter Angle"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_scatter; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_scatter = v; }, PropertyPane::Unit::None, 2, "Random angle added to the ball's bounce direction");
      props.InputFloat<Rubber>(
         m_part, "Hit Height"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitHeight; }, //
         [](Rubber* rubber, float v) { rubber->m_d.m_hitHeight = v; }, PropertyPane::Unit::VPLength, 2, "Height of the collision volume above the table surface");
      props.Checkbox<Rubber>(
         m_part, "Collidable"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_collidable; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_collidable = v; });
      props.Checkbox<Rubber>(
         m_part, "Has Hit Event"s, //
         [](const Rubber* rubber) { return rubber->m_d.m_hitEvent; }, //
         [](Rubber* rubber, bool v) { rubber->m_d.m_hitEvent = v; }, "Fire a Hit event to the script when the ball hits this part");
      props.EndSection();
   }

   props.TimerSection(m_part);
}

}
