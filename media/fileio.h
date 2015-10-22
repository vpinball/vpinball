#pragma once
#define FID(A) *(int *)#A

void TitleFromFilename(char *szfilename, char *sztitle);
void ExtensionFromFilename(char *szfilename, char *szextension);
BOOL RawReadFromFile(char *szfilename, int *psize, char **pszout);
void PathFromFilename(const char * const szfilename, char *szpath);
void TitleAndPathFromFilename(const char * const szfilename, char *szpath);

class BiffReader;

class ILoadable
{
public:
   virtual BOOL LoadToken(int id, BiffReader *pbr) = 0;
};

class BiffWriter
{
public:
   BiffWriter(IStream *pistream, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   HRESULT WriteInt(int id, int value);
   HRESULT WriteString(int id, char *szvalue);
   HRESULT WriteWideString(int id, WCHAR *wzvalue);
   HRESULT WriteBool(int id, BOOL fvalue);
   HRESULT WriteFloat(int id, float value);
   HRESULT WriteStruct(int id, void *pvalue, int size);
   HRESULT WriteVector3(int id, Vertex3Ds* vec);
   HRESULT WriteVector3Padded(int id, Vertex3Ds* vec);
   HRESULT WriteTag(int id);

   HRESULT WriteBytes(const void *pv, unsigned long count, unsigned long *foo);

   HRESULT WriteRecordSize(int size);

   IStream *m_pistream;
   HCRYPTHASH m_hcrypthash;
   HCRYPTKEY m_hcryptkey;
};

class BiffReader
{
public:
   BiffReader(IStream *pistream, ILoadable *piloadable, void *ppassdata, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);

   HRESULT GetIntNoHash(void *pvalue);
   HRESULT GetInt(void *pvalue);
   HRESULT GetString(char *szvalue);
   HRESULT GetWideString(WCHAR *wzvalue);
   HRESULT GetFloat(float *pvalue);
   HRESULT GetBool(BOOL *pfvalue);
   HRESULT GetBool(bool *pvalue);
   HRESULT GetStruct(void *pvalue, int size);
   HRESULT GetVector3(Vertex3Ds* vec);
   HRESULT GetVector3Padded(Vertex3Ds* vec);

   HRESULT ReadBytes(void *pv, unsigned long count, unsigned long *foo);

   HRESULT Load();

   IStream *m_pistream;
   ILoadable *m_piloadable;
   void *m_pdata;
   int m_version;

   int m_bytesinrecordremaining;

   HCRYPTHASH m_hcrypthash;
   HCRYPTKEY m_hcryptkey;
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

   int m_cref;
   Vector<FastIStorage> m_vstg;
   Vector<FastIStream> m_vstm;

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

   void SetSize(unsigned int i);

   int m_cref;

   unsigned int		m_cMax;		// Number of elements allocated
   unsigned int		m_cSeek;	// Last element used
   unsigned int		m_cSize;	// Size of stream
   char	*m_rg;		// Data buffer

   WCHAR *m_wzName;
};
