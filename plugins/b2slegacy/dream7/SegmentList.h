#pragma once


#include <map>

#include "Segment.h"
#include "../utils/Matrix.h"

namespace B2SLegacy {

class SegmentList : public vector<Segment*>
{
public:
   void Transform(Matrix* pMatrix);
};

}
