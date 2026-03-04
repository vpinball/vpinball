// license:GPLv3+

#include "core/stdafx.h"
#include "BiffWriter.h"

#include <mutex>
static std::mutex mtx; //!! only used for Wine multithreading bug workaround

#ifdef __STANDALONE__
#include <fstream>
#endif

BiffWriter::BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash)
   : m_pistream(pistream)
   , m_hcrypthash(hcrypthash)
{
}

void BiffWriter::WriteRecordSize(const int size)
{
   ULONG written = 0;
   m_hasError |= FAILED(m_pistream->Write(&size, sizeof(size), &written));
   m_hasError |= written != sizeof(size);

#ifndef __STANDALONE__
   if (m_hcrypthash && !m_subObjectRecordSizePos.empty() && m_subObjectRecordSizePos.back().QuadPart >= 0)
      CryptHashData(m_hcrypthash, (BYTE *)&size, sizeof(size), 0);
#endif
}

void BiffWriter::WriteBytes(const void *pv, const ULONG count)
{
   ULONG written = 0;
   m_hasError |= FAILED(m_pistream->Write(pv, count, &written));
   m_hasError |= written != count;

#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);
#endif
}

void BiffWriter::BeginObject(const int objectId, bool isArray, bool isSkippable)
{
   // BIFF implementation has always been very hacky regarding encapsulating sub objects.
   // Most legacy sub arry/objects are encapsulated by having a tag, then the sub object ended by
   // a ENDB tag, but the initial tag does not encompass the object in its recordsize. Therefore,
   // if it is not recognized, it won't be skipped and the following field will be wrongly read 
   // and the ENDB tag will be interpreted as the end of the container object and not as the end 
   // of the sub object. To solve this, later sub object have increased the record size of their 
   // initial tag, to cover the complete subobject data up to the end of the ENDB block.
   // Sub objects saved with isSkippable must be read with the reader that found them in order
   // to actuaaly read the record bytes.
   WriteRecordSize(sizeof(int));
   LARGE_INTEGER seek {};
   if (isSkippable)
   {
      ULARGE_INTEGER pos;
      m_pistream->Seek(seek, STREAM_SEEK_CUR, &pos);
      seek.QuadPart = pos.QuadPart - sizeof(int);
      m_subObjectRecordSizePos.push_back(seek);
   }
   else
   {
      seek.QuadPart = -1;
      m_subObjectRecordSizePos.push_back(seek);
   }
   WriteBytes(&objectId, sizeof(int));
}

void BiffWriter::WriteBool(const int id, const bool value)
{
   int v = value ? 1 : 0;
   WriteRecordSize(sizeof(int) * 2);
   WriteBytes(&id, sizeof(int));
   WriteBytes(&v, sizeof(int));
}

void BiffWriter::WriteInt(const int id, const int value)
{
   WriteRecordSize(sizeof(int) * 2);
   WriteBytes(&id, sizeof(int));
   WriteBytes(&value, sizeof(int));
}

void BiffWriter::WriteUInt(const int id, const unsigned int value)
{
   WriteRecordSize(sizeof(int) * 2);
   WriteBytes(&id, sizeof(int));
   WriteBytes(&value, sizeof(int));
}

void BiffWriter::WriteFloat(const int id, const float value)
{
   WriteRecordSize(sizeof(int) + sizeof(float));
   WriteBytes(&id, sizeof(int));
   WriteBytes(&value, sizeof(float));
}

void BiffWriter::WriteString(const int id, const string &szvalue)
{
   const int len = (int)szvalue.length();
   WriteRecordSize((int)sizeof(int) * 2 + len);
   WriteBytes(&id, sizeof(int));
   WriteBytes(&len, sizeof(int));
   WriteBytes(szvalue.data(), len);
}

void BiffWriter::WriteWideString(const int id, const wstring& wzvalue)
{
   const int len = (int)wzvalue.length() * (int)sizeof(WCHAR);
   WriteRecordSize((int)sizeof(int) * 2 + len);
   WriteBytes(&id, sizeof(int));
   WriteBytes(&len, sizeof(int));
   WriteBytes(wzvalue.data(), len);
}

void BiffWriter::WriteVector2(const int id, const Vertex2D &vec) { WriteRaw(id, &vec.x, 2 * sizeof(float)); }

void BiffWriter::WriteVector3(const int id, const vec3 &vec) { WriteRaw(id, &vec.x, 3 * sizeof(float)); }

void BiffWriter::WriteVector4(const int id, const vec4 &vec) { WriteRaw(id, &vec.x, 4 * sizeof(float)); }

void BiffWriter::WriteScript(int fieldId, const string &value)
{
   // Not really a valid BIFF format: the object tag is directly followed by data without a field id
   ULONG writ = 0;
   BeginObject(FID(CODE), false, false);
   size_t nBytes = value.size();
   m_hasError |= FAILED(m_pistream->Write(&nBytes, static_cast<ULONG>(sizeof(int)), &writ));
   m_hasError |= FAILED(m_pistream->Write(value.c_str(), static_cast<ULONG>(nBytes), &writ));
#ifndef __STANDALONE__
   CryptHashData(m_hcrypthash, (const BYTE*)(value.c_str()), static_cast<DWORD>(nBytes), 0);
#endif
}

void BiffWriter::WriteRaw(const int id, const void *pvalue, const int size)
{
   WriteRecordSize((int)sizeof(int) + size);
   WriteBytes(&id, sizeof(int));
   WriteBytes(pvalue, size);
}

void BiffWriter::WriteFontDescriptor(int fieldId, const FontDesc &fontdesc)
{
   const uint8_t nameLen = static_cast<uint8_t>(fontdesc.name.size());
   WriteRecordSize(sizeof(int)); // Invalid BIFF file format: the size does not include the data blob and fields are untagged
   WriteBytes(&fieldId, sizeof(int));
   WriteBytes(&fontdesc.version, 1); // Should always be equal to 1
   WriteBytes(&fontdesc.charset, 2);
   WriteBytes(&fontdesc.attributes, 1);
   WriteBytes(&fontdesc.weight, 2);
   WriteBytes(&fontdesc.size, 4);
   WriteBytes(&nameLen, 1);
   WriteBytes(fontdesc.name.data(), nameLen);
}

void BiffWriter::EndObject()
{
   const int fieldId = FID(ENDB);
   WriteRecordSize(sizeof(int));
   WriteBytes(&fieldId, sizeof(int));

   if (!m_subObjectRecordSizePos.empty())
   {
      LARGE_INTEGER seekStart = m_subObjectRecordSizePos.back();
      m_subObjectRecordSizePos.pop_back();
      if (seekStart.QuadPart >= 0)
      {
         LARGE_INTEGER seek0 {};
         ULARGE_INTEGER curPos;
         m_pistream->Seek(seek0, STREAM_SEEK_CUR, &curPos);
         const int size = static_cast<int>(curPos.QuadPart - 2 * sizeof(int) - seekStart.QuadPart);
         m_pistream->Seek(seekStart, STREAM_SEEK_SET, nullptr);
         WriteRecordSize((int)sizeof(int) + size);
         seekStart.QuadPart = curPos.QuadPart;
         m_pistream->Seek(seekStart, STREAM_SEEK_SET, nullptr);
      }
   }
}
