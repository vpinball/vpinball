#include "stdafx.h"

#include "AnimationInfo.h"

AnimationInfo::AnimationInfo(const string& szName, bool inverted)
{
   m_szName = szName;
   m_inverted = inverted;
}