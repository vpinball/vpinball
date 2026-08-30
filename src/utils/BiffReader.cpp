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

BiffReader::BiffReader(POLE::Stream *stream, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
   : m_stream(stream)
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

   if (m_stream)
   {
      const auto read = m_stream->read(reinterpret_cast<unsigned char *>(pv), count);
      m_hasError |= read != count;
   }
   else
   {
      ULONG read = 0;
      m_hasError |= FAILED(m_pistream->Read(pv, count, &read));
      m_hasError |= read != count;
   }

   if (iow)
      mtx.unlock();

#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);
#endif
}

int BiffReader::GetIntNoHash()
{
   m_bytesinrecordremaining -= sizeof(int32_t);

   ULONG read = 0;
   const bool iow = IsOnWine();
   if (iow)
      mtx.lock();
   int32_t value;
   if (m_stream)
   {
      const auto read = m_stream->read(reinterpret_cast<unsigned char *>(&value), sizeof(value));
      m_hasError |= read != sizeof(value);
   }
   else
   {
      m_hasError |= FAILED(m_pistream->Read(&value, sizeof(value), &read));
      m_hasError |= read != sizeof(value);
   }
   if (iow)
      mtx.unlock();
   return value;
}

bool BiffReader::AsBool()
{
   m_bytesinrecordremaining -= sizeof(int32_t);
   int32_t value;
   ReadBytes(&value, sizeof(int32_t));
   return !!value;
}

int BiffReader::AsInt()
{
   m_bytesinrecordremaining -= sizeof(int32_t);
   int32_t value;
   ReadBytes(&value, sizeof(int32_t));
   return value;
}

unsigned int BiffReader::AsUInt()
{
   m_bytesinrecordremaining -= sizeof(uint32_t);
   uint32_t value;
   ReadBytes(&value, sizeof(uint32_t));
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
   int32_t len;
   ReadBytes(&len, sizeof(int32_t));
   if (m_hasError)
      return string();
   m_bytesinrecordremaining -= len + (int)sizeof(int32_t);
   string value(len, '\0');
   ReadBytes(value.data(), len);
   return value;
}

wstring BiffReader::AsWideString()
{
   // TODO it seems there used to be a bug in collection that would save string twice as long as they should => do we need special processing (truncation ?)
   int32_t len;
   ReadBytes(&len, sizeof(int32_t));
   if (m_hasError)
      return wstring();
   m_bytesinrecordremaining -= len + (int)sizeof(int32_t);
   const int numChars = len / 2;
#if (WCHAR_T_SIZE == 2) // Windows
   wstring value(numChars, L'\0');
#else // Linux, macOS
   std::u16string value(numChars, u'\0');
#endif
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
   AsRaw(&value.x, 2 * sizeof(float));
   return value;
}

vec3 BiffReader::AsVector3()
{
   static_assert(sizeof(vec3) == 3 * sizeof(float)); // fields need to be contiguous
   vec3 value;
   AsRaw(&value.x, 3 * sizeof(float));
   return value;
}

vec4 BiffReader::AsVector4()
{
   static_assert(sizeof(vec4) == 4 * sizeof(float)); // fields need to be contiguous
   vec4 value;
   AsRaw(&value.x, 4 * sizeof(float));
   return value;
}

string BiffReader::AsScript(bool isScriptProtected)
{
   static_assert(sizeof(char) == 1);
   string script;
   ULONG read = 0;
   int32_t cchar;
   if (m_stream)
   {
      const auto read = m_stream->read(reinterpret_cast<unsigned char *>(&cchar), sizeof(cchar));
      m_hasError |= read != sizeof(cchar);
   }
   else
   {
      m_hasError |= FAILED(m_pistream->Read(&cchar, sizeof(cchar), &read));
      m_hasError |= read != sizeof(cchar);
   }

   char *szText = new char[cchar + 1];
   if (m_stream)
   {
      const auto read = m_stream->read(reinterpret_cast<unsigned char *>(szText), cchar);
      m_hasError |= read != cchar;
   }
   else
   {
      m_hasError |= FAILED(m_pistream->Read(szText, cchar, &read));
      m_hasError |= read != cchar;
   }

#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)szText, cchar, 0);

   // if there is a valid key, then decrypt the script text (now in szText, must be done after the hash is updated)
   if (isScriptProtected && (m_hcryptkey != 0))
   {
      // get the size of the data to decrypt
      DWORD cryptlen = cchar;

      // decrypt the script
      CryptDecrypt(m_hcryptkey, // key to use
         0, // not hashing data at the same time
         TRUE, // last block (or only block)
         0, // no flags
         (BYTE *)szText, // buffer to decrypt
         &cryptlen); // size of data to decrypt

      GetLastError(); // purge any errors

      // update the size of the buffer
      cchar = cryptlen;
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
   const int recordSize = m_bytesinrecordremaining;
   const bool skip = isSkippable && m_version > 30;
   const auto getStreamPos = [this]()
   {
      if (m_stream)
      {
         return static_cast<uint64_t>(m_stream->tell());
      }
      else
      {
         ULARGE_INTEGER uiPos;
         LARGE_INTEGER seek { };
         m_pistream->Seek(seek, STREAM_SEEK_CUR, &uiPos);
         return static_cast<uint64_t>(uiPos.QuadPart);
      }
   };

   uint64_t pos = skip ? getStreamPos() : 0;
   while (true)
   {
      if (m_version > 30)
         m_bytesinrecordremaining = isSkippable ? AsInt() : GetIntNoHash();

      const int tag = AsInt();
      if (m_hasError)
         return;

      // PLOGD << "FieldTag: " << (char)((tag) & 0xFF) << (char)((tag >> 8) & 0xFF) << (char)((tag >> 16) & 0xFF) << (char)((tag >> 24) & 0xFF);

      if (tag == FID(ENDB))
         break;

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
            PLOGI << "While reading tag " << (char)(tag & 0xFF) << (char)((tag >> 8) & 0xFF) << (char)((tag >> 16) & 0xFF) << (char)((tag >> 24) & 0xFF) << " " << m_bytesinrecordremaining
                  << " were not read and therefore skipped";
            vector<uint8_t> tmp(m_bytesinrecordremaining);
            ReadBytes(tmp.data(), m_bytesinrecordremaining);
            if (m_hasError)
               return;
         }
      }
   }

   if (skip)
   {
      uint64_t newpos = getStreamPos();
      const int sizeRead = static_cast<int>(newpos - pos);
      if (const int toSkip = recordSize - sizeRead; toSkip > 0)
      {
         if (m_stream)
         {
            m_stream->seek(newpos + toSkip);
         }
         else
         {
            vector<uint8_t> tmp(toSkip);
            ReadBytes(tmp.data(), toSkip);
         }
      }
   }
}
