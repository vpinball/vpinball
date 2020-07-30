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

bool PinBinary::ReadFromFile(const string& szFileName)
{
   HANDLE hFile = CreateFile(szFileName.c_str(),
      GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

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

   m_cdata = GetFileSize(hFile, NULL);

   m_pdata = new char[m_cdata];

   DWORD read;
   /*BOOL foo =*/ ReadFile(hFile, m_pdata, m_cdata, &read, NULL);

   /*foo =*/ CloseHandle(hFile);

   m_szPath = szFileName;
   TitleFromFilename(szFileName, m_szName);

   return true;
}

bool PinBinary::WriteToFile(const string& szfilename)
{
   HANDLE hFile = CreateFile(szfilename.c_str(),
      GENERIC_WRITE, FILE_SHARE_READ,
      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      const string bla = "The temporary file \"" + szfilename + "\" could not be written.";
      ShowError(bla);
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
   BiffWriter bw(pstream, NULL);

   bw.WriteString(FID(NAME), m_szName);
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

bool PinBinary::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(NAME): pbr->GetString(m_szName); break;
   case FID(PATH): pbr->GetString(m_szPath); break;
   case FID(SIZE):
   {
      pbr->GetInt(&m_cdata);
      m_pdata = new char[m_cdata];
      break;
   }
   // Size must come before data, otherwise our structure won't be allocated
   case FID(DATA): pbr->GetStruct(m_pdata, m_cdata); break;
   }
   return true;
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

   char szPath[MAXSTRING];
   GetModuleFileName(NULL, szPath, MAXSTRING);

   char *szEnd = szPath + lstrlen(szPath);

   while (szEnd > szPath)
   {
      if (*szEnd == '\\')
         break;

      szEnd--;
   }

   *(szEnd + 1) = '\0'; // Get rid of exe name

   static int tempFontNumber = -1;
   tempFontNumber++;

   m_szTempFile = szPath + string("VPTemp") + std::to_string(tempFontNumber) + ".ttf";
   WriteToFile(m_szTempFile);

   /*const int fonts =*/ AddFontResource(m_szTempFile.c_str());
}

void PinFont::UnRegister()
{
   /*const BOOL foo =*/ RemoveFontResource(m_szTempFile.c_str());

   DeleteFile(m_szTempFile.c_str());
}
