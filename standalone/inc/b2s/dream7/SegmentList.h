#pragma once

#include "../b2s_i.h"

#include <map>

#include "Segment.h"
#include "../../common/Matrix.h"

class SegmentList : public vector<Segment*>
{
public:
   void Transform(VP::Matrix* pMatrix);
};