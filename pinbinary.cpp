#include "stdafx.h"

PinBinary::PinBinary()
{
   m_pdata = nullptr;
   m_cdata = 0;
}

PinBinary::~PinBinary()
{
   if (m_pdata)
   {
      delete[] m_pdata;
   }
}

bool PinBinary::ReadFromFile(const string& szFileName)
{
#ifndef __STANDALONE__
   const HANDLE hFile = CreateFile(szFileName.c_str(),
      GENERIC_READ, FILE_SHARE_READ,
      nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      const string text = "The file \"" + szFileName + "\" could not be opened.";
      ShowError(text);
      return false;
   }

   if (m_pdata)
   {
      delete[] m_pdata;
   }

   m_cdata = GetFileSize(hFile, nullptr);

   m_pdata = new char[m_cdata];

   DWORD read;
   /*BOOL foo =*/ ReadFile(hFile, m_pdata, m_cdata, &read, nullptr);

   /*foo =*/ CloseHandle(hFile);

   m_szPath = szFileName;
   m_szName = TitleFromFilename(szFileName);
#endif

   return true;
}

bool PinBinary::WriteToFile(const string& szfilename)
{
#ifndef __STANDALONE__
   const HANDLE hFile = CreateFile(szfilename.c_str(),
      GENERIC_WRITE, FILE_SHARE_READ,
      nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      const string bla = "The temporary file \"" + szfilename + "\" could not be written.";
      ShowError(bla);
      return false;
   }

   DWORD write;
   /*int foo =*/ WriteFile(hFile, m_pdata, m_cdata, &write, nullptr);

   /*foo =*/ GetLastError();

   CloseHandle(hFile);
#endif
   return true;
}

HRESULT PinBinary::SaveToStream(IStream *pstream)
{
   BiffWriter bw(pstream, 0);

   bw.WriteString(FID(NAME), m_szName);
   bw.WriteString(FID(PATH), m_szPath);
   bw.WriteInt(FID(SIZE), m_cdata);
   bw.WriteStruct(FID(DATA), m_pdata, m_cdata);
   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinBinary::LoadFromStream(IStream *pstream, int version)
{
   BiffReader br(pstream, this, nullptr, version, 0, 0);

   br.Load();

   return S_OK;
}

bool PinBinary::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(NAME): pbr->GetString(m_szName); break;
   case FID(PATH): pbr->GetString(m_szPath); break;
   case FID(SIZE):
   {
      pbr->GetInt(m_cdata);
      m_pdata = new char[m_cdata];
      break;
   }
   // Size must come before data, otherwise our structure won't be allocated
   case FID(DATA): pbr->GetStruct(m_pdata, m_cdata); break;
   }
   return true;
}

#ifndef __STANDALONE__
int CALLBACK EnumFontFamExProc(
   ENUMLOGFONTEX *lpelfe,    // logical-font data
   NEWTEXTMETRICEX *lpntme,  // physical-font data
   DWORD FontType,           // type of font
   LPARAM lParam             // application-defined data
   )
{
   return 1;
}
#endif

void PinFont::Register()
{
#ifndef __STANDALONE__
   const HDC hdcScreen = GetDC(nullptr);

   LOGFONT lf;
   lf.lfCharSet = DEFAULT_CHARSET;
   lstrcpy(lf.lfFaceName, "");
   lf.lfPitchAndFamily = 0;

   EnumFontFamiliesEx(hdcScreen, &lf, (FONTENUMPROC)EnumFontFamExProc, (size_t)this, 0);

   ReleaseDC(nullptr, hdcScreen);

   char szPath[MAXSTRING];
   GetModuleFileName(nullptr, szPath, MAXSTRING);

   char *szEnd = szPath + lstrlen(szPath);

   while (szEnd > szPath)
   {
      if (*szEnd == PATH_SEPARATOR_CHAR)
         break;

      szEnd--;
   }

   *(szEnd + 1) = '\0'; // Get rid of exe name

   static int tempFontNumber = -1;
   tempFontNumber++;

   m_szTempFile = szPath + "VPTemp"s + std::to_string(tempFontNumber) + ".ttf";
   WriteToFile(m_szTempFile);

   /*const int fonts =*/ AddFontResource(m_szTempFile.c_str());
#endif
}

void PinFont::UnRegister()
{
#ifndef __STANDALONE__
   /*const BOOL foo =*/ RemoveFontResource(m_szTempFile.c_str());

   DeleteFile(m_szTempFile.c_str());
#endif
}
