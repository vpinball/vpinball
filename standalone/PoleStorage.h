#pragma once

#include "objidl.h"
#include "pole/pole.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
using std::string;

// One parsed container, shared by every storage and stream opened from it.
//
// Opening a stream used to clone the storage, which re-parsed the whole container: header,
// FAT, mini FAT and directory tree. A table with 1860 streams therefore parsed it 1860
// times. The clone existed to give each thread its own file handle, since POLE reads
// through a single std::fstream and interleaved seek and read pairs would return the wrong
// bytes. Sharing the parse means sharing that handle, so reads are serialised here instead.
//
// The rest of the read path needs no lock. DirTree::entry with create false only mutates
// under create && writeable, AllocTable::follow is const, and the readahead window lives on
// StreamIO, one per open stream.
struct PoleSharedStorage final
{
   explicit PoleSharedStorage(POLE::Storage* pStorage) : storage(pStorage) { }
   ~PoleSharedStorage() { delete storage; }

   PoleSharedStorage(const PoleSharedStorage&) = delete;
   PoleSharedStorage& operator=(const PoleSharedStorage&) = delete;

   POLE::Storage* const storage;
   std::mutex readMutex;
};

class PoleStorage : public IStorage {
public:
   static HRESULT Create(const string& szFilename, const string& szName, IStorage** ppstg);
   // Another handle onto an already parsed container, rooted at szName.
   static PoleStorage* Attach(const std::shared_ptr<PoleSharedStorage>& shared, const string& szName);

   HRESULT StreamExists(const string& szName);
   const std::shared_ptr<PoleSharedStorage>& getShared() const { return m_shared; }
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
  std::shared_ptr<PoleSharedStorage> m_shared;
  string m_szPath;

  // Streams are opened and released from the table load thread pool, so a plain counter
  // would race even though each stream itself is only touched by one thread.
  std::atomic<ULONG> m_dwRef { 0 };
};
