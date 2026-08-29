// license:GPLv3+

#include "core/stdafx.h"

#include "parts/gate.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/GateWinUIPart.h"

GateWinUIPart::GateWinUIPart(PinTableWnd* editor, Gate* gate)
   : m_editor(editor)
   , m_gate(gate)
{
}

void GateWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void GateWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 2);
   psur->SetObject(m_gate);

   const float halflength = m_gate->m_d.m_length * 0.5f;
   const float len1 = halflength * 0.5f;
   const float len2 = len1 * 0.5f;
   Vertex2D tmp;

   psur->Ellipse(m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y, halflength);

   {
      const float radangle = ANGTORAD(m_gate->m_d.m_rotation);
      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         psur->Line(m_gate->m_d.m_vCenter.x + cs * halflength, m_gate->m_d.m_vCenter.y + sn * halflength,
            m_gate->m_d.m_vCenter.x - cs * halflength, m_gate->m_d.m_vCenter.y - sn * halflength);

         // Draw Arrow
         psur->SetLineColor(RGB(0, 0, 0), false, 1);

         tmp.x = m_gate->m_d.m_vCenter.x + sn * len1;
         tmp.y = m_gate->m_d.m_vCenter.y - cs * len1;

         psur->Line(tmp.x, tmp.y,
            m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y);
      }

      {
         const float arrowang = radangle + 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         psur->Line(tmp.x, tmp.y,
            m_gate->m_d.m_vCenter.x + sn * len2, m_gate->m_d.m_vCenter.y - cs * len2);
      }
   }

   {
      const float arrowang = ANGTORAD(m_gate->m_d.m_rotation) - 0.6f;
      const float sn = sinf(arrowang);
      const float cs = cosf(arrowang);

      psur->Line(tmp.x, tmp.y,
         m_gate->m_d.m_vCenter.x + sn * len2, m_gate->m_d.m_vCenter.y - cs * len2);
   }

   if (m_gate->m_d.m_twoWay)
   {
      const float radangle = ANGTORAD(m_gate->m_d.m_rotation - 180.f);
      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         // Draw Arrow
         psur->SetLineColor(RGB(0, 0, 0), false, 1);

         tmp.x = m_gate->m_d.m_vCenter.x + sn * len1;
         tmp.y = m_gate->m_d.m_vCenter.y - cs * len1;

         psur->Line(tmp.x, tmp.y,
            m_gate->m_d.m_vCenter.x, m_gate->m_d.m_vCenter.y);
      }

      {
         const float arrowang = radangle + 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         psur->Line(tmp.x, tmp.y,
            m_gate->m_d.m_vCenter.x + sn * len2, m_gate->m_d.m_vCenter.y - cs * len2);
      }

      const float arrowang = radangle - 0.6f;
      const float sn = sinf(arrowang);
      const float cs = cosf(arrowang);

      psur->Line(tmp.x, tmp.y,
         m_gate->m_d.m_vCenter.x + sn * len2, m_gate->m_d.m_vCenter.y - cs * len2);
   }
}

void GateWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
}
