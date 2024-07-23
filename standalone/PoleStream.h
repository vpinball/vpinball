#pragma once

#include "objidl.h"
#include "pole.h"

#include "PoleStorage.h"

class PoleStream : public IStream {
public:
   static HRESULT Create(PoleStorage* pStorage, const string& szName, IStream** ppstm);

   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
   ULONG STDMETHODCALLTYPE AddRef();
   ULONG STDMETHODCALLTYPE Release();

   STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
   STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
   STDMETHOD(CopyTo)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
   STDMETHOD(Commit)(DWORD grfCommitFlags);
   STDMETHOD(Revert)();
   STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
   STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
   STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
   STDMETHOD(Clone)(IStream **ppstm);
   STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
   STDMETHOD(Write)(const void *pv, ULONG cb, ULONG *pcbWritten);

private:
   PoleStorage* m_pStorage = nullptr;
   POLE::Stream* m_pPOLEStream = nullptr;

   ULONG m_dwRef = 0;
};
