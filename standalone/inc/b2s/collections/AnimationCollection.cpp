#include "stdafx.h"

#include "../classes/AnimationInfo.h"
#include "AnimationCollection.h"

void AnimationCollection::Add(int key, AnimationInfo* pAnimationInfo)
{
   (*this)[key].push_back(pAnimationInfo);
}