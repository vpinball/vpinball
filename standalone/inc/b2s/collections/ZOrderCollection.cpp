#include "stdafx.h"

#include "ZOrderCollection.h"
#include "../controls/B2SPictureBox.h"

void ZOrderCollection::Add(B2SPictureBox* pPicbox)
{
   if (pPicbox->GetZOrder() > 0)
      (*this)[pPicbox->GetZOrder()].push_back(pPicbox);
}
