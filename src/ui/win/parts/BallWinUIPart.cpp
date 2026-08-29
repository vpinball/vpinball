// license:GPLv3+

#include "core/stdafx.h"

#include "parts/ball.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/BallWinUIPart.h"


BallWinUIPart::BallWinUIPart(PinTableWnd* editor, Ball* ball)
   : m_editor(editor)
   , m_ball(ball)
{
}

void BallWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void BallWinUIPart::UIRenderPass2(Sur* const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(m_ball);
   psur->Ellipse(m_ball->m_hitBall.m_d.m_pos.x, m_ball->m_hitBall.m_d.m_pos.y, m_ball->m_hitBall.m_d.m_radius);
}

void BallWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetObject(m_ball);
   psur->Ellipse(m_ball->m_hitBall.m_d.m_pos.x, m_ball->m_hitBall.m_d.m_pos.y, m_ball->m_hitBall.m_d.m_radius);
}
