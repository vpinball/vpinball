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

   bool ReadFromFile(char *szfilename);
   bool WriteToFile(char *szfilename);
   HRESULT SaveToStream(IStream *pstream);
   HRESULT LoadFromStream(IStream *pstream, int version);

   // ILoadable callback
   virtual BOOL LoadToken(int id, BiffReader *pbr);

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

   char *m_pdata; // Copy of the buffer data so we can save it out
   int m_cdata;
};

class PinFont : public PinBinary
{
public:
   char m_szTempFile[MAX_PATH]; // temporary file name for registering font
   char m_szFontName[LF_FACESIZE]; // Name of the font, as would be seen on a system font list

   void Register();
   void UnRegister();
};

#endif // !defined(AFX_PINBINARY_H__080F5CB3_4174_4B1D_A81E_DCC4CBFE5954__INCLUDED_)
