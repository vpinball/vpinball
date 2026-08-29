// license:GPLv3+

#include "core/stdafx.h"

#include "parts/textbox.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/TextboxWinUIPart.h"

TextboxWinUIPart::TextboxWinUIPart(PinTableWnd* editor, Textbox* textbox)
   : m_editor(editor)
   , m_textbox(textbox)
{
}

void TextboxWinUIPart::UIRenderPass1(Sur* const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_textbox->m_d.m_backcolor);
   psur->SetObject(m_textbox);

   psur->Rectangle(m_textbox->m_d.m_v1.x, m_textbox->m_d.m_v1.y, m_textbox->m_d.m_v2.x, m_textbox->m_d.m_v2.y);
}

void TextboxWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_textbox);
   psur->SetObject(nullptr);

   psur->Rectangle(m_textbox->m_d.m_v1.x, m_textbox->m_d.m_v1.y, m_textbox->m_d.m_v2.x, m_textbox->m_d.m_v2.y);
}
