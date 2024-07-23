#include "stdafx.h"
#include "Kerning.h"

Kerning::Kerning()
{
}

Kerning::~Kerning()
{
}

int Kerning::GetHash()
{
   return (m_firstCharacter << 16) | m_secondCharacter;
}