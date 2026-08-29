// license:GPLv3+

#include "core/stdafx.h"

#include "parts/trigger.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TriggerWinUIPart.h"

TriggerWinUIPart::TriggerWinUIPart(PinTableWnd* editor, Trigger* trigger)
   : m_editor(editor)
   , m_trigger(trigger)
{
}

void TriggerWinUIPart::UIRenderPass1(Sur* const psur)
{
   if (m_trigger->m_vdpoint.empty())
      m_trigger->InitShape(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y);

   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(m_trigger);

   if (m_trigger->m_d.m_shape != TriggerStar && m_trigger->m_d.m_shape != TriggerButton)
   {
      psur->SetFillColor(m_trigger->m_ptable->RenderSolid() ? RGB(200, 220, 200) : -1);

      vector<RenderVertex> vvertex;
      m_trigger->GetRgVertex(vvertex);

      psur->Polygon(vvertex);
   }
   else
   {
      psur->SetFillColor(-1);
      psur->Ellipse(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y, m_trigger->m_d.m_radius);
   }
}

void TriggerWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_trigger);
   psur->SetFillColor(-1);

   if (m_trigger->m_d.m_shape != TriggerStar && m_trigger->m_d.m_shape != TriggerButton)
   {
      vector<RenderVertex> vvertex;
      m_trigger->GetRgVertex(vvertex);

      psur->SetObject(nullptr);
      psur->SetBorderColor(RGB(0, 180, 0), false, 1);

      psur->Polygon(vvertex);

      bool drawDragpoints = (m_trigger->m_selectstate != ISelect::SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints);
      // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
      if (!drawDragpoints)
      {
         // if any of the dragpoints of this object are selected then draw all the dragpoints
         for (size_t i = 0; i < m_trigger->m_vdpoint.size(); i++)
         {
            const CComObject<DragPoint>* const pdp = m_trigger->m_vdpoint[i];
            if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
            {
               drawDragpoints = true;
               break;
            }
         }
      }

      if (drawDragpoints)
      {
         for (size_t i = 0; i < m_trigger->m_vdpoint.size(); i++)
         {
            CComObject<DragPoint>* const pdp = m_trigger->m_vdpoint[i];
            psur->SetFillColor(-1);
            psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(0, 180, 0), false, 0);
            psur->SetObject(pdp);

            psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
         }
      }
   }
   else
   {
      psur->SetObject(nullptr);
      psur->SetBorderColor(RGB(0, 180, 0), false, 1);

      psur->Line(m_trigger->m_d.m_vCenter.x - m_trigger->m_d.m_radius, m_trigger->m_d.m_vCenter.y, m_trigger->m_d.m_vCenter.x + m_trigger->m_d.m_radius, m_trigger->m_d.m_vCenter.y);
      psur->Line(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y - m_trigger->m_d.m_radius, m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y + m_trigger->m_d.m_radius);

      static const float sp4 = (float)sin(M_PI / 4.0);
      const float r2 = m_trigger->m_d.m_radius * sp4;

      psur->Line(m_trigger->m_d.m_vCenter.x - r2, m_trigger->m_d.m_vCenter.y - r2, m_trigger->m_d.m_vCenter.x + r2, m_trigger->m_d.m_vCenter.y + r2);
      psur->Line(m_trigger->m_d.m_vCenter.x - r2, m_trigger->m_d.m_vCenter.y + r2, m_trigger->m_d.m_vCenter.x + r2, m_trigger->m_d.m_vCenter.y - r2);
   }

   if (m_trigger->m_d.m_shape == TriggerWireA || m_trigger->m_d.m_shape == TriggerWireB || m_trigger->m_d.m_shape == TriggerWireC || m_trigger->m_d.m_shape == TriggerWireD || m_trigger->m_d.m_shape == TriggerInder)
   {
      if (m_trigger->m_numIndices > 0)
      {
         const size_t numPts = m_trigger->m_numIndices / 3 + 1;
         vector<Vertex2D> drawVertices(numPts);

         const Vertex3Ds& A = m_trigger->m_vertices[m_trigger->m_faceIndices[0]];
         drawVertices[0] = Vertex2D(A.x, A.y);

         size_t o = 1;
         for (int i = 0; i < m_trigger->m_numIndices; i += 3, ++o)
         {
            const Vertex3Ds& B = m_trigger->m_vertices[m_trigger->m_faceIndices[i + 1]];
            drawVertices[o] = Vertex2D(B.x, B.y);
         }

         psur->Polyline(drawVertices.data(), (int)drawVertices.size());
      }
   }
}

void TriggerWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_trigger);

   psur->Ellipse(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y, m_trigger->m_d.m_radius);
}
