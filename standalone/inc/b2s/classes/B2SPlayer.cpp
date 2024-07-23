#include "stdafx.h"

#include "B2SPlayer.h"
#include "../collections/ControlCollection.h"

void B2SPlayer::Add(int playerno)
{
   (*this)[playerno] = new ControlCollection();
}