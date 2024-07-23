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

   if (!m_led && (string_starts_with_case_insensitive(szReelType, "led") || string_starts_with_case_insensitive(szReelType, "importedled")))
      m_led = true;

   (*this)[key] = pReelbox;
}
