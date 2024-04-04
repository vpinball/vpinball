#pragma once

#include "objidl.h"
#include "pole.h"

#include <string>
using std::string;

class PoleStorage : public IStorage {
public:
   static HRESULT Create(const string& szFilename, const string& szName, IStorage** ppstg);
   static HRESULT Clone(PoleStorage* pPoleStorage, IStorage** ppstg);

   HRESULT StreamExists(const string& szName);
   POLE::Storage* getPOLEStorage();
   string getPath();

   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
   ULONG STDMETHODCALLTYPE AddRef();
   ULONG STDMETHODCALLTYPE Release();

   STDMETHOD(CreateStream)(LPCOLESTR pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm);
   STDMETHOD(OpenStream)(LPCOLESTR pwcsName, void *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm);
   STDMETHOD(CreateStorage)(LPCOLESTR pwcsName, DWORD grfMode, DWORD dwStgFmt, DWORD reserved2, IStorage **ppstg);
   STDMETHOD(OpenStorage)(LPCOLESTR pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg);
   STDMETHOD(CopyTo)(DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest);
   STDMETHOD(MoveElementTo)(LPCOLESTR pwcsName, IStorage *pstgDest, LPCOLESTR pwcsNewName, DWORD grfFlags);
   STDMETHOD(Commit)(DWORD grfCommitFlags);
   STDMETHOD(Revert)();
   STDMETHOD(EnumElements)(DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum);
   STDMETHOD(DestroyElement)(LPCOLESTR pwcsName);
   STDMETHOD(RenameElement)(LPCOLESTR pwcsOldName, LPCOLESTR pwcsNewName);
   STDMETHOD(SetElementTimes)(LPCOLESTR pwcsName, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime);
   STDMETHOD(SetClass)(REFCLSID clsid);
   STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
   STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

private:
  POLE::Storage* m_pPOLEStorage = nullptr;
  string m_szFilename;
  string m_szPath;

  ULONG m_dwRef = 0;
};
