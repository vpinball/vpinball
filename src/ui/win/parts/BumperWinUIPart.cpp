// license:GPLv3+

#include "core/stdafx.h"

#include "parts/bumper.h"
#include "parts/Material.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/BumperWinUIPart.h"


BumperWinUIPart::BumperWinUIPart(PinTableWnd* editor, Bumper* bumper)
   : m_editor(editor)
   , m_bumper(bumper)
{
}

void BumperWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetBorderColor(-1, false, 0);

   psur->SetObject(m_bumper);
   const float radangle = ANGTORAD(m_bumper->m_d.m_orientation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_bumper->m_d.m_vCenter.x - cs * (m_bumper->m_d.m_radius + 10.f);
   const float y1 = m_bumper->m_d.m_vCenter.y - sn * (m_bumper->m_d.m_radius + 10.f);
   const float x2 = m_bumper->m_d.m_vCenter.x + cs * (m_bumper->m_d.m_radius + 10.f);
   const float y2 = m_bumper->m_d.m_vCenter.y + sn * (m_bumper->m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);

   if (m_bumper->m_ptable->m_renderSolid)
   {
      const Material* const mat = m_bumper->m_ptable->GetMaterial(m_bumper->m_d.m_szCapMaterial);
      psur->SetFillColor(mat->m_cBase);
   }
   else
      psur->SetFillColor(-1);

   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius * 1.5f);
   if (m_bumper->m_ptable->m_renderSolid)
   {
      const Material* const mat = m_bumper->m_ptable->GetMaterial(m_bumper->m_d.m_szBaseMaterial);
      psur->SetFillColor(mat->m_cBase);
   }
   else
      psur->SetFillColor(-1);

   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius);
}

void BumperWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_bumper);
   psur->SetObject(nullptr);
   const float radangle = ANGTORAD(m_bumper->m_d.m_orientation - 90.f);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_bumper->m_d.m_vCenter.x - cs * (m_bumper->m_d.m_radius + 10.f);
   const float y1 = m_bumper->m_d.m_vCenter.y - sn * (m_bumper->m_d.m_radius + 10.f);
   const float x2 = m_bumper->m_d.m_vCenter.x + cs * (m_bumper->m_d.m_radius + 10.f);
   const float y2 = m_bumper->m_d.m_vCenter.y + sn * (m_bumper->m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);
   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius * 1.5f);
   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius);

   if (m_editor->m_vpxEditor->m_alwaysDrawLightCenters)
   {
      psur->Line(m_bumper->m_d.m_vCenter.x - 10.0f, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_vCenter.x + 10.0f, m_bumper->m_d.m_vCenter.y);
      psur->Line(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y - 10.0f, m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y + 10.0f);
   }
}

void BumperWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(m_bumper);
   psur->SetObject(nullptr);
   const float radangle = ANGTORAD(m_bumper->m_d.m_orientation - 90.f);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_bumper->m_d.m_vCenter.x - cs * (m_bumper->m_d.m_radius + 10.f);
   const float y1 = m_bumper->m_d.m_vCenter.y - sn * (m_bumper->m_d.m_radius + 10.f);
   const float x2 = m_bumper->m_d.m_vCenter.x + cs * (m_bumper->m_d.m_radius + 10.f);
   const float y2 = m_bumper->m_d.m_vCenter.y + sn * (m_bumper->m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);
   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius * 1.5f);
   psur->Ellipse(m_bumper->m_d.m_vCenter.x, m_bumper->m_d.m_vCenter.y, m_bumper->m_d.m_radius);
}
