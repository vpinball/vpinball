#include "StdAfx.h"

PinBinary::PinBinary()
{
   m_pdata = NULL;
   m_cdata = 0;
}

PinBinary::~PinBinary()
{
   if (m_pdata)
   {
      delete[] m_pdata;
   }
}

bool PinBinary::ReadFromFile(char *szfilename)
{
   HANDLE hFile = CreateFile(szfilename,
      GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      char text[MAX_PATH];
      sprintf_s(text, "The file \"%s\" could not be opened.", szfilename);
      ShowError(text);
      return false;
   }

   if (m_pdata)
   {
      delete[] m_pdata;
   }

   m_cdata = GetFileSize(hFile, NULL);

   m_pdata = new char[m_cdata];

   DWORD read;

   /*int fFoo =*/ ReadFile(hFile, m_pdata, m_cdata, &read, NULL);

   /*fFoo =*/ CloseHandle(hFile);

   strncpy_s(m_szPath, szfilename, MAX_PATH);

   TitleFromFilename(szfilename, m_szName);

   strncpy_s(m_szInternalName, m_szName, MAXTOKEN);

   CharLowerBuff(m_szInternalName, lstrlen(m_szInternalName));
   return true;
}

bool PinBinary::WriteToFile(char *szfilename)
{
   HANDLE hFile = CreateFile(szfilename,
      GENERIC_WRITE, FILE_SHARE_READ,
      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      ShowError("The temporary file could not be written.");
      return false;
   }

   DWORD write;
   /*int foo =*/ WriteFile(hFile, m_pdata, m_cdata, &write, NULL);

   /*foo =*/ GetLastError();

   CloseHandle(hFile);
   return true;
}

HRESULT PinBinary::SaveToStream(IStream *pstream)
{
   //HRESULT hr;

   BiffWriter bw(pstream, NULL, NULL);

   bw.WriteString(FID(NAME), m_szName);

   bw.WriteString(FID(INME), m_szInternalName);

   bw.WriteString(FID(PATH), m_szPath);

   bw.WriteInt(FID(SIZE), m_cdata);

   bw.WriteStruct(FID(DATA), m_pdata, m_cdata);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinBinary::LoadFromStream(IStream *pstream, int version)
{
   BiffReader br(pstream, this, NULL, version, NULL, NULL);

   br.Load();

   return S_OK;
}

BOOL PinBinary::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(NAME))
   {
      pbr->GetString(m_szName);
   }
   else if (id == FID(INME))
   {
      pbr->GetString(m_szInternalName);
   }
   else if (id == FID(PATH))
   {
      pbr->GetString(m_szPath);
   }
   else if (id == FID(SIZE))
   {
      pbr->GetInt(&m_cdata);
      m_pdata = new char[m_cdata];
   }
   else if (id == FID(DATA))
   {
      // Size must come before data, otherwise our structure won't be allocated
      pbr->GetStruct(m_pdata, m_cdata);
   }
   return fTrue;
}

int CALLBACK EnumFontFamExProc(
   ENUMLOGFONTEX *lpelfe,    // logical-font data
   NEWTEXTMETRICEX *lpntme,  // physical-font data
   DWORD FontType,           // type of font
   LPARAM lParam             // application-defined data
   )
{
   return 1;
}

void PinFont::Register()
{

   HDC hdcScreen = GetDC(NULL);

   LOGFONT lf;

   lf.lfCharSet = DEFAULT_CHARSET;
   lstrcpy(lf.lfFaceName, "");
   lf.lfPitchAndFamily = 0;

   EnumFontFamiliesEx(hdcScreen, &lf, (FONTENUMPROC)EnumFontFamExProc, (size_t)this, 0);

   ReleaseDC(NULL, hdcScreen);

   char szPath[MAX_PATH];

   GetModuleFileName(NULL, szPath, MAX_PATH);

   char *szEnd = szPath + lstrlen(szPath);

   while (szEnd > szPath)
   {
      if (*szEnd == '\\')
      {
         break;
      }
      szEnd--;
   }

   *(szEnd + 1) = '\0'; // Get rid of exe name

   lstrcat(szPath, "VPTemp.ttf");

   strcpy_s(m_szTempFile, sizeof(m_szTempFile), szPath);

   WriteToFile(m_szTempFile);

   /*const int fonts =*/ AddFontResource(m_szTempFile);
}

void PinFont::UnRegister()
{
   /*const BOOL fFoo =*/ RemoveFontResource(m_szTempFile);

   DeleteFile(m_szTempFile);
}
