// license:GPLv3+

#include "core/stdafx.h"

#include "parts/light.h"
#include "ui/win/parts/LightCenterWinUIPart.h"

LightCenterWinUIPart::LightCenterWinUIPart(PinTableWnd* editor, LightCenter* lightCenter)
   : IWinUIPart(editor, lightCenter->GetIEditable())
{
}

Vertex2D LightCenterWinUIPart::GetCenter() const { return static_cast<Light*>(GetEditable())->m_d.m_vCenter; }

void LightCenterWinUIPart::Translate(const Vertex2D& offset)
{
   Light* const light = static_cast<Light*>(GetEditable());
   light->m_d.m_vCenter.x += offset.x;
   light->m_d.m_vCenter.y += offset.y;
}
