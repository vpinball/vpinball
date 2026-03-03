// license:GPLv3+

#pragma once
#define FID(A) (int)((unsigned int)(#A[0])|((unsigned int)(#A[1])<<8)|((unsigned int)(#A[2])<<16)|((unsigned int)(#A[3])<<24))

bool DirExists(const std::filesystem::path& dirPath);
bool FileExists(const std::filesystem::path& filePath);
string TitleFromFilename(const std::filesystem::path& filename);
std::filesystem::path PathFromFilename(const std::filesystem::path& filename);

class BiffWriter final
{
public:
   BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash);

   HRESULT WriteInt(const int id, const int value);
   HRESULT WriteString(const int id, const string& szvalue);

   HRESULT WriteWideString(const int id, const WCHAR * const wzvalue);
   HRESULT WriteWideString(const int id, const std::basic_string<WCHAR>& wzvalue);
   HRESULT WriteBool(const int id, const BOOL value);
   HRESULT WriteFloat(const int id, const float value);
   HRESULT WriteStruct(const int id, const void * const pvalue, const int size);
   HRESULT WriteVector2(const int id, const Vertex2D& vec);
   HRESULT WriteVector3(const int id, const Vertex3Ds& vec);
   HRESULT WriteVector3Padded(const int id, const Vertex3Ds& vec);
   HRESULT WriteTag(const int id);

   HRESULT WriteBytes(const void *pv, const ULONG count, ULONG *foo);

   HRESULT WriteRecordSize(const int size);

   IStream *m_pistream;
   HCRYPTHASH m_hcrypthash;
};


class IObjectReader
{
public:
   virtual ~IObjectReader() = default;
   virtual int GetVersion() const = 0;
   virtual bool HasError() const = 0;

   virtual bool AsBool() = 0;
   virtual int AsInt() = 0;
   virtual unsigned int AsUInt() = 0;
   virtual float AsFloat() = 0;
   virtual string AsString() = 0;
   virtual wstring AsWideString() = 0;
   virtual Vertex2D AsVector2() = 0;
   virtual Vertex3Ds AsVector3(bool isPadded = false) = 0;
   virtual string AsScript(bool isScriptProtected) = 0;
   struct FontDesc
   {
      uint8_t version;
      uint16_t charset;
      uint8_t attributes;
      uint16_t weight;
      uint32_t size;
      string name;
      #ifndef __STANDALONE__
      FONTDESC ToOLEFontDesc()
      {
         FONTDESC fd;
         fd.cbSizeofstruct = sizeof(FONTDESC);
         fd.lpstrName = MakeWide(name);
         fd.cySize.Hi = 0;
         fd.cySize.Lo = size;
         fd.sWeight = weight;
         fd.sCharset = charset;
         fd.fItalic = (attributes & 0x02) ? TRUE : FALSE;
         fd.fUnderline = (attributes & 0x04) ? TRUE : FALSE;
         fd.fStrikethrough = (attributes & 0x08) ? TRUE : FALSE;
         return fd;
      }
      #endif
   };
   virtual FontDesc AsFontDescriptor() = 0;
   virtual void AsRaw(void *pvalue, const int size) = 0;
   virtual void AsObject(const std::function<bool(const int fieldTag, IObjectReader &fieldReader)> &processField = nullptr) = 0;

   // TODO remove these methods as they do not conform to the object reading design (direct byte acces instead of typed fields)
   virtual int GetBytesInRecordRemaining() const = 0;
};


class BiffReader final :
   public IObjectReader
{
public:
   BiffReader(IStream *pistream, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey);
   int GetVersion() const override { return m_version; }
   bool HasError() const override { return m_hasError; }

   bool AsBool() override;
   int AsInt() override;
   unsigned int AsUInt() override;
   float AsFloat() override;
   string AsString() override;
   wstring AsWideString() override;
   Vertex2D AsVector2() override;
   Vertex3Ds AsVector3(bool isPadded = false) override;
   string AsScript(bool isScriptProtected) override;
   FontDesc AsFontDescriptor() override;
   void AsRaw(void *pvalue, const int size) override;
   void AsObject(const std::function<bool(const int tag, IObjectReader &reader)> &processToken) override;

   int GetBytesInRecordRemaining() const override { return m_bytesinrecordremaining; }

   void ReadBytes(void *const pv, const uint32_t count);

   IStream *m_pistream;
   HCRYPTHASH m_hcrypthash;
   HCRYPTKEY m_hcryptkey;

private:
   int GetIntNoHash();

   const int m_version;
   int m_bytesinrecordremaining = 0;
   bool m_hasError = false;
};



