#include "Frame.h"

namespace Flex {

void Frame::Draw(Flex::SurfaceGraphics* pGraphics)
{
   Actor::Draw(pGraphics);

   if (GetVisible()) {
      if (m_fill) {
         pGraphics->SetColor(m_fillColor);
         pGraphics->FillRectangle({ (int)(GetX() + (float)m_thickness), (int)(GetY() + (float)m_thickness), GetWidth() - 2 * m_thickness, GetHeight() - 2 * m_thickness });
      }
      if (m_thickness > 0) {
         pGraphics->SetColor(m_borderColor);
         pGraphics->FillRectangle({ (int)GetX(), (int)GetY(), GetWidth(), m_thickness });
         pGraphics->FillRectangle({ (int)GetX(), (int)(GetY() + (float)(GetHeight() - m_thickness)), GetWidth(), m_thickness });
         pGraphics->FillRectangle({ (int)GetX(), (int)(GetY() + (float)m_thickness), m_thickness, GetHeight() - 2 * m_thickness });
         pGraphics->FillRectangle({ (int)(GetX() + (float)(GetWidth() - m_thickness)), (int)(GetY() + (float)m_thickness), m_thickness, GetHeight() - 2 * m_thickness });
      }
   }
}

}
