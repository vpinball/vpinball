// license:GPLv3+

#include "core/stdafx.h"
#include "ViewportPane.h"

#include "ui/live/EditorUI.h"

namespace VPX::EditorUI
{

void ViewportPane::RenderPane(const AreaRect& rect, const int leafId)
{
   // Header-only for now: the 3D scene still renders behind the leaf rect
   m_editor.RenderAreaTypeButton(leafId, rect);
}

void ViewportPane::OnUnhosted() { }

}
