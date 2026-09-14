// license:GPLv3+

#include "core/stdafx.h"

#include "parts/hittarget.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/HitTargetWinUIPart.h"

HitTargetWinUIPart::HitTargetWinUIPart(PinTableWnd* editor, HitTarget* hittarget)
   : IWinUIPart(editor, hittarget)
   , m_hittarget(hittarget)
{
}

void HitTargetWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(m_hittarget->m_d.m_vPosition.x, m_hittarget->m_d.m_vPosition.y);
}

void HitTargetWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void HitTargetWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(m_hittarget);

   vector<Vertex2D> edges;
   m_hittarget->GetEditorWireframe(edges);
   if (!edges.empty())
      psur->Lines(edges.data(), (int)(edges.size() / 2));

   if (m_selectstate == SelectState::NotSelected)
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
