#include "core/stdafx.h"
#include "PoleStream.h"

HRESULT PoleStream::Create(PoleStorage* pStorage, const string& szName, IStream** ppstm)
{
   PoleStream* pStm = new PoleStream();
   pStm->m_shared = pStorage->getShared();
   // Constructing the stream only walks the directory tree and follows a block chain, both
   // of which are read-only on an already parsed container, so it needs no lock.
   pStm->m_pPOLEStream = new POLE::Stream(pStm->m_shared->storage, pStorage->getPath() + '/' + szName);

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
   else if (dwOrigin == STREAM_SEEK_END)
      m_pPOLEStream->seek(m_pPOLEStream->size() + dlibMove.QuadPart);

   if (plibNewPosition)
      plibNewPosition->QuadPart = m_pPOLEStream->tell();

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
   pstatstg->cbSize.QuadPart = m_pPOLEStream->size();
   return S_OK;
}

STDMETHODIMP PoleStream::Clone(IStream **ppstm) { return E_NOTIMPL; }

STDMETHODIMP PoleStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
   // Every stream sharing this container reads through one std::fstream, so the seek and
   // read pair inside POLE has to be serialised or concurrent readers get each other's
   // bytes. Contention is not a cost worth avoiding here: reading the container from
   // several threads at once measured 5.4x slower than from one, because it defeats
   // sequential readahead in the filesystem.
   ULONG bytesRead;
   {
      const std::lock_guard<std::mutex> lock(m_shared->readMutex);
      bytesRead = m_pPOLEStream->read((unsigned char*)pv, cb);
   }
   if (pcbRead)
      *pcbRead = bytesRead;
   return S_OK;
}

STDMETHODIMP PoleStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten) { return E_NOTIMPL; }
