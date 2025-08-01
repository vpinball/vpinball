#pragma once


namespace B2SLegacy {

class Sound
{
public:
   Sound(const vector<unsigned char>& stream);
   ~Sound();

   uint8_t* GetStream() { return m_pStream; }

private:
   uint8_t* m_pStream;
};

}
