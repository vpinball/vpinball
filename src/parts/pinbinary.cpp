// license:GPLv3+

#include "core/stdafx.h"

PinBinary::PinBinary()
{
}

PinBinary::~PinBinary()
{
}

bool PinBinary::ReadFromFile(const string& filename)
{
   m_buffer = read_file(filename);
   m_path = filename;
   m_name = TitleFromFilename(filename);
   return true;
}

bool PinBinary::WriteToFile(const string& filename)
{
   write_file(filename, m_buffer);
   return true;
}

HRESULT PinBinary::SaveToStream(IStream *pstream)
{
   BiffWriter bw(pstream, 0);

   bw.WriteString(FID(NAME), m_name);
   bw.WriteString(FID(PATH), m_path);
   bw.WriteInt(FID(SIZE), m_buffer.size());
   bw.WriteStruct(FID(DATA), m_buffer.data(), m_buffer.size());
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
   case FID(NAME): pbr->GetString(m_name); break;
   case FID(PATH): pbr->GetString(m_path); break;
   case FID(SIZE):
   {
      int size;
      pbr->GetInt(size);
      m_buffer.resize(size);
      break;
   }
   // Size must come before data, otherwise our structure won't be allocated
   case FID(DATA): pbr->GetStruct(m_buffer.data(), m_buffer.size()); break;
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
