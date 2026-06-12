#include "../common.h"

#include "B2SBaseBox.h"

namespace B2SLegacy {

B2SBaseBox::B2SBaseBox(VPXPluginAPI* vpxApi, B2SData* pB2SData) :
   Control(vpxApi)
{
   m_pB2SData = pB2SData;
}

}
