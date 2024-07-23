#include "stdafx.h"

#include "AnimationInfo.h"

AnimationInfo::AnimationInfo(const string& szAnimationName, bool inverted)
{
   m_szAnimationName = szAnimationName;
   m_inverted = inverted;
}