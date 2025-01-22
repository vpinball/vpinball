#include "Frame.h"

void Frame::Draw(VP::SurfaceGraphics* pGraphics)
{
   Actor::Draw(pGraphics);

   if (GetVisible()) {
      if (m_fill) {
         pGraphics->SetColor(m_fillColor);
         pGraphics->FillRectangle({ (int)(GetX() + m_thickness), (int)(GetY() + m_thickness), (int)(GetWidth() - 2.0f * m_thickness), (int)(GetHeight() - 2.0f * m_thickness) });
      }
      if (m_thickness > 0.0f) {
         pGraphics->SetColor(m_borderColor);
         pGraphics->FillRectangle({ (int)GetX(), (int)GetY(), (int)GetWidth(), (int)m_thickness });
         pGraphics->FillRectangle({ (int)GetX(), (int)(GetY() + GetHeight() - m_thickness), (int)GetWidth(), (int)m_thickness });
         pGraphics->FillRectangle({ (int)GetX(), (int)(GetY() + m_thickness), (int)m_thickness, (int)(GetHeight() - 2.0f * m_thickness) });
         pGraphics->FillRectangle({ (int)(GetX() + GetWidth() - m_thickness), (int)(GetY() + m_thickness), (int)m_thickness, (int)(GetHeight() - 2.0f * m_thickness) });
      }
   }
}