// license:GPLv3+

#include "core/stdafx.h"

#include "parts/spinner.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/SpinnerWinUIPart.h"

SpinnerWinUIPart::SpinnerWinUIPart(PinTableWnd* editor, Spinner* spinner)
   : m_editor(editor)
   , m_spinner(spinner)
{
}

void SpinnerWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void SpinnerWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 3);
   psur->SetObject(m_spinner);

   const float halflength = m_spinner->m_d.m_length * 0.5f;

   const float radangle = ANGTORAD(m_spinner->m_d.m_rotation);
   float sn = sinf(radangle);
   float cs = cosf(radangle);

   psur->Line(m_spinner->m_d.m_vCenter.x + cs * halflength, m_spinner->m_d.m_vCenter.y + sn * halflength,
      m_spinner->m_d.m_vCenter.x - cs * halflength, m_spinner->m_d.m_vCenter.y - sn * halflength);

   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(m_spinner);

   psur->Line(m_spinner->m_d.m_vCenter.x + cs * halflength, m_spinner->m_d.m_vCenter.y + sn * halflength,
      m_spinner->m_d.m_vCenter.x - cs * halflength, m_spinner->m_d.m_vCenter.y - sn * halflength);

   if (sn == 0.0f) sn = 1.0f;
   if (cs == 0.0f) cs = 1.0f;
   psur->Rectangle(m_spinner->m_d.m_vCenter.x - cs * halflength * 0.65f, m_spinner->m_d.m_vCenter.y - sn * halflength * 0.65f,
                   m_spinner->m_d.m_vCenter.x + cs * halflength * 0.65f, m_spinner->m_d.m_vCenter.y + sn * halflength * 0.65f);
}
