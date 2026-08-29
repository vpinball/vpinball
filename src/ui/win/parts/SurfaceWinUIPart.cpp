// license:GPLv3+

#include "core/stdafx.h"

#include "parts/surface.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/SurfaceWinUIPart.h"

SurfaceWinUIPart::SurfaceWinUIPart(PinTableWnd* editor, Surface* surface)
   : m_editor(editor)
   , m_surface(surface)
{
}

void SurfaceWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetFillColor(m_surface->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetObject(m_surface);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   vector<RenderVertex> vvertex;
   m_surface->GetRgVertex(vvertex);
   if (!m_surface->m_ptable->RenderSolid() || !m_surface->m_d.m_displayTexture)
      psur->Polygon(vvertex);
   else if (const Texture* const ppi = m_surface->m_ptable->GetImage(m_surface->m_d.m_szImage); ppi && ppi->GetGDIBitmap())
      psur->PolygonImage(vvertex, ppi->GetGDIBitmap(), m_surface->m_ptable->m_left, m_surface->m_ptable->m_top, m_surface->m_ptable->m_right, m_surface->m_ptable->m_bottom, ppi->m_width, ppi->m_height);
   else
      psur->Polygon(vvertex);
}

void SurfaceWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_surface); // For selected formatting
   psur->SetObject(nullptr);

   {
      vector<RenderVertex> vvertex; //!! check/reuse from prerender
      m_surface->GetRgVertex(vvertex);
      psur->Polygon(vvertex);
   }

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   bool drawDragpoints = ((m_surface->m_selectstate != ISelect::SelectState::NotSelected) || m_editor->m_vpxEditor->m_alwaysDrawDragPoints);

   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_surface->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_surface->m_vdpoint[i];
         if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   for (size_t i = 0; i < m_surface->m_vdpoint.size(); i++)
   {
      CComObject<DragPoint>* const pdp = m_surface->m_vdpoint[i];
      if (!(drawDragpoints || pdp->m_slingshot))
         continue;
      psur->SetFillColor(-1);
      psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);

      if (drawDragpoints)
      {
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }

      if (pdp->m_slingshot)
      {
         psur->SetObject(nullptr);
         const CComObject<DragPoint>* const pdp2 = m_surface->m_vdpoint[(i < m_surface->m_vdpoint.size() - 1) ? (i + 1) : 0];
         psur->SetLineColor(RGB(0, 0, 0), false, 3);

         psur->Line(pdp->m_v.x, pdp->m_v.y, pdp2->m_v.x, pdp2->m_v.y);
      }
   }
}

void SurfaceWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   // Don't render dragpoints for blueprint
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_surface); // For selected formatting
   psur->SetObject(nullptr);

   vector<RenderVertex> vvertex;
   m_surface->GetRgVertex(vvertex);

   psur->Polygon(vvertex);
}
