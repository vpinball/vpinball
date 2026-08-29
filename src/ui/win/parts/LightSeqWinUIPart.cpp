// license:GPLv3+

#include "core/stdafx.h"

#include "parts/lightseq.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/LightSeqWinUIPart.h"

LightSeqWinUIPart::LightSeqWinUIPart(PinTableWnd* editor, LightSeq* lightseq)
   : m_editor(editor)
   , m_lightseq(lightseq)
{
}

void LightSeqWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_lightseq);

   for (int i = 0; i < 8; ++i)
   {
      psur->SetFillColor((i % 2 == 0) ? RGB(255, 0, 0) : RGB(128, 0, 0));
      const float angle = (float)((M_PI * 2.0) / 8.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_lightseq->m_d.m_v.x + sn * 12.0f, m_lightseq->m_d.m_v.y - cs * 12.0f, 4.0f);
   }

   psur->SetFillColor(RGB(255, 0, 0));
   psur->Ellipse(m_lightseq->m_d.m_v.x, m_lightseq->m_d.m_v.y - 3.0f, 4.0f);
}

void LightSeqWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(m_lightseq);

   psur->Ellipse(m_lightseq->m_d.m_v.x, m_lightseq->m_d.m_v.y, 18.0f);

   for (int i = 0; i < 8; ++i)
   {
      const float angle = (float)((M_PI * 2.0) / 8.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_lightseq->m_d.m_v.x + sn * 12.0f, m_lightseq->m_d.m_v.y - cs * 12.0f, 4.0f);
   }

   psur->Ellipse(m_lightseq->m_d.m_v.x, m_lightseq->m_d.m_v.y - 3.0f, 4.0f);

   RenderOutline(psur);
}

void LightSeqWinUIPart::RenderOutline(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject((ISelect*)m_lightseq);

   psur->Line(m_lightseq->m_d.m_vCenter.x - 10.0f, m_lightseq->m_d.m_vCenter.y, m_lightseq->m_d.m_vCenter.x + 10.0f, m_lightseq->m_d.m_vCenter.y);
   psur->Line(m_lightseq->m_d.m_vCenter.x, m_lightseq->m_d.m_vCenter.y - 10.0f, m_lightseq->m_d.m_vCenter.x, m_lightseq->m_d.m_vCenter.y + 10.0f);

   for (int i = 0; i < 8; ++i)
   {
      psur->SetFillColor((i % 2 == 0) ? RGB(255, 0, 0) : RGB(128, 0, 0));
      const float angle = (float)((M_PI * 2.0) / 8.0) * (float)i;
      const float sn = sinf(angle);
      const float cs = cosf(angle);
      psur->Ellipse(m_lightseq->m_d.m_vCenter.x + sn * 7.0f, m_lightseq->m_d.m_vCenter.y - cs * 7.0f, 2.0f);
   }

   psur->SetFillColor(RGB(255, 0, 0));
   psur->Ellipse(m_lightseq->m_d.m_vCenter.x, m_lightseq->m_d.m_vCenter.y - 2.5f, 2.0f);
}

void LightSeqWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
}
