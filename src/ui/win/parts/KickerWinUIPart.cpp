// license:GPLv3+

#include "core/stdafx.h"

#include "parts/kicker.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/KickerWinUIPart.h"


KickerWinUIPart::KickerWinUIPart(PinTableWnd* editor, Kicker* kicker)
   : m_editor(editor)
   , m_kicker(kicker)
{
}

void KickerWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void KickerWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_kicker);

   // Draw Arrow to display (default) orientation
   {
       const float radangle = ANGTORAD(m_kicker->m_d.m_orientation);
       constexpr float halflength = 50.0f;

       psur->SetLineColor(RGB(255, 0, 0), false, 1);

       Vertex2D tmp;
       {
           const float sn = sinf(radangle);
           const float cs = cosf(radangle);

           constexpr float len1 = halflength * 0.5f;
           tmp.x = m_kicker->m_d.m_vCenter.x + sn * len1;
           tmp.y = m_kicker->m_d.m_vCenter.y - cs * len1;
       }

       psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y);
       constexpr float len2 = halflength * 0.25f;
       {
           const float arrowang = radangle + 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x + sn * len2, m_kicker->m_d.m_vCenter.y - cs * len2);
       }
       {
           const float arrowang = radangle - 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x + sn * len2, m_kicker->m_d.m_vCenter.y - cs * len2);
       }
   }

   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.75f);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.5f);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.25f);
}

void KickerWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetObject(m_kicker);

   // Draw Arrow to display (default) orientation
   {
       const float radangle = ANGTORAD(m_kicker->m_d.m_orientation);
       constexpr float halflength = 50.0f;

       psur->SetLineColor(RGB(255, 0, 0), false, 1);

       Vertex2D tmp;
       {
           const float sn = sinf(radangle);
           const float cs = cosf(radangle);

           constexpr float len1 = halflength * 0.5f;
           tmp.x = m_kicker->m_d.m_vCenter.x + sn * len1;
           tmp.y = m_kicker->m_d.m_vCenter.y - cs * len1;
       }

       psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y);
       constexpr float len2 = halflength * 0.25f;
       {
           const float arrowang = radangle + 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x + sn * len2, m_kicker->m_d.m_vCenter.y - cs * len2);
       }
       {
           const float arrowang = radangle - 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_kicker->m_d.m_vCenter.x + sn * len2, m_kicker->m_d.m_vCenter.y - cs * len2);
       }
   }

   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.75f);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.5f);
   psur->Ellipse(m_kicker->m_d.m_vCenter.x, m_kicker->m_d.m_vCenter.y, m_kicker->m_d.m_radius*0.25f);
}
