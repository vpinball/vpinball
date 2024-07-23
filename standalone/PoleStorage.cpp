#include "stdafx.h"
#include "PoleStorage.h"
#include "PoleStream.h"

HRESULT PoleStorage::Create(const string& szFilename, const string& szName, IStorage** ppstg)
{
   POLE::Storage* pPOLEStorage = new POLE::Storage(szFilename.c_str());

   if (pPOLEStorage->open() && pPOLEStorage->result() == POLE::Storage::Ok) {
      PoleStorage* pStg = new PoleStorage();
      pStg->m_szFilename = szFilename;
      pStg->m_szPath = szName;
      pStg->m_pPOLEStorage = pPOLEStorage;

      pStg->AddRef();

      *ppstg = pStg;

      return S_OK;
   }

   delete pPOLEStorage;

   return STG_E_FILENOTFOUND;
}

HRESULT PoleStorage::Clone(PoleStorage* pPoleStorage, IStorage** ppstg)
{
   return PoleStorage::Create(pPoleStorage->m_szFilename, pPoleStorage->m_szPath, ppstg);
}

HRESULT PoleStorage::StreamExists(const string& szName)
{
   return m_pPOLEStorage->exists(m_szPath + '/' + szName) ? S_OK : STG_E_INVALIDNAME;
}

POLE::Storage* PoleStorage::getPOLEStorage()
{
   return m_pPOLEStorage;
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
      delete m_pPOLEStorage;

      delete this;
   }

   return dwRef;
}

STDMETHODIMP PoleStorage::CreateStream(LPCOLESTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm) { return E_NOTIMPL; }

STDMETHODIMP PoleStorage::OpenStream(LPCOLESTR pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm)
{
   HRESULT hr;
   PoleStorage* pStg;

   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   if (SUCCEEDED(hr = PoleStorage::Clone(this, (IStorage**)&pStg))) {
      if (SUCCEEDED(hr = pStg->StreamExists(szName)))
         hr = PoleStream::Create(pStg, szName, ppstm);

      if (FAILED(hr))
         pStg->Release();
   }

   return hr;
}

STDMETHODIMP PoleStorage::CreateStorage(LPCOLESTR pwcsName, DWORD grfMode, DWORD dwStgFmt, DWORD reserved2, IStorage **ppstg) { return E_NOTIMPL; }

STDMETHODIMP PoleStorage::OpenStorage(LPCOLESTR pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   return Create(m_szFilename, szName, ppstg);
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
