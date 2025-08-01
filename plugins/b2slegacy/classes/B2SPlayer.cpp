#include "../common.h"

#include "B2SPlayer.h"
#include "../collections/ControlCollection.h"

namespace B2SLegacy {

void B2SPlayer::Add(int playerno)
{
   (*this)[playerno] = new ControlCollection();
}

}
