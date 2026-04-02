// license:GPLv3+

#include "core/stdafx.h"


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
