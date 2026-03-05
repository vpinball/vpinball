// license:GPLv3+

#include "core/stdafx.h"
#include "BiffReader.h"

#include <mutex>
static std::mutex mtx; //!! only used for Wine multithreading bug workaround

#ifdef __STANDALONE__
#include <fstream>
#endif


BiffReader::BiffReader(IStream *pistream, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
   : m_pistream(pistream)
   , m_hcrypthash(hcrypthash)
   , m_hcryptkey(hcryptkey)
   , m_version(version)
{
}

void BiffReader::ReadBytes(void * const pv, const uint32_t count)
{
   const bool iow = IsOnWine();
   if (iow)
      mtx.lock();
   ULONG read = 0;
   m_hasError |= FAILED(m_pistream->Read(pv, count, &read));
   m_hasError |= read != count;
   if (iow)
      mtx.unlock();

#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);
#endif
}

int BiffReader::GetIntNoHash()
{
   m_bytesinrecordremaining -= sizeof(int);

   ULONG read = 0;
   const bool iow = IsOnWine();
   if (iow)
      mtx.lock();
   int value;
   m_hasError |= FAILED(m_pistream->Read(&value, sizeof(int), &read));
   m_hasError |= read != sizeof(int);
   if (iow)
      mtx.unlock();
   return value;
}

bool BiffReader::AsBool()
{
   m_bytesinrecordremaining -= sizeof(int);
   int value;
   ReadBytes(&value, sizeof(int));
   return !!value;
}

int BiffReader::AsInt()
{
   m_bytesinrecordremaining -= sizeof(int);
   int value;
   ReadBytes(&value, sizeof(int));
   return value;
}

unsigned int BiffReader::AsUInt()
{
   m_bytesinrecordremaining -= sizeof(unsigned int);
   unsigned int value;
   ReadBytes(&value, sizeof(unsigned int));
   return value;
}

float BiffReader::AsFloat()
{
   m_bytesinrecordremaining -= sizeof(float);
   float value;
   ReadBytes(&value, sizeof(float));
   return value;
}

string BiffReader::AsString()
{
   int len;
   string value;
   ReadBytes(&len, sizeof(int));
   if (m_hasError)
      return value;
   m_bytesinrecordremaining -= len + (int)sizeof(int);
   value.resize(len, '\0');
   ReadBytes(value.data(), len);
   return value;
}

wstring BiffReader::AsWideString()
{
   // TODO it seems there used to be a bug in collection that would save string twice as long as they should => do we need special processing (truncation ?)
   int len;
   ReadBytes(&len, sizeof(int));
   if (m_hasError)
      return value;
   m_bytesinrecordremaining -= len + (int)sizeof(int);
   const int numChars = len / 2;
#if (WCHAR_T_SIZE == 2) // Windows
   assert(sizeof(WCHAR) == 2);
   wstring value;
#else // Linux, macOS
   assert(sizeof(wchar_t) == 4);
   assert(sizeof(WCHAR) == 4);
   std::u16string value;
#endif
   value.resize(numChars, '\0');
   ReadBytes(value.data(), len);
#if (WCHAR_T_SIZE == 2) // Windows
   return value;
#else // Linux, macOS
   return utf16_to_utf32(value);
#endif
}

Vertex2D BiffReader::AsVector2()
{
   Vertex2D value;
   static_assert(sizeof(Vertex2D) == 2 * sizeof(float)); // fields need to be contiguous
   AsRaw(&value.x, sizeof(Vertex2D));
   return value;
}

vec3 BiffReader::AsVector3()
{
   static_assert(sizeof(vec3) == 3 * sizeof(float)); // fields need to be contiguous
   vec3 value;
   AsRaw(&value.x, sizeof(vec3));
   return value;
}

vec4 BiffReader::AsVector4()
{
   static_assert(sizeof(vec4) == 4 * sizeof(float)); // fields need to be contiguous
   vec4 value;
   AsRaw(&value.x, sizeof(vec4));
   return value;
}

string BiffReader::AsScript(bool isScriptProtected)
{
   string script;
   ULONG read = 0;
   int cchar;
   m_hasError |= FAILED(m_pistream->Read(&cchar, sizeof(int), &read));

   char *szText = new char[cchar + 1];
   m_hasError |= FAILED(m_pistream->Read(szText, cchar * (int)sizeof(char), &read));

#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)szText, cchar, 0);

   // if there is a valid key, then decrypt the script text (now in szText, must be done after the hash is updated)
   if (isScriptProtected && (m_hcryptkey != 0))
   {
      // get the size of the data to decrypt
      DWORD cryptlen = cchar * (int)sizeof(char);

      // decrypt the script
      CryptDecrypt(m_hcryptkey, // key to use
         0, // not hashing data at the same time
         TRUE, // last block (or only block)
         0, // no flags
         (BYTE *)szText, // buffer to decrypt
         &cryptlen); // size of data to decrypt

      GetLastError(); // purge any errors

      // update the size of the buffer
      cchar = cryptlen / (DWORD)sizeof(char);
   }
#endif

   // ensure that the script is null terminated
   szText[cchar] = '\0';

   script = szText;
   delete[] szText;

   return script;
}

FontDesc BiffReader::AsFontDescriptor()
{
   FontDesc fontdesc;
   ReadBytes(&fontdesc.version, 1); // Should always be equal to 1
   ReadBytes(&fontdesc.charset, 2);
   ReadBytes(&fontdesc.attributes, 1);
   ReadBytes(&fontdesc.weight, 2);
   ReadBytes(&fontdesc.size, 4);
   uint8_t nameLen;
   ReadBytes(&nameLen, 1);
   fontdesc.name.resize(nameLen, '\0');
   ReadBytes(fontdesc.name.data(), nameLen);
   return fontdesc;
}

void BiffReader::AsRaw(void *pvalue, const int size)
{
   m_bytesinrecordremaining -= size;
   ReadBytes(pvalue, size);
}

void BiffReader::AsObject(const std::function<bool(const int, IObjectReader &)> &processField, bool isSkippable)
{
   while (true)
   {
      if (m_version > 30)
         m_bytesinrecordremaining = isSkippable ? AsInt() : GetIntNoHash();

      const int tag = AsInt();
      if (m_hasError || tag == FID(ENDB))
         return;

      if (const bool cont = processField(tag, *this); !cont)
      {
         m_hasError = true;
         return;
      }

      if (m_version > 30)
      {
         assert(m_bytesinrecordremaining >= 0);
         if (m_bytesinrecordremaining > 0)
         {
            BYTE * const szT = new BYTE[m_bytesinrecordremaining];
            ReadBytes(szT, m_bytesinrecordremaining);
            delete[] szT;
            if (m_hasError)
               return;
         }
      }
   }
}
