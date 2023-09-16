#include "stdafx.h"

#include "B2SPictureBox.h"

B2SPictureBox::B2SPictureBox() : B2SBaseBox()
{
   m_pictureBoxType = ePictureBoxType_StandardImage;
   m_szGroupName = "";
   m_intensity = 1;
   m_initialState = 0;
   m_dualMode = eDualMode_Both;
   m_zorder = 0;
   m_imageSnippit = false;
   m_snippitRotationStopBehaviour = eSnippitRotationStopBehaviour_SpinOff;
   m_setThruAnimation = false;
   m_pBackgroundImage = NULL;
   m_pOffImage = NULL;

   SetVisible(false);
}

B2SPictureBox::~B2SPictureBox()
{
}

void B2SPictureBox::OnPaint(SDL_Surface* pSurface)
{
   if (IsVisible()) {
     //SDL_Rect rect = { GetLeft(), GetTop(), GetWidth() - 1, GetHeight() - 1 };
     //SDL_FillRect(pSurface, &rect, SDL_MapRGBA(pSurface->format, GetRValue(0xD3), GetGValue(0xD3), GetBValue(0xD3), 0xFF));
   }
}