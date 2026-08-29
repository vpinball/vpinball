// license:GPLv3+

#include "core/stdafx.h"

#include "parts/PartGroup.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/PartGroupWinUIPart.h"

PartGroupWinUIPart::PartGroupWinUIPart(PinTableWnd* editor, PartGroup* partgroup)
   : m_editor(editor)
   , m_partgroup(partgroup)
{
}

void PartGroupWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void PartGroupWinUIPart::UIRenderPass2(Sur* const psur)
{
}

void PartGroupWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
}
