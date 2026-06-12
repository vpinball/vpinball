#include "../common.h"

#include "B2SPictureBox.h"

namespace B2SLegacy {

B2SPictureBox::B2SPictureBox(VPXPluginAPI* vpxApi, B2SData* pB2SData) : B2SBaseBox(vpxApi, pB2SData)
{
   SetVisible(false);
}

B2SPictureBox::~B2SPictureBox()
{
   if (m_pOffImage)
      m_vpxApi->DeleteTexture(m_pOffImage);
}

}
