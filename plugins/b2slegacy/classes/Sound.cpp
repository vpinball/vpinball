#include "../common.h"

#include "Sound.h"

namespace B2SLegacy {

Sound::Sound(const string& stream)
{
   m_pStream = new uint8_t[stream.size()];
   memcpy(m_pStream, stream.data(), stream.size());
}

Sound::~Sound()
{
   delete [] m_pStream;
}

}
