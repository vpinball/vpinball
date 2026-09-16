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
