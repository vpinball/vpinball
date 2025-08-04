#pragma once

#include <map>


namespace B2SLegacy {

class AnimationInfo;

class AnimationCollection final : public std::map<int, vector<AnimationInfo*>>
{
public:
   void Add(int key, AnimationInfo* pAnimationInfo);
};

}
