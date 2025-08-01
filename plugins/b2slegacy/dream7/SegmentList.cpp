#include "../common.h"

#include "SegmentList.h"

namespace B2SLegacy {

void SegmentList::Transform(Matrix* pMatrix)
{
   for (auto& pSegment : *this)
      pSegment->Transform(pMatrix);
}

}
