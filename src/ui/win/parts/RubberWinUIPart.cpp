// license:GPLv3+

#include "core/stdafx.h"

#include "parts/rubber.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/RubberWinUIPart.h"

RubberWinUIPart::RubberWinUIPart(PinTableWnd* editor, Rubber* rubber)
   : m_editor(editor)
   , m_rubber(rubber)
{
}

void RubberWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   if (m_rubber->m_ptable->RenderSolid())
      psur->SetFillColor(RGB(192, 192, 192));
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(m_rubber);

   if (!m_rubber->m_d.m_showInEditor)
   {
      int cvertex;
      const Vertex2D* const rgvLocal = m_rubber->GetSplineVertex(cvertex, nullptr, nullptr, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));
      psur->Polygon(rgvLocal, cvertex * 2);
      delete[] rgvLocal;
   }
   else
   {
      m_rubber->DrawRubberMesh(psur);
   }
}

void RubberWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_rubber);
   psur->SetObject(nullptr); // nullptr so this won't be hit-tested

   if (!m_rubber->m_d.m_showInEditor)
   {
      int cvertex;
      bool* pfCross;
      const Vertex2D* const rgvLocal = m_rubber->GetSplineVertex(cvertex, &pfCross, nullptr, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));

      psur->Polygon(rgvLocal, cvertex * 2);
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

      delete[] rgvLocal;
      delete[] pfCross;
   }
   else
   {
      m_rubber->DrawRubberMesh(psur);

      // if rotation is used don't show dragpoints
      return;
   }

   bool drawDragpoints = ((m_rubber->m_selectstate != ISelect::SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints));

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_rubber->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_rubber->m_vdpoint[i];
         if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   if (drawDragpoints)
   {
      for (size_t i = 0; i < m_rubber->m_vdpoint.size(); i++)
      {
         CComObject<DragPoint>* const pdp = m_rubber->m_vdpoint[i];
         psur->SetFillColor(-1);
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
         psur->SetObject(pdp);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void RubberWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_rubber);
   psur->SetObject(nullptr); // nullptr so this won't be hit-tested

   if (!m_rubber->m_d.m_showInEditor)
   {
      int cvertex;
      bool* pfCross;
      const Vertex2D* const rgvLocal = m_rubber->GetSplineVertex(cvertex, &pfCross, nullptr, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));

      psur->Polygon(rgvLocal, cvertex * 2);
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

      delete[] rgvLocal;
      delete[] pfCross;
   }
}
