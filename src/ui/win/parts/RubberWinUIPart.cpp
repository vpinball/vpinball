// license:GPLv3+

#include "core/stdafx.h"

#include "parts/rubber.h"
#include "ui/win/DragPointDialogs.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/RubberWinUIPart.h"

RubberWinUIPart::RubberWinUIPart(PinTableWnd* editor, Rubber* rubber)
   : IWinUIPart(editor, rubber)
   , m_rubber(rubber)
   , m_pointParts(editor, rubber)
{
}

void RubberWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(0.f, 0.f);
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
      vector<Vertex2D> outline;
      m_rubber->GetEditorOutline(outline, nullptr, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));
      if (!outline.empty())
         psur->Polygon(outline.data(), (int)outline.size());
   }
   else
   {
      vector<Vertex2D> edges;
      m_rubber->GetEditorWireframe(edges);
      if (!edges.empty())
         psur->Lines(edges.data(), (int)(edges.size() / 2));
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
      vector<Vertex2D> outline;
      vector<bool> crossFlags;
      m_rubber->GetEditorOutline(outline, &crossFlags, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));

      if (!outline.empty())
         psur->Polygon(outline.data(), (int)outline.size());
      for (size_t i = 0; i < crossFlags.size(); i++)
         if (crossFlags[i])
            psur->Line(outline[i].x, outline[i].y, outline[outline.size() - i - 1].x, outline[outline.size() - i - 1].y);
   }
   else
   {
      vector<Vertex2D> edges;
      m_rubber->GetEditorWireframe(edges);
      if (!edges.empty())
         psur->Lines(edges.data(), (int)(edges.size() / 2));

      // if rotation is used don't show dragpoints
      return;
   }

   bool drawDragpoints = ((m_selectstate != SelectState::NotSelected) || (m_editor->m_vpxEditor->m_alwaysDrawDragPoints));

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_rubber->m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint>* const pdp = m_rubber->m_vdpoint[i];
         if (m_pointParts.IsSelected(pdp))
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
         psur->SetBorderColor(m_pointParts.IsDragging(pdp) ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
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
      vector<Vertex2D> outline;
      vector<bool> crossFlags;
      m_rubber->GetEditorOutline(outline, &crossFlags, 4.0f * powf(10.0f, (10.0f - HIT_SHAPE_DETAIL_LEVEL) * (float)(1.0 / 1.5)));

      if (!outline.empty())
         psur->Polygon(outline.data(), (int)outline.size());
      for (size_t i = 0; i < crossFlags.size(); i++)
         if (crossFlags[i])
            psur->Line(outline[i].x, outline[i].y, outline[outline.size() - i - 1].x, outline[outline.size() - i - 1].y);
   }
}

void RubberWinUIPart::DoCommand(int icmd, int x, int y)
{
   IWinUIPart::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP: m_rubber->FlipPointY(m_rubber->GetPointCenter()); break;

   case ID_WALLMENU_MIRROR: m_rubber->FlipPointX(m_rubber->GetPointCenter()); break;

   case ID_WALLMENU_ROTATE: (void)VPX::WinUI::RotatePointsDialog(m_rubber); break;

   case ID_WALLMENU_SCALE: (void)VPX::WinUI::ScalePointsDialog(m_rubber); break;

   case ID_WALLMENU_TRANSLATE: (void)VPX::WinUI::TranslatePointsDialog(m_rubber); break;

   case ID_WALLMENU_ADDPOINT:
   {
      m_rubber->AddPoint(m_editor->TransformPoint(x, y), true);
   }
   break;
   }
}
