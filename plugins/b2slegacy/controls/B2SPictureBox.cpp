#include "../common.h"

#include "B2SPictureBox.h"

namespace B2SLegacy {

B2SPictureBox::B2SPictureBox(VPXPluginAPI* vpxApi, B2SData* pB2SData) : B2SBaseBox(vpxApi, pB2SData)
{
   m_pictureBoxType = ePictureBoxType_StandardImage;
   m_szGroupName.clear();
   m_intensity = 1;
   m_initialState = 0;
   m_dualMode = eDualMode_Both;
   m_zorder = 0;
   m_imageSnippit = false;
   m_snippitRotationStopBehaviour = eSnippitRotationStopBehaviour_SpinOff;
   m_setThruAnimation = false;
   m_pOffImage = nullptr;

   SetVisible(false);
}

B2SPictureBox::~B2SPictureBox()
{
   if (m_pOffImage)
      m_vpxApi->DeleteTexture(m_pOffImage);
}

}
