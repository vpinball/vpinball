#include "../common.h"

#include "../classes/AnimationInfo.h"
#include "AnimationCollection.h"

namespace B2SLegacy {

void AnimationCollection::Add(int key, AnimationInfo* pAnimationInfo)
{
   (*this)[key].push_back(pAnimationInfo);
}

}
