#pragma once
#define FID(A) (int)((unsigned int)(#A[0])|((unsigned int)(#A[1])<<8)|((unsigned int)(#A[2])<<16)|((unsigned int)(#A[3])<<24))

bool Exists(const string& filePath);
void TitleFromFilename(const string& szfilename, string& sztitle);
void ExtensionFromFilename(const string& szfilename, string& szextension);
bool RawReadFromFile(const char * const szfilename, int *psize, char **pszout);
void PathFromFilename(const string& szfilename, string& szpath);
void TitleAndPathFromFilename(const char * const szfilename, char *szpath);
bool ReplaceExtensionFromFilename(string& szfilename, const string& newextension);

class BiffReader;

class ILoadable
{
public:
   virtual bool LoadToken(const int id, BiffReader * const pbr) = 0;
};

class BiffWriter
{
public:
   BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash);

   HRESULT WriteInt(const int id, const int value);
   HRESULT WriteString(const int id, const char * const szvalue);
   HRESULT WriteString(const int id, const std::string& szvalue);

   HRESULT WriteWideString(const int id, const WCHAR * const wzvalue);
   HRESULT WriteWideString(const int id, const std::basic_string<WCHAR>& wzvalue);
   HRESULT WriteBool(const int id, const BOOL value);
   HRESULT WriteFloat(const int id, const float value);
   HRESULT WriteStruct(const int id, const void *pvalue, const int size);
   HRESULT WriteVector3(const int id, const Vertex3Ds* vec);
   HRESULT WriteVector3Padded(const int id, const Vertex3Ds* vec);
   HRESULT WriteTag(const int id);

   HRESULT WriteBytes(const void *pv, const unsigned long count, unsigned long *foo);

   HRESULT WriteRecordSize(const int size);

   IStream *m_pistream;
   HCRYPTHASH m_hcrypthash;
};

class BiffReader
{
public:
   BiffReader(IStream *pistream, ILoadable *piloadable, void *ppassdata, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey);

   HRESULT GetIntNoHash(void *pvalue);
   HRESULT GetInt(void *pvalue);
   HRESULT GetString(char *szvalue);
   HRESULT GetString(std::string& szvalue);
   HRESULT GetWideString(WCHAR* wzvalue);
   HRESULT GetWideString(std::basic_string<WCHAR>& wzvalue);
   HRESULT GetFloat(float *pvalue);
   HRESULT GetBool(BOOL *pfvalue);
   HRESULT GetBool(bool *pvalue);
   HRESULT GetStruct(void *pvalue, const int size);
   HRESULT GetVector3(Vertex3Ds* vec);
   HRESULT GetVector3Padded(Vertex3Ds* vec);

   HRESULT ReadBytes(void *pv, const unsigned long count, unsigned long *foo);

   HRESULT Load();

   IStream *m_pistream;
   void *m_pdata;
   int m_version;

   HCRYPTHASH m_hcrypthash;
   HCRYPTKEY m_hcryptkey;

private:
   ILoadable *m_piloadable;
   int m_bytesinrecordremaining;
};

class FastIStream;

class FastIStorage : public IStorage
{
public:
   FastIStorage();
   virtual ~FastIStorage();

   long __stdcall QueryInterface(const struct _GUID &, void **);
   unsigned long __stdcall AddRef();
   unsigned long __stdcall Release();

   long __stdcall CreateStream(const WCHAR *, unsigned long, unsigned long, unsigned long, struct IStream **);
   long __stdcall OpenStream(const WCHAR *, void *, unsigned long, unsigned long, struct IStream **);
   long __stdcall CreateStorage(const WCHAR *, unsigned long, unsigned long, unsigned long, struct IStorage **);
   long __stdcall OpenStorage(const WCHAR *, struct IStorage *, unsigned long, WCHAR **, unsigned long, struct IStorage **);
   long __stdcall CopyTo(unsigned long, const struct _GUID *, WCHAR **, struct IStorage *);
   long __stdcall MoveElementTo(const WCHAR *, struct IStorage *, const WCHAR *, unsigned long);
   long __stdcall Commit(unsigned long);
   long __stdcall Revert();
   long __stdcall EnumElements(unsigned long, void *, unsigned long, struct IEnumSTATSTG **);
   long __stdcall DestroyElement(const WCHAR *);
   long __stdcall RenameElement(const WCHAR *, const WCHAR *);
   long __stdcall SetElementTimes(const WCHAR *, const struct _FILETIME *, const struct _FILETIME *, const struct _FILETIME *);
   long __stdcall SetClass(const struct _GUID &);
   long __stdcall SetStateBits(unsigned long, unsigned long);
   long __stdcall Stat(struct tagSTATSTG *, unsigned long);

private:
   int m_cref;

   vector<FastIStorage*> m_vstg;
   vector<FastIStream*> m_vstm;

   WCHAR *m_wzName;
};

class FastIStream : public IStream
{
public:
   FastIStream();
   virtual ~FastIStream();

   long __stdcall QueryInterface(const struct _GUID &, void **);
   unsigned long __stdcall AddRef();
   unsigned long __stdcall Release();
   long __stdcall Read(void *pv, unsigned long count, unsigned long *foo);
   long __stdcall Write(const void *pv, unsigned long count, unsigned long *foo);
   long __stdcall Seek(union _LARGE_INTEGER, unsigned long, union _ULARGE_INTEGER *);
   long __stdcall SetSize(union _ULARGE_INTEGER);
   long __stdcall CopyTo(struct IStream *, union _ULARGE_INTEGER, union _ULARGE_INTEGER *, union _ULARGE_INTEGER *);
   long __stdcall Commit(unsigned long);
   long __stdcall Revert();

   long __stdcall LockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, unsigned long);
   long __stdcall UnlockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, unsigned long);
   long __stdcall Stat(struct tagSTATSTG *, unsigned long);
   long __stdcall Clone(struct IStream **);

   char  *m_rg;          // Data buffer
   WCHAR *m_wzName;
   unsigned int m_cSize; // Size of stream

private:
   void SetSize(const unsigned int i);

   int m_cref;

   unsigned int m_cMax;  // Number of elements allocated
   unsigned int m_cSeek; // Last element used
};
