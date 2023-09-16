#include "stdafx.h"

#include "ReelDisplayReelBoxCollection.h"
#include "../controls/B2SReelBox.h"

ReelDisplayReelBoxCollection::ReelDisplayReelBoxCollection()
{
   m_led = false;
}

void ReelDisplayReelBoxCollection::Add(int key, B2SReelBox* pReelbox)
{
   string szReelType = pReelbox->GetReelType();
   std::transform(szReelType.begin(), szReelType.end(), szReelType.begin(), [](unsigned char c){ return std::tolower(c); });

   if (!m_led && (szReelType.compare(0, 3, "led", 3) == 0 || szReelType.compare(0, 11, "importedled", 11) == 0))
      m_led = true;

   (*this)[key] = pReelbox;
}
