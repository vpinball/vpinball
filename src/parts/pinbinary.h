// license:GPLv3+

// interface for the PinBinary class.

#pragma once

class PinBinary : public ILoadable
{
public:
   PinBinary();
   virtual ~PinBinary();

   bool ReadFromFile(const string& szfilename);
   bool WriteToFile(const string& szfilename);
   HRESULT SaveToStream(IStream *pstream);
   HRESULT LoadFromStream(IStream *pstream, int version);

   // ILoadable callback
   bool LoadToken(const int id, BiffReader * const pbr) override;

   string m_szName;
   string m_szPath;

   char *m_pdata; // Copy of the buffer data so we can save it out
   int m_cdata;
};

class PinFont final : public PinBinary
{
public:
   void Register();
   void UnRegister();

private:
   string m_szTempFile; // temporary file name for registering font
};
