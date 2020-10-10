#include "stdafx.h"

bool Exists(const string& filePath)
{
	//This will get the file attributes bitlist of the file
	const DWORD fileAtt = GetFileAttributesA(filePath.c_str());

	//If an error occurred it will equal to INVALID_FILE_ATTRIBUTES
	if (fileAtt == INVALID_FILE_ATTRIBUTES)
		return false;

	//If the path referers to a directory it should also not exists.
	return ((fileAtt & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

void ExtensionFromFilename(const string& szfilename, string& szextension)
{
   const int len = (int)szfilename.length();

   int begin;
   for (begin = len; begin >= 0; begin--)
   {
      if (szfilename[begin] == '.')
      {
         begin++;
         break;
      }
   }

   if (begin <= 0)
      szextension.clear();
   else
      szextension = szfilename.c_str()+begin;
}

void TitleFromFilename(const string& szfilename, string& sztitle)
{
   const int len = (int)szfilename.length();

   int begin;
   for (begin = len; begin >= 0; begin--)
   {
      if (szfilename[begin] == '\\')
      {
         begin++;
         break;
      }
   }

   int end;
   for (end = len; end >= 0; end--)
   {
      if (szfilename[end] == '.')
         break;
   }

   if (end == 0)
      end = len - 1;

   const char *szT = szfilename.c_str()+begin;
   int count = end - begin;

   sztitle.clear();
   while (count--) { sztitle.push_back(*szT++); }
}

void PathFromFilename(const string& szfilename, string& szpath)
{
   const int len = (int)szfilename.length();
   // find the last '\' in the filename
   int end;
   for (end = len; end >= 0; end--)
   {
      if (szfilename[end] == '\\' || szfilename[end] == '/')
         break;
   }

   if (end == 0)
      end = len - 1;

   // copy from the start of the string to the end (or last '\')
   const char * szT = szfilename.c_str();
   int count = end + 1;

   szpath.clear();
   while (count--) { szpath.push_back(*szT++); }
}

void TitleAndPathFromFilename(const char * const szfilename, char *szpath)
{
   const int len = lstrlen(szfilename);
   // find the last '.' in the filename
   int end;
   for (end = len; end >= 0; end--)
   {
      if (szfilename[end] == '.')
         break;
   }

   if (end == 0)
      end = len;

   // copy from the start of the string to the end (or last '\')
   const char *szT = szfilename;
   int count = end;

   while (count-- > 0) { *szpath++ = *szT++; }
   *szpath = '\0';
}

bool ReplaceExtensionFromFilename(string& szfilename, const string& newextension)
{
   const size_t i = szfilename.find_last_of('.');

   if (i != string::npos)
   {
       szfilename.replace(i + 1, newextension.length(), newextension);
       return true;
   }
   else
       return false;
}

bool RawReadFromFile(const char * const szfilename, int *psize, char **pszout)
{
   const HANDLE hFile = CreateFile(szfilename,
      GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
   {
      //ShowError("The file could not be opened.");
      return false;
   }

   *psize = GetFileSize(hFile, NULL);

   *pszout = new char[*psize + 2];

   DWORD read;
   /*BOOL foo =*/ ReadFile(hFile, *pszout, *psize, &read, NULL);

   (*pszout)[*psize] = '\0';
   (*pszout)[*psize + 1] = '\0'; // In case this is a unicode file, end it with a null character properly

   /*foo =*/ CloseHandle(hFile);

   return true;
}

BiffWriter::BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash)
{
   m_pistream = pistream;
   m_hcrypthash = hcrypthash;
}

HRESULT BiffWriter::WriteBytes(const void *pv, const unsigned long count, unsigned long *foo)
{
   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);

   return m_pistream->Write(pv, count, foo);
}

HRESULT BiffWriter::WriteRecordSize(const int size)
{
   ULONG writ = 0;
   HRESULT hr = m_pistream->Write(&size, sizeof(size), &writ);

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

HRESULT BiffWriter::WriteString(const int id, const char * const szvalue)
{
   ULONG writ = 0;
   HRESULT hr;
   const int len = lstrlen(szvalue);

   if (FAILED(hr = WriteRecordSize((int)sizeof(int) * 2 + len)))
      return hr;

   if (FAILED(hr = WriteBytes(&id, sizeof(int), &writ)))
      return hr;

   if (FAILED(hr = WriteBytes(&len, sizeof(int), &writ)))
      return hr;

   hr = WriteBytes(szvalue, len, &writ);

   return hr;
}

HRESULT BiffWriter::WriteString(const int id, const std::string &szvalue)
{
   ULONG writ = 0;
   HRESULT hr;
   const int len = (int)szvalue.size();

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
   const int len = lstrlenW(wzvalue) * (int)sizeof(WCHAR);

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

HRESULT BiffWriter::WriteStruct(const int id, const void *pvalue, const int size)
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

HRESULT BiffWriter::WriteVector3(const int id, const Vertex3Ds* vec)
{
   return WriteStruct(id, &vec->x, 3 * sizeof(float));
}

HRESULT BiffWriter::WriteVector3Padded(const int id, const Vertex3Ds* vec)
{
   const float data[4] = { vec->x,vec->y,vec->z,0.0f };
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

BiffReader::BiffReader(IStream *pistream, ILoadable *piloadable, void *ppassdata, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey)
{
   m_pistream = pistream;
   m_piloadable = piloadable;
   m_pdata = ppassdata;
   m_version = version;

   m_bytesinrecordremaining = 0;

   m_hcrypthash = hcrypthash;
   m_hcryptkey = hcryptkey;
}

HRESULT BiffReader::ReadBytes(void *pv, const unsigned long count, unsigned long *foo)
{
   HRESULT hr = m_pistream->Read(pv, count, foo);

   if (m_hcrypthash)
      CryptHashData(m_hcrypthash, (BYTE *)pv, count, 0);

   return hr;
}

HRESULT BiffReader::GetIntNoHash(void *pvalue)
{
   m_bytesinrecordremaining -= sizeof(int);

   ULONG read = 0;
   return m_pistream->Read(pvalue, sizeof(int), &read);
}

HRESULT BiffReader::GetInt(void *pvalue)
{
   m_bytesinrecordremaining -= sizeof(int);

   ULONG read = 0;
   return ReadBytes(pvalue, sizeof(int), &read);
}

HRESULT BiffReader::GetString(char *szvalue)
{
   ULONG read = 0;
   HRESULT hr;
   int len;

   if (FAILED(hr = ReadBytes(&len, sizeof(int), &read)))
   {
      szvalue[0] = 0;
      return hr;
   }

   m_bytesinrecordremaining -= len + (int)sizeof(int);

   hr = ReadBytes(szvalue, len, &read);
   szvalue[len] = 0;
   return hr;
}

HRESULT BiffReader::GetString(std::string &szvalue)
{
   ULONG read = 0;
   HRESULT hr;
   int len;

   if (FAILED(hr = ReadBytes(&len, sizeof(int), &read)))
   {
      szvalue.clear();
      return hr;
   }

   m_bytesinrecordremaining -= len + (int)sizeof(int);

   char * tmp = new char[len+1];
   hr = ReadBytes(tmp, len, &read);
   tmp[len] = 0;
   szvalue = tmp;
   delete[] tmp;
   return hr;
}

HRESULT BiffReader::GetWideString(WCHAR *wzvalue)
{
   ULONG read = 0;
   HRESULT hr;
   int len;

   if (FAILED(hr = ReadBytes(&len, sizeof(int), &read)))
   {
      wzvalue[0] = 0;
      return hr;
   }

   m_bytesinrecordremaining -= len + (int)sizeof(int);

   hr = ReadBytes(wzvalue, len, &read);
   wzvalue[len/sizeof(WCHAR)] = 0;
   return hr;
}

HRESULT BiffReader::GetWideString(std::basic_string<WCHAR>& wzvalue)
{
   ULONG read = 0;
   HRESULT hr;
   int len;

   if (FAILED(hr = ReadBytes(&len, sizeof(int), &read)))
   {
      wzvalue.clear();
      return hr;
   }

   m_bytesinrecordremaining -= len + (int)sizeof(int);

   WCHAR * tmp = new WCHAR[len/sizeof(WCHAR)+1];
   hr = ReadBytes(tmp, len, &read);
   tmp[len/sizeof(WCHAR)] = 0;
   wzvalue = tmp;
   delete[] tmp;
   return hr;

}

HRESULT BiffReader::GetFloat(float *pvalue)
{
   m_bytesinrecordremaining -= sizeof(float);

   ULONG read = 0;
   return ReadBytes(pvalue, sizeof(float), &read);
}

HRESULT BiffReader::GetBool(BOOL *pfvalue)
{
   m_bytesinrecordremaining -= sizeof(BOOL);

   ULONG read = 0;
   //return m_pistream->Read(pfvalue, sizeof(BOOL), &read);
   return ReadBytes(pfvalue, sizeof(BOOL), &read);
}

HRESULT BiffReader::GetBool(bool *pvalue)
{
   BOOL val;
   HRESULT hr = GetBool(&val);
   *pvalue = !!val;
   return hr;
}

HRESULT BiffReader::GetStruct(void *pvalue, const int size)
{
   m_bytesinrecordremaining -= size;

   ULONG read = 0;
   return ReadBytes(pvalue, size, &read);
}

HRESULT BiffReader::GetVector3(Vertex3Ds* vec)
{
   assert(sizeof(Vertex3Ds) == 3 * sizeof(float));     // fields need to be contiguous
   return GetStruct(&vec->x, 3 * sizeof(float));
}

HRESULT BiffReader::GetVector3Padded(Vertex3Ds* vec)
{
   float data[4];
   HRESULT hr = GetStruct(data, 4 * sizeof(float));
   if (SUCCEEDED(hr))
   {
      vec->x = data[0];
      vec->y = data[1];
      vec->z = data[2];
   }
   return hr;
}

HRESULT BiffReader::Load()
{
   int tag = 0;
   while (tag != FID(ENDB))
   {
      if (m_version > 30)
      {
         /*const HRESULT hr =*/ GetIntNoHash(&m_bytesinrecordremaining);
      }

      const HRESULT hr = GetInt(&tag);

      bool cont = false;
      if (hr == S_OK)
         cont = m_piloadable->LoadToken(tag, this);

      if (!cont)
         return E_FAIL;

      if (m_version > 30)
      {
         assert(m_bytesinrecordremaining >= 0);

         if (m_bytesinrecordremaining > 0)
         {
            BYTE * const szT = new BYTE[m_bytesinrecordremaining];
            /*const HRESULT hr =*/ GetStruct(szT, m_bytesinrecordremaining);
            delete[] szT;
         }
      }
   }

   return S_OK;
}

FastIStorage::FastIStorage()
{
   m_wzName = NULL;
   m_cref = 0;
}

FastIStorage::~FastIStorage()
{
   for (size_t i = 0; i < m_vstg.size(); i++)
      m_vstg[i]->Release();

   for (size_t i = 0; i < m_vstm.size(); i++)
      m_vstm[i]->Release();

   SAFE_VECTOR_DELETE(m_wzName);
}

long __stdcall FastIStorage::QueryInterface(const struct _GUID &, void **)
{
   return S_OK;
}

unsigned long __stdcall FastIStorage::AddRef()
{
   m_cref++;

   return S_OK;
}

unsigned long __stdcall FastIStorage::Release()
{
   m_cref--;

   if (m_cref == 0)
   {
      delete this; // legal, but meh
   }

   return S_OK;
}

long __stdcall FastIStorage::CreateStream(const WCHAR *wzName, unsigned long, unsigned long, unsigned long, struct IStream **ppstm)
{
   FastIStream * const pfs = new FastIStream();
   const int wzNameLen = lstrlenW(wzName) + 1;
   pfs->AddRef(); // AddRef once for us, and once for the caller
   pfs->AddRef();
   pfs->m_wzName = new WCHAR[wzNameLen];
   WideStrNCopy((WCHAR *)wzName, pfs->m_wzName, wzNameLen);

   *ppstm = pfs;

   m_vstm.push_back(pfs);

   return S_OK;
}

long __stdcall FastIStorage::OpenStream(const WCHAR *, void *, unsigned long, unsigned long, struct IStream **)
{
   return S_OK;
}

long __stdcall FastIStorage::CreateStorage(const WCHAR *wzName, unsigned long, unsigned long, unsigned long, struct IStorage **ppstg)
{
   FastIStorage * const pfs = new FastIStorage();
   const int wzNameLen = lstrlenW(wzName) + 1;
   pfs->AddRef(); // AddRef once for us, and once for the caller
   pfs->AddRef();
   pfs->m_wzName = new WCHAR[wzNameLen];
   WideStrNCopy((WCHAR *)wzName, pfs->m_wzName, wzNameLen);

   *ppstg = pfs;

   m_vstg.push_back(pfs);

   return S_OK;
}

long __stdcall FastIStorage::OpenStorage(const WCHAR *, struct IStorage *, unsigned long, WCHAR **, unsigned long, struct IStorage **)
{
   return S_OK;
}

long __stdcall FastIStorage::CopyTo(unsigned long, const struct _GUID *, WCHAR **, struct IStorage *pstgNew)
{
   HRESULT hr;
   IStorage *pstgT;
   IStream *pstmT;

   for (size_t i = 0; i < m_vstg.size(); i++)
   {
      FastIStorage * const pstgCur = m_vstg[i];
      if (SUCCEEDED(hr = pstgNew->CreateStorage(pstgCur->m_wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgT)))
      {
         pstgCur->CopyTo(0, NULL, NULL, pstgT);
         pstgT->Release();
      }
   }

   for (size_t i = 0; i < m_vstm.size(); i++)
   {
      FastIStream * const pstmCur = m_vstm[i];
      if (SUCCEEDED(hr = pstgNew->CreateStream(pstmCur->m_wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmT)))
      {
         ULONG writ;
         //pstmCur->CopyTo(0,NULL,NULL,pstmT);
         pstmT->Write(pstmCur->m_rg, pstmCur->m_cSize, &writ);
         pstmT->Release();
      }
   }

   return S_OK;
}

long __stdcall FastIStorage::MoveElementTo(const WCHAR *, struct IStorage *, const WCHAR *, unsigned long)
{
   return S_OK;
}

long __stdcall FastIStorage::Commit(unsigned long)
{
   return S_OK;
}

long __stdcall FastIStorage::Revert()
{
   return S_OK;
}

long __stdcall FastIStorage::EnumElements(unsigned long, void *, unsigned long, struct IEnumSTATSTG **)
{
   return S_OK;
}

long __stdcall FastIStorage::DestroyElement(const WCHAR *)
{
   return S_OK;
}

long __stdcall FastIStorage::RenameElement(const WCHAR *, const WCHAR *)
{
   return S_OK;
}

long __stdcall FastIStorage::SetElementTimes(const WCHAR *, const struct _FILETIME *, const struct _FILETIME *, const struct _FILETIME *)
{
   return S_OK;
}

long __stdcall FastIStorage::SetClass(const struct _GUID &)
{
   return S_OK;
}

long __stdcall FastIStorage::SetStateBits(unsigned long, unsigned long)
{
   return S_OK;
}

long __stdcall FastIStorage::Stat(struct tagSTATSTG *, unsigned long)
{
   return S_OK;
}

FastIStream::FastIStream()
{
   m_cref = 0;

   m_cMax = 0;
   m_cSeek = 0;
   m_cSize = 0;
   m_rg = NULL;

   m_wzName = NULL;
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

long __stdcall FastIStream::QueryInterface(const struct _GUID &, void **)
{
   return S_OK;
}

unsigned long __stdcall FastIStream::AddRef()
{
   m_cref++;

   return S_OK;
}

unsigned long __stdcall FastIStream::Release()
{
   m_cref--;

   if (m_cref == 0)
   {
      delete this;
   }

   return S_OK;
}

long __stdcall FastIStream::Read(void *pv, const unsigned long count, unsigned long *foo)
{
   memcpy(pv, m_rg + m_cSeek, count);
   m_cSeek += count;

   if (foo != NULL)
   {
      *foo = count;
   }

   return S_OK;
}

long __stdcall FastIStream::Write(const void *pv, const unsigned long count, unsigned long *foo)
{
   if ((m_cSeek + count) > m_cMax)
      SetSize(max(m_cSeek * 2, m_cSeek + count));

   memcpy(m_rg + m_cSeek, pv, count);
   m_cSeek += count;

   m_cSize = max(m_cSize, m_cSeek);

   if (foo != NULL)
      *foo = count;

   return S_OK;
}

long __stdcall FastIStream::Seek(union _LARGE_INTEGER li, const unsigned long origin, union _ULARGE_INTEGER *puiOut)
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
   {
      puiOut->QuadPart = m_cSeek;
   }

   return S_OK;
}

long __stdcall FastIStream::SetSize(union _ULARGE_INTEGER)
{
   return S_OK;
}

long __stdcall FastIStream::CopyTo(struct IStream *, union _ULARGE_INTEGER, union _ULARGE_INTEGER *, union _ULARGE_INTEGER *)
{
   return S_OK;
}

long __stdcall FastIStream::Commit(unsigned long)
{
   return S_OK;
}

long __stdcall FastIStream::Revert()
{
   return S_OK;
}

long __stdcall FastIStream::LockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, unsigned long)
{
   return S_OK;
}

long __stdcall FastIStream::UnlockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, unsigned long)
{
   return S_OK;
}

long __stdcall FastIStream::Stat(struct tagSTATSTG *, unsigned long)
{
   return S_OK;
}

long __stdcall FastIStream::Clone(struct IStream **)
{
   return S_OK;
}
