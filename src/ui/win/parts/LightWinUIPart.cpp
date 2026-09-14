// license:GPLv3+

#include "core/stdafx.h"

#include "parts/light.h"
#include "ui/win/DragPointDialogs.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/WinUIPartRegistry.h"
#include "ui/win/parts/LightWinUIPart.h"

LightWinUIPart::LightWinUIPart(PinTableWnd* editor, Light* light)
   : IWinUIPart(editor, light)
   , m_light(light)
   , m_pointParts(editor, light)
{
}

IWinUIPart* LightWinUIPart::GetSubPart(ISelect* select)
{
   if (select == m_light->GetLightCenterSelect())
   {
      if (!m_centerPart)
         m_centerPart = WinUIPartRegistry::Create(m_editor, select);
      return m_centerPart.get();
   }
   return m_pointParts.Get(select);
}

void LightWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(m_light->m_d.m_vCenter.x, m_light->m_d.m_vCenter.y);
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
   bool drawDragpoints = ((m_selectstate != SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints));

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_light->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_light->m_vdpoint[i];
         if (m_pointParts.IsSelected(pdp))
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
         psur->SetBorderColor(m_pointParts.IsDragging(pdp) ? RGB(0, 255, 0) : RGB(0, 0, 200), false, 0);
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

      psur->SetObject(m_light->GetLightCenterSelect());
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

void LightWinUIPart::EditMenu(CMenu& menu)
{
   menu.EnableMenuItem(ID_WALLMENU_FLIP, MF_BYCOMMAND | ((m_light->m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   menu.EnableMenuItem(ID_WALLMENU_MIRROR, MF_BYCOMMAND | ((m_light->m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   menu.EnableMenuItem(ID_WALLMENU_ROTATE, MF_BYCOMMAND | ((m_light->m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   menu.EnableMenuItem(ID_WALLMENU_SCALE, MF_BYCOMMAND | ((m_light->m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   menu.EnableMenuItem(ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_light->m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
}

void LightWinUIPart::DoCommand(int icmd, int x, int y)
{
   IWinUIPart::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP: m_light->FlipPointY(m_light->GetPointCenter()); break;

   case ID_WALLMENU_MIRROR: m_light->FlipPointX(m_light->GetPointCenter()); break;

   case ID_WALLMENU_ROTATE: (void)VPX::WinUI::RotatePointsDialog(m_light); break;

   case ID_WALLMENU_SCALE: (void)VPX::WinUI::ScalePointsDialog(m_light); break;

   case ID_WALLMENU_TRANSLATE: (void)VPX::WinUI::TranslatePointsDialog(m_light); break;

   case ID_WALLMENU_ADDPOINT: m_light->AddPoint(m_editor->TransformPoint(x, y), true); break;
   }
}
