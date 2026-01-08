#pragma once


namespace B2SLegacy {

class Sound final
{
public:
   Sound(vector<uint8_t>&& stream) : m_pStream(std::move(stream)) {}
   ~Sound() {}

   const uint8_t* GetStream() const { return m_pStream.data(); }

private:
   vector<uint8_t> m_pStream;
};

}
