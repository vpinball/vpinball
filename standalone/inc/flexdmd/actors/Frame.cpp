#include "stdafx.h"

#include "Frame.h"

Frame::Frame(FlexDMD* pFlexDMD, const string& name) : Actor(pFlexDMD, name)
{
   m_thickness = 2;
   m_borderColor = RGB(255, 255, 255);
   m_fill = false;
   m_fillColor = RGB(0, 0, 0);
}

Frame::~Frame()
{
}

STDMETHODIMP Frame::get_Thickness(LONG *pRetVal)
{
   *pRetVal = m_thickness;

   return S_OK;
}

STDMETHODIMP Frame::put_Thickness(LONG pRetVal)
{
   m_thickness = pRetVal;

   return S_OK;
}

STDMETHODIMP Frame::get_BorderColor(OLE_COLOR *pRetVal)
{
   *pRetVal = m_borderColor;

   return S_OK;
}

STDMETHODIMP Frame::put_BorderColor(OLE_COLOR pRetVal)
{
   m_borderColor = pRetVal;

   return S_OK;
}

STDMETHODIMP Frame::get_Fill(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_fill ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Frame::put_Fill(VARIANT_BOOL pRetVal)
{
   m_fill = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Frame::get_FillColor(OLE_COLOR *pRetVal)
{
   *pRetVal = m_fillColor;

   return S_OK;
}

STDMETHODIMP Frame::put_FillColor(OLE_COLOR pRetVal)
{
   m_fillColor = pRetVal;

   return S_OK;
}

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