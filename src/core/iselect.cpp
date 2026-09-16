// license:GPLv3+

#include "core/stdafx.h"
#include "iselect.h"

#include "core/editablereg.h"
#include "core/VPApp.h"
#include "parts/pintable.h"
#include "ui/win/WinEditor.h"

ISelect::ISelect()
   : m_vpinball(g_pvp)
{
}

void ISelect::SetDirtyDraw()
{
   if (PinTable* const table = GetIEditable()->GetPTable())
      table->SetDirtyDraw();
}

void ISelect::FlipY(const Vertex2D& pvCenter)
{
   const Vertex2D vCenter = GetCenter();
   Translate(Vertex2D(0.f, -2.f * (vCenter.y - pvCenter.y)));
}

void ISelect::FlipX(const Vertex2D& pvCenter)
{
   const Vertex2D vCenter = GetCenter();
   Translate(Vertex2D(-2.f * (vCenter.x - pvCenter.x), 0.f));
}

void ISelect::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   const Vertex2D vCenter = GetCenter();

   const float sn = sinf(ANGTORAD(ang));
   const float cs = cosf(ANGTORAD(ang));

   const float dx = vCenter.x - pvCenter.x;
   const float dy = vCenter.y - pvCenter.y;

   Translate(Vertex2D(pvCenter.x + cs * dx - sn * dy - vCenter.x, pvCenter.y + cs * dy + sn * dx - vCenter.y));
}

void ISelect::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   const Vertex2D vCenter = GetCenter();

   const float dx = vCenter.x - pvCenter.x;
   const float dy = vCenter.y - pvCenter.y;

   Translate(Vertex2D(pvCenter.x + dx * scalex - vCenter.x, pvCenter.y + dy * scaley - vCenter.y));
}
