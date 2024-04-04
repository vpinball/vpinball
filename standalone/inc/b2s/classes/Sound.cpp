#include "stdafx.h"

#include "Sound.h"

Sound::Sound(const vector<unsigned char>& stream)
{
   m_pStream = (UINT8*)malloc(stream.size());
   memcpy(m_pStream, &stream[0], stream.size());
}

Sound::~Sound()
{
   if (m_pStream)
      free(m_pStream);
}