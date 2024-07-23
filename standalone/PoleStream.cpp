#include "stdafx.h"
#include "PoleStream.h"

HRESULT PoleStream::Create(PoleStorage* pStorage, const string& szName, IStream** ppstm)
{
   PoleStream* pStm = new PoleStream();
   pStm->m_pPOLEStream = new POLE::Stream(pStorage->getPOLEStorage(), pStorage->getPath() + '/' + szName);
   pStm->m_pStorage = pStorage;

   pStm->AddRef();

   *ppstm = pStm;

   return S_OK;
}

STDMETHODIMP PoleStream::QueryInterface(REFIID iid, void **ppvObjOut) { return E_NOTIMPL; }

STDMETHODIMP_(ULONG) PoleStream::AddRef()
{
   m_dwRef++;

   return m_dwRef;
}

STDMETHODIMP_(ULONG) PoleStream::Release()
{
   ULONG dwRef = --m_dwRef;

   if (dwRef == 0) {
      delete m_pPOLEStream;

      m_pStorage->Release();
   
      delete this;
   }

   return dwRef;
}

STDMETHODIMP PoleStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
   if (dwOrigin == STREAM_SEEK_SET)
      m_pPOLEStream->seek(dlibMove.QuadPart);
   else if (dwOrigin == STREAM_SEEK_CUR)
      m_pPOLEStream->seek(m_pPOLEStream->tell() + dlibMove.QuadPart);

   return S_OK;
}

STDMETHODIMP PoleStream::SetSize(ULARGE_INTEGER libNewSize) { return E_NOTIMPL; }
STDMETHODIMP PoleStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) { return E_NOTIMPL; }
STDMETHODIMP PoleStream::Commit(DWORD grfCommitFlags) { return E_NOTIMPL; }
STDMETHODIMP PoleStream::Revert() { return E_NOTIMPL; }
STDMETHODIMP PoleStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) { return E_NOTIMPL; }
STDMETHODIMP PoleStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) { return E_NOTIMPL; }

STDMETHODIMP PoleStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
   pstatstg->cbSize.LowPart = m_pPOLEStream->size();
   return S_OK;
}

STDMETHODIMP PoleStream::Clone(IStream **ppstm) { return E_NOTIMPL; }

STDMETHODIMP PoleStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
   *pcbRead = m_pPOLEStream->read((unsigned char*)pv, cb);
   return S_OK;
}

STDMETHODIMP PoleStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten) { return E_NOTIMPL; }
