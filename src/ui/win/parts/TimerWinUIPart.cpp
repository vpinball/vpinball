// license:GPLv3+

#include "core/stdafx.h"

#include "parts/timer.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TimerWinUIPart.h"

TimerWinUIPart::TimerWinUIPart(PinTableWnd* editor, Timer* timer)
   : m_editor(editor)
   , m_timer(timer)
{
}

void TimerWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void TimerWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(m_timer);

   psur->Ellipse(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, 18);
   psur->Ellipse(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, 15);

   for (int i = 0; i < 12; i++)
   {
      const float angle = (float)(M_PI * 2.0 / 12.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Line(m_timer->m_d.m_v.x + sn * 9.0f, m_timer->m_d.m_v.y - cs * 9.0f, m_timer->m_d.m_v.x + sn * 15.0f, m_timer->m_d.m_v.y - cs * 15.0f);
   }

   psur->Line(m_timer->m_d.m_v.x, m_timer->m_d.m_v.y, m_timer->m_d.m_v.x + 10.5f, m_timer->m_d.m_v.y - 7.5f);
}

void TimerWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
}
