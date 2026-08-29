// license:GPLv3+

#include "core/stdafx.h"

#include "parts/flasher.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/FlasherWinUIPart.h"

FlasherWinUIPart::FlasherWinUIPart(PinTableWnd* editor, Flasher* flasher)
   : m_editor(editor)
   , m_flasher(flasher)
{
}

void FlasherWinUIPart::UIRenderPass1(Sur * const psur)
{
   if (m_flasher->m_vdpoint.empty())
      m_flasher->InitShape();

   psur->SetFillColor(m_flasher->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetObject(m_flasher);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   vector<RenderVertex> vvertex;
   m_flasher->GetRgVertex(vvertex);
   if (!m_flasher->m_ptable->RenderSolid() || !m_flasher->m_d.m_displayTexture)
   {
      psur->Polygon(vvertex);
   }
   else if (const Texture *const ppi = m_flasher->m_ptable->GetImage(m_flasher->m_d.m_szImageA); ppi && ppi->GetGDIBitmap())
   {
      if (m_flasher->m_d.m_imagealignment == ImageModeWrap)
      {
         float _minx = FLT_MAX;
         float _miny = FLT_MAX;
         float _maxx = -FLT_MAX;
         float _maxy = -FLT_MAX;
         for (const auto& v : vvertex)
         {
            if (v.x < _minx) _minx = v.x;
            if (v.x > _maxx) _maxx = v.x;
            if (v.y < _miny) _miny = v.y;
            if (v.y > _maxy) _maxy = v.y;
         }

         psur->PolygonImage(vvertex, ppi->GetGDIBitmap(), _minx, _miny, _minx + (_maxx - _minx), _miny + (_maxy - _miny), ppi->m_width, ppi->m_height);
      }
      else
      {
         psur->PolygonImage(vvertex, ppi->GetGDIBitmap(), m_flasher->m_ptable->m_left, m_flasher->m_ptable->m_top, m_flasher->m_ptable->m_right, m_flasher->m_ptable->m_bottom, ppi->m_width, ppi->m_height);
      }
   }
   else
   {
      psur->Polygon(vvertex);
   }
}

void FlasherWinUIPart::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_flasher); // For selected formatting
   psur->SetObject(nullptr);

   vector<RenderVertex> vvertex; //!! check/reuse from UIRenderPass1
   m_flasher->GetRgVertex(vvertex);
   psur->Polygon(vvertex);

   // Except for flasher mode, shape is simplified before rendering into its bounding rectangle
   if (m_flasher->m_d.m_renderMode != FlasherData::RenderMode::FLASHER)
   {
      float _minx = FLT_MAX;
      float _miny = FLT_MAX;
      float _maxx = -FLT_MAX;
      float _maxy = -FLT_MAX;
      for (const auto& v : vvertex)
      {
         if (v.x < _minx) _minx = v.x;
         if (v.x > _maxx) _maxx = v.x;
         if (v.y < _miny) _miny = v.y;
         if (v.y > _maxy) _maxy = v.y;
      }
      psur->Rectangle(_minx, _miny, _maxx, _maxy);
   }

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   bool drawDragpoints = ((m_flasher->m_selectstate != ISelect::SelectState::NotSelected) || m_editor->m_vpxEditor->m_alwaysDrawDragPoints);
   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (const auto& pdp : m_flasher->m_vdpoint)
      {
         if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   if (drawDragpoints)
   {
      psur->SetFillColor(-1);
      for (const auto &pdp : m_flasher->m_vdpoint)
      {
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }

   // Little cross at the object center
   psur->Line(m_flasher->m_d.m_vCenter.x - 10.0f, m_flasher->m_d.m_vCenter.y, m_flasher->m_d.m_vCenter.x + 10.0f, m_flasher->m_d.m_vCenter.y);
   psur->Line(m_flasher->m_d.m_vCenter.x, m_flasher->m_d.m_vCenter.y - 10.0f, m_flasher->m_d.m_vCenter.x, m_flasher->m_d.m_vCenter.y + 10.0f);
}
