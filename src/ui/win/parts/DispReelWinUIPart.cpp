// license:GPLv3+

#include "core/stdafx.h"

#include "parts/dispreel.h"
#include "ui/win/sur.h"
#include "ui/win/parts/DispReelWinUIPart.h"

DispReelWinUIPart::DispReelWinUIPart(PinTableWnd* editor, DispReel* dispreel)
   : m_editor(editor)
   , m_dispreel(dispreel)
{
}

void DispReelWinUIPart::UIRenderPass1(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_dispreel->m_d.m_backcolor);
   psur->SetObject(m_dispreel);

   // draw background box
   psur->Rectangle(m_dispreel->m_d.m_v1.x, m_dispreel->m_d.m_v1.y, m_dispreel->m_d.m_v2.x, m_dispreel->m_d.m_v2.y);

   // draw n reels in the box (in blue)
   psur->SetFillColor(RGB(0, 0, 255));
   for (int i = 0; i < m_dispreel->m_d.m_reelcount; ++i)
   {
      // set up top corner point
      const float fi = (float)i;
      const float x = m_dispreel->m_d.m_v1.x + fi*(m_dispreel->m_d.m_width + m_dispreel->m_d.m_reelspacing) + m_dispreel->m_d.m_reelspacing;
      const float y = m_dispreel->m_d.m_v1.y + m_dispreel->m_d.m_reelspacing;
      const float x2 = x + m_dispreel->m_d.m_width;
      const float y2 = y + m_dispreel->m_d.m_height;

      // set up points (clockwise)
      const Vertex2D rgv[4] = { Vertex2D(x, y), Vertex2D(x2, y), Vertex2D(x2, y2), Vertex2D(x, y2) };
      psur->Polygon(rgv, 4);
   }
}

void DispReelWinUIPart::UIRenderPass2(Sur * const psur)
{
   if (!m_dispreel->GetPTable()->GetEMReelsEnabled()) return;

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_dispreel);
   psur->SetObject(nullptr);

   // draw background box
   psur->Rectangle(m_dispreel->m_d.m_v1.x, m_dispreel->m_d.m_v1.y, m_dispreel->m_d.m_v2.x, m_dispreel->m_d.m_v2.y);

   // draw n reels in the box
   for (int i = 0; i < m_dispreel->m_d.m_reelcount; ++i)
   {
      // set up top corner point
      const float fi = (float)i;
      const float x = m_dispreel->m_d.m_v1.x + fi*(m_dispreel->m_d.m_width + m_dispreel->m_d.m_reelspacing) + m_dispreel->m_d.m_reelspacing;
      const float y = m_dispreel->m_d.m_v1.y + m_dispreel->m_d.m_reelspacing;
      const float x2 = x + m_dispreel->m_d.m_width;
      const float y2 = y + m_dispreel->m_d.m_height;

      // set up points (clockwise)
      const Vertex2D rgv[4] = { Vertex2D(x, y), Vertex2D(x2, y), Vertex2D(x2, y2), Vertex2D(x, y2) };
      psur->Polygon(rgv, 4);
   }
}
