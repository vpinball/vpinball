// license:GPLv3+

// interface for the PinBinary class.

#pragma once

class PinBinary
{
public:
   PinBinary() = default;
   virtual ~PinBinary() = default;

   bool ReadFromFile(const std::filesystem::path& filename);
   bool WriteToFile(const string& filename);
   HRESULT SaveToStream(IStream *pstream);
   HRESULT LoadFromStream(IStream *pstream, int version);

   string m_name;
   std::filesystem::path m_path;
   vector<uint8_t> m_buffer;
};

class PinFont final : public PinBinary
{
public:
   void Register();
   void UnRegister();

private:
   string m_szTempFile; // temporary file name for registering font
};
