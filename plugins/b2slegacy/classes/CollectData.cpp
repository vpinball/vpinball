#include "../common.h"

#include "CollectData.h"

namespace B2SLegacy {

CollectData::CollectData(const int state, const int types)
{
   m_state = state;
   m_types = types;
   m_earlyoffmode = false;
}

CollectData::CollectData(const int state, const int types, bool earlyoffmode)
{
   m_state = state;
   m_types = types;
   m_earlyoffmode = earlyoffmode;
}

}
