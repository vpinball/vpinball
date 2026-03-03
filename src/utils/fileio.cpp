// license:GPLv3+

#include "core/stdafx.h"

#include <mutex>
static std::mutex mtx; //!! only used for Wine multithreading bug workaround

#ifdef __STANDALONE__
#include <fstream>
#endif

bool DirExists(const std::filesystem::path& dirPath)
{
   return std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath);
}

bool FileExists(const std::filesystem::path& filePath)
{
   return std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath);
}

string TitleFromFilename(const std::filesystem::path& filename)
{
   return filename.stem().string();
}

std::filesystem::path PathFromFilename(const std::filesystem::path &filename)
{
   return filename.parent_path();
}

BiffWriter::BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash)
{
   m_pistream = pistream;
   m_hcrypthash = hcrypthash;
}

HRESULT BiffWriter::WriteBytes(const void *pv, const ULONG count, ULONG *foo)
{
#ifndef __STANDALONE__
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);
#endif

   return m_pistream->Write(pv, count, foo);
}

HRESULT BiffWriter::WriteRecordSize(const int size)
{
   ULONG writ = 0;
   const HRESULT hr = m_pistream->Write(&size, sizeof(size), &writ);
   return hr;
}

HRESULT BiffWriter::WriteInt(const int id, const int value)
{
   ULONG writ = 0;
   HRESULT hr;

   if (FAILED(hr = WriteRecordSize(sizeof(int) * 2)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(&value, sizeof(int), &writ);

   return hr;
}

HRESULT BiffWriter::WriteString(const int id, const string &szvalue)
{
   ULONG writ = 0;
   HRESULT hr;
   const int len = (int)szvalue.length();

   if (FAILED(hr = WriteRecordSize((int)sizeof(int) * 2 + len)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = WriteBytes(&len, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(szvalue.data(), len, &writ);

   return hr;
}

HRESULT BiffWriter::WriteWideString(const int id, const WCHAR * const wzvalue)
{
   ULONG writ = 0;
   HRESULT hr;
   const int len = (int)wcslen(wzvalue) * (int)sizeof(WCHAR);

   if (FAILED(hr = WriteRecordSize((int)sizeof(int) * 2 + len)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = WriteBytes(&len, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(wzvalue, len, &writ);

   return hr;
}

HRESULT BiffWriter::WriteWideString(const int id, const std::basic_string<WCHAR>& wzvalue)
{
   ULONG writ = 0;
   HRESULT hr;
   const int len = (int)wzvalue.length() * (int)sizeof(WCHAR);

   if (FAILED(hr = WriteRecordSize((int)sizeof(int) * 2 + len)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = WriteBytes(&len, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(wzvalue.data(), len, &writ);

   return hr;
}

HRESULT BiffWriter::WriteBool(const int id, const BOOL value)
{
   ULONG writ = 0;
   HRESULT hr;

   if (FAILED(hr = WriteRecordSize(sizeof(int) * 2)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(&value, sizeof(BOOL), &writ);

   return hr;
}

HRESULT BiffWriter::WriteFloat(const int id, const float value)
{
   ULONG writ = 0;
   HRESULT hr;

   if (FAILED(hr = WriteRecordSize(sizeof(int) + sizeof(float))))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(&value, sizeof(float), &writ);

   return hr;
}

HRESULT BiffWriter::WriteStruct(const int id, const void * const pvalue, const int size)
{
   ULONG writ = 0;
   HRESULT hr;

   if (FAILED(hr = WriteRecordSize((int)sizeof(int) + size)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(pvalue, size, &writ);

   return hr;
}

HRESULT BiffWriter::WriteVector2(const int id, const Vertex2D& vec)
{
   return WriteStruct(id, &vec.x, 2 * sizeof(float));
}

HRESULT BiffWriter::WriteVector3(const int id, const Vertex3Ds& vec)
{
   return WriteStruct(id, &vec.x, 3 * sizeof(float));
}

HRESULT BiffWriter::WriteVector3Padded(const int id, const Vertex3Ds& vec)
{
   const float data[4] = { vec.x,vec.y,vec.z,0.0f };
   return WriteStruct(id, data, 4 * sizeof(float));
}

HRESULT BiffWriter::WriteTag(const int id)
{
   ULONG writ = 0;
   HRESULT hr;

   if (FAILED(hr = WriteRecordSize(sizeof(int))))
      return hr;

   hr = WriteBytes(&id, sizeof(int), &writ);

   return hr;
}

BiffReader::BiffReader(IStream *pistream, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
   : m_version(version)
   , m_pistream(pistream)
   , m_hcrypthash(hcrypthash)
   , m_hcryptkey(hcryptkey)
{
}

void BiffReader::ReadBytes(void * const pv, const uint32_t count)
{
   const bool iow = IsOnWine();
   if (iow)
      mtx.lock();
   ULONG foo;
   HRESULT hr = m_pistream->Read(pv, count, &foo);
   m_hasError |= foo != count; // if we didn't read the expected number of bytes, return an error
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
   if (iow)
      mtx.unlock();
   return value;
}

bool BiffReader::AsBool()
{
   m_bytesinrecordremaining -= sizeof(BOOL);
   BOOL value;
   ReadBytes(&value, sizeof(BOOL));
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
   wstring value;
   ReadBytes(&len, sizeof(int));
   if (m_hasError)
      return value;
   m_bytesinrecordremaining -= len + (int)sizeof(int);
   value.resize(len / 2, '\0');
   ReadBytes(value.data(), len);
   return value;
}

Vertex2D BiffReader::AsVector2()
{
   Vertex2D value;
   assert(sizeof(Vertex2D) == 2 * sizeof(float)); // fields need to be contiguous
   AsRaw(&value.x, 2 * sizeof(float));
   return value;
}

Vertex3Ds BiffReader::AsVector3(bool isPadded)
{
   Vertex3Ds value;
   if (isPadded)
   {
      float data[4];
      AsRaw(data, 4 * sizeof(float));
      value.x = data[0];
      value.y = data[1];
      value.z = data[2];
   }
   else
   {
      assert(sizeof(Vertex3Ds) == 3 * sizeof(float)); // fields need to be contiguous
      AsRaw(&value.x, 3 * sizeof(float));
   }
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

      /*const int foo =*/GetLastError(); // purge any errors

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

IObjectReader::FontDesc BiffReader::AsFontDescriptor()
{
   IObjectReader::FontDesc fontdesc;
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

void BiffReader::AsObject(const std::function<bool(const int, IObjectReader &)> &processField)
{
   int tag = 0;
   while (!m_hasError && tag != FID(ENDB))
   {
      if (m_version > 30)
         m_bytesinrecordremaining = GetIntNoHash();

      tag = AsInt();
      if (m_hasError)
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
         }
      }
   }
}



FastIStorage::FastIStorage()
   : m_cref(0)
   , m_wzName(nullptr)
{
}

FastIStorage::~FastIStorage()
{
   for (size_t i = 0; i < m_vstg.size(); i++)
      m_vstg[i]->Release();

   for (size_t i = 0; i < m_vstm.size(); i++)
      m_vstm[i]->Release();

   SAFE_VECTOR_DELETE(m_wzName);
}

ULONG __stdcall FastIStorage::AddRef()
{
   m_cref++;

   return S_OK;
}

ULONG __stdcall FastIStorage::Release()
{
   m_cref--;

   if (m_cref == 0)
   {
      delete this; // legal, but meh
   }

   return S_OK;
}

HRESULT __stdcall FastIStorage::CreateStream(const WCHAR *wzName, ULONG, ULONG, ULONG, struct IStream **ppstm)
{
   FastIStream * const pfs = new FastIStream();
   const size_t wzNameLen = wcslen(wzName) + 1; // incl. zero terminator
   pfs->AddRef(); // AddRef once for us, and once for the caller
   pfs->AddRef();
   pfs->m_wzName = new WCHAR[wzNameLen];
   wcsncpy_s(pfs->m_wzName, wzNameLen, wzName);

   *ppstm = pfs;

   m_vstm.push_back(pfs);

   return S_OK;
}

HRESULT __stdcall FastIStorage::CreateStorage(const WCHAR *wzName, ULONG, ULONG, ULONG, struct IStorage **ppstg)
{
   FastIStorage * const pfs = new FastIStorage();
   const size_t wzNameLen = wcslen(wzName) + 1; // incl. zero terminator
   pfs->AddRef(); // AddRef once for us, and once for the caller
   pfs->AddRef();
   pfs->m_wzName = new WCHAR[wzNameLen];
   wcsncpy_s(pfs->m_wzName, wzNameLen, wzName);

   *ppstg = pfs;

   m_vstg.push_back(pfs);

   return S_OK;
}

HRESULT __stdcall FastIStorage::CopyTo(ULONG, const struct _GUID *, WCHAR **, struct IStorage *pstgNew)
{
   HRESULT hr;
   IStorage *pstgT;
   IStream *pstmT;

   for (size_t i = 0; i < m_vstg.size(); i++)
   {
      FastIStorage * const pstgCur = m_vstg[i];
      if (SUCCEEDED(hr = pstgNew->CreateStorage(pstgCur->m_wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgT)))
      {
         pstgCur->CopyTo(0, nullptr, nullptr, pstgT);
         pstgT->Release();
      }
   }

   for (size_t i = 0; i < m_vstm.size(); i++)
   {
      const FastIStream * const pstmCur = m_vstm[i];
      if (SUCCEEDED(hr = pstgNew->CreateStream(pstmCur->m_wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmT)))
      {
         ULONG writ;
         //pstmCur->CopyTo(0, nullptr, nullptr, pstmT);
         pstmT->Write(pstmCur->m_rg, pstmCur->m_cSize, &writ);
         pstmT->Release();
      }
   }

   return S_OK;
}

FastIStream::FastIStream()
   : m_rg(nullptr)
   , m_wzName(nullptr)
   , m_cSize(0)
   , m_cref(0)
   , m_cMax(0)
   , m_cSeek(0)
{
}

FastIStream::~FastIStream()
{
   free(m_rg);
   SAFE_VECTOR_DELETE(m_wzName);
}

void FastIStream::SetSize(const unsigned int i)
{
   if (i > m_cMax)
   {
      void *m_rgNew;

      if (m_rg)
         m_rgNew = realloc((void *)m_rg, i);
      else
         m_rgNew = malloc(i);

      m_rg = (char *)m_rgNew;
      m_cMax = i;
   }
}

ULONG __stdcall FastIStream::AddRef()
{
   m_cref++;

   return S_OK;
}

ULONG __stdcall FastIStream::Release()
{
   m_cref--;

   if (m_cref == 0)
   {
      delete this; //!! legal, but meh
   }

   return S_OK;
}

HRESULT __stdcall FastIStream::Read(void *pv, const ULONG count, ULONG *foo)
{
   memcpy(pv, m_rg + m_cSeek, count);
   m_cSeek += count;

   if (foo != nullptr)
      *foo = count;

   return S_OK;
}

HRESULT __stdcall FastIStream::Write(const void *pv, const ULONG count, ULONG *foo)
{
   if ((m_cSeek + (unsigned int)count) > m_cMax)
      SetSize(max(m_cSeek * 2, m_cSeek + (unsigned int)count));

   memcpy(m_rg + m_cSeek, pv, count);
   m_cSeek += (unsigned int)count;

   m_cSize = max(m_cSize, m_cSeek);

   if (foo != nullptr)
      *foo = count;

   return S_OK;
}

HRESULT __stdcall FastIStream::Seek(union _LARGE_INTEGER li, const ULONG origin, union _ULARGE_INTEGER *puiOut)
{
   switch (origin)
   {
   case STREAM_SEEK_SET:
      m_cSeek = li.LowPart;
      break;

   case STREAM_SEEK_CUR:
      m_cSeek += li.LowPart;
      break;
   }

   if (puiOut)
      puiOut->QuadPart = m_cSeek;

   return S_OK;
}
