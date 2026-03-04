// license:GPLv3+

#include "core/stdafx.h"

bool PinBinary::ReadFromFile(const std::filesystem::path& filename)
{
   m_buffer = read_file(filename);
   m_path = filename;
   m_name = TitleFromFilename(filename);
   return true;
}

bool PinBinary::WriteToFile(const string& filename) const
{
   write_file(filename, m_buffer);
   return true;
}

void PinBinary::Save(IObjectWriter& writer) const
{
   writer.WriteString(FID(NAME), m_name);
   writer.WriteString(FID(PATH), m_path.string());
   writer.WriteInt(FID(SIZE), static_cast<int>(m_buffer.size()));
   writer.WriteRaw(FID(DATA), m_buffer.data(), static_cast<int>(m_buffer.size()));
   writer.EndObject();
}

void PinBinary::Load(IObjectReader& reader)
{
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         switch (tag)
         {
         case FID(NAME): m_name = reader.AsString(); break;
         case FID(PATH): m_path = reader.AsString(); break;
         case FID(SIZE): m_buffer.resize(reader.AsInt()); break;
         // Size must come before data, otherwise our structure won't be allocated
         case FID(DATA): reader.AsRaw(m_buffer.data(), static_cast<int>(m_buffer.size())); break;
         }
         return true;
      });
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
   lf.lfFaceName[0] = '\0';
   lf.lfPitchAndFamily = 0;

   EnumFontFamiliesEx(hdcScreen, &lf, (FONTENUMPROC)EnumFontFamExProc, (size_t)this, 0);

   ReleaseDC(nullptr, hdcScreen);

   string path = GetExecutablePath();
   const size_t pos = path.find_last_of(PATH_SEPARATOR_CHAR);
   if (pos != string::npos)
       path.erase(pos + 1);

   static int tempFontNumber = -1;
   tempFontNumber++;

   m_szTempFile = path + "VPTemp" + std::to_string(tempFontNumber) + ".ttf";
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
