// license:GPLv3+

#include "core/stdafx.h"

#include "parts/plunger.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/PlungerWinUIPart.h"

PlungerWinUIPart::PlungerWinUIPart(PinTableWnd* editor, Plunger* plunger)
   : m_editor(editor)
   , m_plunger(plunger)
{
}

void PlungerWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void PlungerWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);

   // draw the park position, if appropriate
   if (m_plunger->m_d.m_parkPosition > 0.0f && m_plunger->m_d.m_parkPosition < 1.0f)
   {
      const float park = m_plunger->m_d.m_parkPosition * m_plunger->m_d.m_stroke;
      psur->SetLineColor(RGB(0, 180, 0), false, //
         (m_plunger->m_selectstate == ISelect::SelectState::Selected)           ? 4 //
            : (m_plunger->m_selectstate == ISelect::SelectState::MultiSelected) ? 3 //
            : m_plunger->IsUILocked()                                           ? 1 //
                                                                                : 2);
      psur->Line(m_plunger->m_d.m_v.x - m_plunger->m_d.m_width, m_plunger->m_d.m_v.y - m_plunger->m_d.m_stroke + park, m_plunger->m_d.m_v.x + m_plunger->m_d.m_width, m_plunger->m_d.m_v.y - m_plunger->m_d.m_stroke + park);
   }

   psur->SetObject(m_plunger);
   psur->Rectangle(m_plunger->m_d.m_v.x - m_plunger->m_d.m_width, m_plunger->m_d.m_v.y - m_plunger->m_d.m_stroke,
      m_plunger->m_d.m_v.x + m_plunger->m_d.m_width, m_plunger->m_d.m_v.y + m_plunger->m_d.m_height);
}
