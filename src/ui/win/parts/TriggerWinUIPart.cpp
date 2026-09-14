// license:GPLv3+

#include "core/stdafx.h"

#include "parts/trigger.h"
#include "ui/win/DragPointDialogs.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TriggerWinUIPart.h"

TriggerWinUIPart::TriggerWinUIPart(PinTableWnd* editor, Trigger* trigger)
   : IWinUIPart(editor, trigger)
   , m_trigger(trigger)
   , m_pointParts(editor, trigger)
{
}

void TriggerWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(m_trigger->m_d.m_vCenter.x, m_trigger->m_d.m_vCenter.y);
}

void TriggerWinUIPart::UIRenderPass1(Sur* const psur)
{
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

      bool drawDragpoints = (m_selectstate != SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints);
      // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
      if (!drawDragpoints)
      {
         // if any of the dragpoints of this object are selected then draw all the dragpoints
         for (size_t i = 0; i < m_trigger->m_vdpoint.size(); i++)
         {
            const CComObject<DragPoint>* const pdp = m_trigger->m_vdpoint[i];
            if (m_pointParts.IsSelected(pdp))
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
            psur->SetBorderColor(m_pointParts.IsDragging(pdp) ? RGB(0, 255, 0) : RGB(0, 180, 0), false, 0);
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
      vector<Vertex2D> outline;
      m_trigger->GetWireOutline(outline);
      if (!outline.empty())
         psur->Polyline(outline.data(), (int)outline.size());
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

void TriggerWinUIPart::EditMenu(CMenu& menu)
{
   menu.EnableMenuItem(ID_WALLMENU_FLIP, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_MIRROR, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_ROTATE, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_SCALE, MF_BYCOMMAND | MF_ENABLED);
   menu.EnableMenuItem(ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | MF_ENABLED);
}

void TriggerWinUIPart::DoCommand(int icmd, int x, int y)
{
   IWinUIPart::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP: m_trigger->FlipPointY(m_trigger->GetPointCenter()); break;

   case ID_WALLMENU_MIRROR: m_trigger->FlipPointX(m_trigger->GetPointCenter()); break;

   case ID_WALLMENU_ROTATE: (void)VPX::WinUI::RotatePointsDialog(m_trigger); break;

   case ID_WALLMENU_SCALE: (void)VPX::WinUI::ScalePointsDialog(m_trigger); break;

   case ID_WALLMENU_TRANSLATE: (void)VPX::WinUI::TranslatePointsDialog(m_trigger); break;

   case ID_WALLMENU_ADDPOINT:
   {
      m_trigger->BeginUndo();
      m_trigger->MarkForUndo();

      const Vertex2D v = m_editor->TransformPoint(x, y);

      vector<RenderVertex> vvertex;
      m_trigger->GetRgVertex(vvertex);

      int iSeg;
      Vertex2D vOut;
      ClosestPointOnPolygon(vvertex, v, vOut, iSeg, true);

      // Go through vertices (including iSeg itself) counting control points until iSeg
      int icp = 0;
      for (int i = 0; i < (iSeg + 1); i++)
         if (vvertex[i].controlPoint)
            icp++;

      //if (icp == 0) // need to add point after the last point
      //icp = m_trigger->m_vdpoint.size();

      CComObject<DragPoint>* pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(m_trigger, vOut.x, vOut.y, 0.f, false);
         m_trigger->m_vdpoint.insert(m_trigger->m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
      }

      m_trigger->EndUndo();
      if (m_trigger->GetPTable())
         m_trigger->GetPTable()->SetDirtyDraw();
   }
   break;
   }
}