class FastIStream;

class FastIStorage : public IStorage
{
public:
   FastIStorage();
   virtual ~FastIStorage();

   HRESULT __stdcall QueryInterface(const struct _GUID &, void **) override { return S_OK; }
   ULONG __stdcall AddRef() override;
   ULONG __stdcall Release() override;

   HRESULT __stdcall CreateStream(const WCHAR *, ULONG, ULONG, ULONG, struct IStream **) override;
   HRESULT __stdcall OpenStream(const WCHAR *, void *, ULONG, ULONG, struct IStream **) override { return S_OK; }
   HRESULT __stdcall CreateStorage(const WCHAR *, ULONG, ULONG, ULONG, struct IStorage **) override;
   HRESULT __stdcall OpenStorage(const WCHAR *, struct IStorage *, ULONG, WCHAR **, ULONG, struct IStorage **) override { return S_OK; }
   HRESULT __stdcall CopyTo(ULONG, const struct _GUID *, WCHAR **, struct IStorage *) override;
   HRESULT __stdcall MoveElementTo(const WCHAR *, struct IStorage *, const WCHAR *, ULONG) override { return S_OK; }
   HRESULT __stdcall Commit(ULONG) override { return S_OK; }
   HRESULT __stdcall Revert() override { return S_OK; }
   HRESULT __stdcall EnumElements(ULONG, void *, ULONG, struct IEnumSTATSTG **) override { return S_OK; }
   HRESULT __stdcall DestroyElement(const WCHAR *) override { return S_OK; }
   HRESULT __stdcall RenameElement(const WCHAR *, const WCHAR *) override { return S_OK; }
   HRESULT __stdcall SetElementTimes(const WCHAR *, const struct _FILETIME *, const struct _FILETIME *, const struct _FILETIME *) override { return S_OK; }
   HRESULT __stdcall SetClass(const struct _GUID &) override { return S_OK; }
   HRESULT __stdcall SetStateBits(ULONG, ULONG) override { return S_OK; }
   HRESULT __stdcall Stat(struct tagSTATSTG *, ULONG) override { return S_OK; }

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

   HRESULT __stdcall QueryInterface(const struct _GUID &, void **) override { return S_OK; }
   ULONG __stdcall AddRef() override;
   ULONG __stdcall Release() override;
   HRESULT __stdcall Read(void *pv, ULONG count, ULONG *foo) override;
   HRESULT __stdcall Write(const void *pv, ULONG count, ULONG *foo) override;
   HRESULT __stdcall Seek(union _LARGE_INTEGER, ULONG, union _ULARGE_INTEGER *) override;
   HRESULT __stdcall SetSize(union _ULARGE_INTEGER) override { return S_OK; }
   HRESULT __stdcall CopyTo(struct IStream *, union _ULARGE_INTEGER, union _ULARGE_INTEGER *, union _ULARGE_INTEGER *) override { return S_OK; }
   HRESULT __stdcall Commit(ULONG) override { return S_OK; }
   HRESULT __stdcall Revert() override { return S_OK; }

   HRESULT __stdcall LockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) override { return S_OK; }
   HRESULT __stdcall UnlockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) override { return S_OK; }
   HRESULT __stdcall Stat(struct tagSTATSTG *, ULONG) override { return S_OK; }
   HRESULT __stdcall Clone(struct IStream **) override { return S_OK; }

   char  *m_rg;          // Data buffer
   WCHAR *m_wzName;
   unsigned int m_cSize; // Size of stream

private:
   void SetSize(const unsigned int i);

   int m_cref;

   unsigned int m_cMax;  // Number of elements allocated
   unsigned int m_cSeek; // Last element used
};
