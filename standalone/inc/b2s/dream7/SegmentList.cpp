#include "stdafx.h"

#include "SegmentList.h"

void SegmentList::Transform(VP::Matrix* pMatrix)
{
   for (auto& pSegment : *this)
      pSegment->Transform(pMatrix);
}
