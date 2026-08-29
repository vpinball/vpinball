// license:GPLv3+

#include "core/stdafx.h"

#include "parts/decal.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/DecalWinUIPart.h"


DecalWinUIPart::DecalWinUIPart(PinTableWnd* editor, Decal* decal)
   : m_editor(editor)
   , m_decal(decal)
{
}

void DecalWinUIPart::UIRenderPass1(Sur* const psur)
{
   if (!(m_decal->m_desktopBackdrop && !m_decal->GetPTable()->GetDecalsEnabled()))
   {
      psur->SetBorderColor(-1, false, 0);
      psur->SetFillColor(m_decal->m_ptable->RenderSolid() ? RGB(0, 0, 255) : -1);
      psur->SetObject(m_decal);

      const float halfwidth = m_decal->m_realwidth * 0.5f;
      const float halfheight = m_decal->m_realheight * 0.5f;

      const float radangle = ANGTORAD(m_decal->m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      const Vertex2D rgv[4] = {
         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight - sn*halfwidth) };

      psur->Polygon(rgv, 4);
   }
}

void DecalWinUIPart::UIRenderPass2(Sur* const psur)
{
   if (!(m_decal->m_desktopBackdrop && !m_decal->GetPTable()->GetDecalsEnabled()))
   {
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->SetFillColor(-1);
      psur->SetObject(m_decal);
      psur->SetObject(nullptr);

      const float halfwidth = m_decal->m_realwidth * 0.5f;
      const float halfheight = m_decal->m_realheight * 0.5f;

      const float radangle = ANGTORAD(m_decal->m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      const Vertex2D rgv[4] = {
         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight - sn*halfwidth) };

      psur->Polygon(rgv, 4);
   }
}

void DecalWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   if (!(m_decal->m_desktopBackdrop && !m_decal->GetPTable()->GetDecalsEnabled()))
   {
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
      psur->SetObject(m_decal);
      psur->SetObject(nullptr);

      const float halfwidth = m_decal->m_realwidth * 0.5f;
      const float halfheight = m_decal->m_realheight * 0.5f;

      const float radangle = ANGTORAD(m_decal->m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      const Vertex2D rgv[4] = {
         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

         Vertex2D(m_decal->m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
         m_decal->m_d.m_vCenter.y + cs*halfheight - sn*halfwidth) };

      psur->Polygon(rgv, 4);
   }
}
