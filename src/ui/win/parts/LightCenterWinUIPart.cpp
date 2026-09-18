// license:GPLv3+

#include "core/stdafx.h"

#include "parts/light.h"
#include "ui/win/parts/LightCenterWinUIPart.h"

LightCenterWinUIPart::LightCenterWinUIPart(PinTableWnd* editor, Light* light)
   : IWinUIPart(editor, light)
   , m_light(light)
{
}

Vertex2D LightCenterWinUIPart::GetCenter() const { return m_light->m_d.m_vCenter; }

void LightCenterWinUIPart::Translate(const Vertex2D& offset)
{
   m_light->m_d.m_vCenter.x += offset.x;
   m_light->m_d.m_vCenter.y += offset.y;
}
