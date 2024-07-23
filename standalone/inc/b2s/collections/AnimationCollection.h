#pragma once

#include <map>

class AnimationInfo;

class AnimationCollection : public std::map<int, vector<AnimationInfo*>>
{
public:
   void Add(int key, AnimationInfo* pAnimationInfo);
};