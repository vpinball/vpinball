// license:GPLv3+

#include "core/stdafx.h"

#include "parts/light.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/LightWinUIPart.h"

LightWinUIPart::LightWinUIPart(PinTableWnd* editor, Light* light)
   : m_editor(editor)
   , m_light(light)
{
}

void LightWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_light->m_ptable->RenderSolid() ? (((m_light->m_d.m_color & 0xFEFEFE) + (m_light->m_d.m_color2 & 0xFEFEFE)) / 2) : -1);
   psur->SetObject(m_light);

   switch (m_light->m_d.m_shape)
   {
   default:
   case ShapeCustom:
      vector<RenderVertex> vvertex;
      m_light->GetRgVertex(vvertex);

      // Check if we should display the image in the editor.
      psur->Polygon(vvertex);

      break;
   }
}

void LightWinUIPart::UIRenderPass2(Sur* const psur)
{
   bool drawDragpoints = ((m_light->m_selectstate != ISelect::SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints));

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_light->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_light->m_vdpoint[i];
         if (pdp->m_selectstate != ISelect::SelectState::NotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   RenderOutline(psur);

   if ((m_light->m_d.m_shape == ShapeCustom) && drawDragpoints)
   {
      for (size_t i = 0; i < m_light->m_vdpoint.size(); i++)
      {
         CComObject<DragPoint>* const pdp = m_light->m_vdpoint[i];
         psur->SetFillColor(-1);
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(0, 0, 200), false, 0);
         psur->SetObject(pdp);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void LightWinUIPart::RenderOutline(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_light);
   psur->SetObject(nullptr);

   switch (m_light->m_d.m_shape)
   {
   case ShapeCircle:
   default:
   {
      psur->Ellipse(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y, m_light->m_d.m_falloff);
      break;
   }

   case ShapeCustom:
   {
      vector<RenderVertex> vvertex;
      m_light->GetRgVertex(vvertex);
      psur->SetBorderColor(RGB(255, 0, 0), false, 0);
      psur->Ellipse(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y, m_light->m_d.m_falloff);
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->Polygon(vvertex);

      psur->SetObject((ISelect*)&m_light->m_lightcenter);
      break;
   }
   }

   if (m_light->m_d.m_shape == ShapeCustom || m_editor->m_vpxEditor->m_alwaysDrawLightCenters)
   {
      psur->Line(m_light->m_d.m_vCenter.x - 10.0f, m_light->m_d.m_vCenter.y, m_light->m_d.m_vCenter.x + 10.0f, m_light->m_d.m_vCenter.y);
      psur->Line(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y - 10.0f, m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y + 10.0f);
   }

   if (m_light->m_d.m_showBulbMesh)
   {
      psur->SetBorderColor(RGB(0, 127, 255), false, 0);
      psur->Ellipse(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y, m_light->m_d.m_meshRadius * 0.5f);
   }
}

void LightWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   RenderOutline(psur);
}
