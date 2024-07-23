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
   m_pOffImage = NULL;

   SetVisible(false);
}