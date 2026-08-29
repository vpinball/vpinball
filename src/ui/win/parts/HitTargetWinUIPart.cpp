// license:GPLv3+

#include "core/stdafx.h"

#include "parts/hittarget.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/HitTargetWinUIPart.h"

HitTargetWinUIPart::HitTargetWinUIPart(PinTableWnd* editor, HitTarget* hittarget)
   : m_editor(editor)
   , m_hittarget(hittarget)
{
}

void HitTargetWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void HitTargetWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(m_hittarget);

   for (unsigned i = 0; i < m_hittarget->m_numIndices; i += 3)
   {
      const Vertex3Ds* const A = &m_hittarget->m_hitUIVertices[m_hittarget->m_indices[i]];
      const Vertex3Ds* const B = &m_hittarget->m_hitUIVertices[m_hittarget->m_indices[i + 1]];
      const Vertex3Ds* const C = &m_hittarget->m_hitUIVertices[m_hittarget->m_indices[i + 2]];
      psur->Line(A->x, A->y, B->x, B->y);
      psur->Line(B->x, B->y, C->x, C->y);
      psur->Line(C->x, C->y, A->x, A->y);
   }

   if (m_hittarget->m_selectstate == ISelect::SelectState::NotSelected)
      return;

   const float radangle = ANGTORAD(m_hittarget->m_d.m_rotZ - 180.0f);
   constexpr float halflength = 50.0f;
   constexpr float len1 = halflength * 0.5f;
   constexpr float len2 = len1 * 0.5f;
   {
      Vertex2D tmp;

      // Draw Arrow
      psur->SetLineColor(RGB(255, 0, 0), false, 1);

      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         tmp.x = m_hittarget->m_d.m_vPosition.x + sn * len1;
         tmp.y = m_hittarget->m_d.m_vPosition.y - cs * len1;
      }

      psur->Line(tmp.x, tmp.y, m_hittarget->m_d.m_vPosition.x, m_hittarget->m_d.m_vPosition.y);
      {
         const float arrowang = radangle + 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         psur->Line(tmp.x, tmp.y, m_hittarget->m_d.m_vPosition.x + sn * len2, m_hittarget->m_d.m_vPosition.y - cs * len2);
      }
      {
         const float arrowang = ANGTORAD(m_hittarget->m_d.m_rotZ - 180.0f) - 0.6f;
         const float sn = sinf(arrowang);
         const float cs = cosf(arrowang);

         psur->Line(tmp.x, tmp.y,
            m_hittarget->m_d.m_vPosition.x + sn * len2, m_hittarget->m_d.m_vPosition.y - cs * len2);
      }
   }
}
