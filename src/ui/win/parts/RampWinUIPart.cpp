// license:GPLv3+

#include "core/stdafx.h"

#include "parts/ramp.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/RampWinUIPart.h"

RampWinUIPart::RampWinUIPart(PinTableWnd* editor, Ramp* ramp)
   : m_editor(editor)
   , m_ramp(ramp)
{
}

void RampWinUIPart::UIRenderPass1(Sur* const psur)
{
   // make 1-wire ramps look unique in editor - uses ramp color
   psur->SetFillColor(m_ramp->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(m_ramp);

   int cvertex;
   const Vertex2D* const rgvLocal = m_ramp->GetRampVertex(cvertex, nullptr, nullptr, nullptr, nullptr, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   delete[] rgvLocal;
}

void RampWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_ramp);
   psur->SetObject(nullptr); // nullptr so this won't be hit-tested

   bool* pfCross;
   Vertex2D* middlePoints;
   int cvertex;
   const Vertex2D* const rgvLocal = m_ramp->GetRampVertex(cvertex, nullptr, &pfCross, nullptr, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (m_ramp->IsHabitrail())
   {
      psur->Polyline(middlePoints, cvertex);
      if (m_ramp->m_d.m_type == RampType4Wire || m_ramp->m_d.m_type == RampType3WireRight)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(rgvLocal, cvertex);
      }
      if (m_ramp->m_d.m_type == RampType4Wire || m_ramp->m_d.m_type == RampType3WireLeft)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(&rgvLocal[cvertex], cvertex);
      }
   }
   else
   {
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);
   }

   delete[] rgvLocal;
   delete[] pfCross;
   delete[] middlePoints;

   bool drawDragpoints = ((m_ramp->m_selectstate != ISelect::SelectState::NotSelected) || m_editor->m_vpxEditor->m_alwaysDrawDragPoints);
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the drag points of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_ramp->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_ramp->m_vdpoint[i];
         if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   if (drawDragpoints)
   {
      for (size_t i = 0; i < m_ramp->m_vdpoint.size(); i++)
      {
         CComObject<DragPoint>* const pdp = m_ramp->m_vdpoint[i];
         psur->SetFillColor(-1);
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : ((i == 0) ? RGB(0, 0, 255) : RGB(255, 0, 0)), false, 0);
         psur->SetObject(pdp);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void RampWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_ramp);
   psur->SetObject(nullptr); // nullptr so this won't be hit-tested

   bool* pfCross;
   Vertex2D* middlePoints;
   int cvertex;
   const Vertex2D* const rgvLocal = m_ramp->GetRampVertex(cvertex, nullptr, &pfCross, nullptr, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (m_ramp->IsHabitrail())
   {
      psur->Polyline(middlePoints, cvertex - 1);
      if (m_ramp->m_d.m_type == RampType4Wire || m_ramp->m_d.m_type == RampType3WireRight)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(rgvLocal, cvertex);
      }
      if (m_ramp->m_d.m_type == RampType4Wire || m_ramp->m_d.m_type == RampType3WireLeft)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(&rgvLocal[cvertex], cvertex);
      }
   }

   for (int i = 0; i < cvertex; i++)
      if (pfCross[i])
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

   delete[] rgvLocal;
   delete[] pfCross;
   delete[] middlePoints;
}
