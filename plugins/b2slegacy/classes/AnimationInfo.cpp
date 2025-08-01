#include "../common.h"
#include "AnimationInfo.h"

namespace B2SLegacy {

AnimationInfo::AnimationInfo(const string& szAnimationName, bool inverted)
{
   m_szAnimationName = szAnimationName;
   m_inverted = inverted;
}

}
