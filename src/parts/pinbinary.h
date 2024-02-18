// PinBinary.h: interface for the PinBinary class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINBINARY_H__080F5CB3_4174_4B1D_A81E_DCC4CBFE5954__INCLUDED_)
#define AFX_PINBINARY_H__080F5CB3_4174_4B1D_A81E_DCC4CBFE5954__INCLUDED_

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

#endif // !defined(AFX_PINBINARY_H__080F5CB3_4174_4B1D_A81E_DCC4CBFE5954__INCLUDED_)
