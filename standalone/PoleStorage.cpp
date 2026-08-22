#include "core/stdafx.h"
#include "PoleStorage.h"
#include "PoleStream.h"

HRESULT PoleStorage::Create(const string& szFilename, const string& szName, IStorage** ppstg)
{
   POLE::Storage* pPOLEStorage = new POLE::Storage(szFilename.c_str());

   if (pPOLEStorage->open() && pPOLEStorage->result() == POLE::Storage::Ok) {
      auto shared = std::make_shared<PoleSharedStorage>(pPOLEStorage);

      *ppstg = Attach(shared, szName);

      return S_OK;
   }

   delete pPOLEStorage;

   return STG_E_FILENOTFOUND;
}

PoleStorage* PoleStorage::Attach(const std::shared_ptr<PoleSharedStorage>& shared, const string& szName)
{
   PoleStorage* pStg = new PoleStorage();
   pStg->m_shared = shared;
   pStg->m_szPath = szName;

   pStg->AddRef();

   return pStg;
}

HRESULT PoleStorage::StreamExists(const string& szName)
{
   return m_shared->storage->exists(m_szPath + '/' + szName) ? S_OK : STG_E_INVALIDNAME;
}

POLE::Storage* PoleStorage::getPOLEStorage()
{
   return m_shared->storage;
}

string PoleStorage::getPath()
{
   return m_szPath;
}

STDMETHODIMP PoleStorage::QueryInterface(REFIID iid, void **ppvObjOut) { return E_NOTIMPL; }

STDMETHODIMP_(ULONG) PoleStorage::AddRef()
{
   m_dwRef++;

   return m_dwRef;
}

STDMETHODIMP_(ULONG) PoleStorage::Release()
{
   ULONG dwRef = --m_dwRef;

   if (dwRef == 0) {
      // The parsed container outlives this handle if any stream still holds it.
      delete this;
   }

   return dwRef;
}

STDMETHODIMP PoleStorage::CreateStream(LPCOLESTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm) { return E_NOTIMPL; }

STDMETHODIMP PoleStorage::OpenStream(LPCOLESTR pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   const HRESULT hr = StreamExists(szName);
   if (FAILED(hr))
      return hr;

   return PoleStream::Create(this, szName, ppstm);
}

STDMETHODIMP PoleStorage::CreateStorage(LPCOLESTR pwcsName, DWORD grfMode, DWORD dwStgFmt, DWORD reserved2, IStorage **ppstg) { return E_NOTIMPL; }

STDMETHODIMP PoleStorage::OpenStorage(LPCOLESTR pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   *ppstg = Attach(m_shared, szName);

   return S_OK;
}

STDMETHODIMP PoleStorage::CopyTo(DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::MoveElementTo(LPCOLESTR pwcsName, IStorage *pstgDest, LPCOLESTR pwcsNewName, DWORD grfFlags) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::Commit(DWORD grfCommitFlags) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::Revert() { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::EnumElements(DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::DestroyElement(LPCOLESTR pwcsName) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::RenameElement(LPCOLESTR pwcsOldName, LPCOLESTR pwcsNewName) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::SetElementTimes(LPCOLESTR pwcsName, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::SetClass(REFCLSID clsid) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask) { return E_NOTIMPL; }
STDMETHODIMP PoleStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag) { return E_NOTIMPL; }
