#pragma once

class Sound
{
public:
   Sound(const vector<unsigned char>& stream);
   ~Sound();

   UINT8* GetStream() { return m_pStream; }

private:
   UINT8* m_pStream;
};