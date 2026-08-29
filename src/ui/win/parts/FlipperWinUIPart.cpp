// license:GPLv3+

#include "core/stdafx.h"

#include "parts/flipper.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/FlipperWinUIPart.h"

FlipperWinUIPart::FlipperWinUIPart(PinTableWnd* editor, Flipper* flipper)
   : m_editor(editor)
   , m_flipper(flipper)
{
}

void FlipperWinUIPart::UIRenderPass1(Sur* const psur)
{
   const float rubBaseRadius = m_flipper->m_d.m_BaseRadius - m_flipper->m_d.m_rubberthickness;
   const float rubEndRadius = m_flipper->m_d.m_EndRadius - m_flipper->m_d.m_rubberthickness;
   const float anglerad = ANGTORAD(m_flipper->m_d.m_StartAngle);

   m_flipper->m_d.m_FlipperRadius = m_flipper->m_d.m_FlipperRadiusMax;

   psur->SetFillColor(m_flipper->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

   psur->SetObject(m_flipper);

   psur->Polygon(rgv, 4);
   psur->Ellipse(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius);
   psur->Ellipse(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius);

   // rubber
   m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, rubBaseRadius, rubEndRadius);

   psur->SetObject(m_flipper);
   psur->SetLineColor(RGB(128, 0, 0), false, 0);

   psur->Polygon(rgv, 4);
   psur->Ellipse(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, rubBaseRadius);
   psur->Ellipse(vendcenter.x, vendcenter.y, rubEndRadius);
}

void FlipperWinUIPart::UIRenderPass2(Sur* const psur)
{
   const float anglerad = ANGTORAD(m_flipper->m_d.m_StartAngle);
   const float anglerad2 = ANGTORAD(m_flipper->m_d.m_EndAngle);
   const float rubBaseRadius = m_flipper->m_d.m_BaseRadius - m_flipper->m_d.m_rubberthickness;
   const float rubEndRadius = m_flipper->m_d.m_EndRadius - m_flipper->m_d.m_rubberthickness;

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

   psur->SetFillColor(m_flipper->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(m_flipper);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   // rubber
   m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, rubBaseRadius, rubEndRadius);

   psur->SetFillColor(m_flipper->m_ptable->RenderSolid() ? m_editor->m_vpxEditor->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(m_flipper);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, rubBaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, rubEndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   // draw the flipper up position
   m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

   psur->SetLineColor(RGB(128, 128, 128), true, 0);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   rgv[0].x = m_flipper->m_d.m_Center.x + sinf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
   rgv[0].y = m_flipper->m_d.m_Center.y - cosf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

   rgv[1].x = m_flipper->m_d.m_Center.x + sinf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
   rgv[1].y = m_flipper->m_d.m_Center.y - cosf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

   if (m_flipper->m_d.m_EndAngle < m_flipper->m_d.m_StartAngle)
      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   else
      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

   if (m_flipper->m_d.m_FlipperRadiusMin > 0.f && m_flipper->m_d.m_FlipperRadiusMax > m_flipper->m_d.m_FlipperRadiusMin)
   {
      m_flipper->m_d.m_FlipperRadius = (m_flipper->m_ptable->m_globalDifficulty > 0.f) ? m_flipper->m_d.m_FlipperRadiusMin : m_flipper->m_d.m_FlipperRadiusMax;
      m_flipper->m_d.m_FlipperRadius = max(m_flipper->m_d.m_FlipperRadius, m_flipper->m_d.m_BaseRadius - m_flipper->m_d.m_EndRadius + 0.05f);
   }
   else
      return;

   if (m_flipper->m_d.m_FlipperRadius != m_flipper->m_d.m_FlipperRadiusMax)
   {
      m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

      psur->SetObject(m_flipper);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

      psur->SetLineColor(RGB(128, 128, 128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_flipper->m_d.m_Center.x + sinf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
      rgv[0].y = m_flipper->m_d.m_Center.y - cosf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

      rgv[1].x = m_flipper->m_d.m_Center.x + sinf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
      rgv[1].y = m_flipper->m_d.m_Center.y - cosf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

      if (m_flipper->m_d.m_EndAngle < m_flipper->m_d.m_StartAngle)
         psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else
         psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

      m_flipper->m_d.m_FlipperRadius = m_flipper->m_d.m_FlipperRadiusMax - (m_flipper->m_d.m_FlipperRadiusMax - m_flipper->m_d.m_FlipperRadiusMin) * m_flipper->m_ptable->m_globalDifficulty;
      m_flipper->m_d.m_FlipperRadius = max(m_flipper->m_d.m_FlipperRadius, m_flipper->m_d.m_BaseRadius - m_flipper->m_d.m_EndRadius + 0.05f);

      m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

      psur->SetObject(m_flipper);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      m_flipper->SetVertices(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_flipper->m_d.m_BaseRadius, m_flipper->m_d.m_EndRadius);

      psur->SetLineColor(RGB(128, 128, 128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_flipper->m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_flipper->m_d.m_Center.x + sinf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
      rgv[0].y = m_flipper->m_d.m_Center.y - cosf(anglerad) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

      rgv[1].x = m_flipper->m_d.m_Center.x + sinf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);
      rgv[1].y = m_flipper->m_d.m_Center.y - cosf(anglerad2) * (m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius);

      if (m_flipper->m_d.m_EndAngle < m_flipper->m_d.m_StartAngle)
         psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else
         psur->Arc(m_flipper->m_d.m_Center.x, m_flipper->m_d.m_Center.y, m_flipper->m_d.m_FlipperRadius + m_flipper->m_d.m_EndRadius, rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);
   }

   m_flipper->m_d.m_FlipperRadius = m_flipper->m_d.m_FlipperRadiusMax;
}
