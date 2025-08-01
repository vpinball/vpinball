#include "../common.h"

#include "Sound.h"

namespace B2SLegacy {

Sound::Sound(const vector<unsigned char>& stream)
{
   m_pStream = (uint8_t*)malloc(stream.size());
   memcpy(m_pStream, stream.data(), stream.size());
}

Sound::~Sound()
{
   if (m_pStream)
      free(m_pStream);
}

}
