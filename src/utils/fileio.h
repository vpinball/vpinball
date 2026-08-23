// license:GPLv3+

#pragma once
#define FID(A) (int)((unsigned int)(#A[0])|((unsigned int)(#A[1])<<8)|((unsigned int)(#A[2])<<16)|((unsigned int)(#A[3])<<24))

struct FontDesc
{
   uint8_t version = 1;
   uint16_t charset = 0;
   uint8_t attributes = 0;
   uint16_t weight = 0;
   uint32_t size = 0;
   string name;

   bool IsBold() const { return weight > 550; }
   bool IsItalic() const { return (attributes & 0x02) != 0; }
   bool IsUnderline() const { return (attributes & 0x04) != 0; }
   bool IsStrikeThrough() const { return (attributes & 0x08) != 0; }

#ifndef __STANDALONE__
   FONTDESC ToOLEFontDesc() const
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

   LOGFONT ToLogFont() const
   {
      LOGFONT lf = {};
      FONTDESC const oleFD = ToOLEFontDesc();
      IFont *pIFont = nullptr;
      OleCreateFontIndirect(const_cast<FONTDESC*>(&oleFD), IID_IFont, (void **)&pIFont);
      if (pIFont)
      {
         HFONT hFont;
         pIFont->get_hFont(&hFont);
         GetObject(hFont, sizeof(LOGFONT), &lf);
         pIFont->Release();
      }
      delete[] oleFD.lpstrName;
      return lf;
   }

   void FromOLEFont(IFont *pIFont)
   {
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);
      pIFont->get_Name((BSTR *)&fd.lpstrName);
      pIFont->get_Size(&fd.cySize);
      pIFont->get_Weight(&fd.sWeight);
      pIFont->get_Charset(&fd.sCharset);
      pIFont->get_Italic(&fd.fItalic);
      pIFont->get_Underline(&fd.fUnderline);
      pIFont->get_Strikethrough(&fd.fStrikethrough);
      name = MakeString((BSTR)fd.lpstrName);
      size = fd.cySize.Lo;
      weight = fd.sWeight;
      charset = fd.sCharset;
      attributes = (fd.fItalic ? 0x02 : 0x00) | (fd.fUnderline ? 0x04 : 0x00) | (fd.fStrikethrough ? 0x08 : 0x00);
      SysFreeString((BSTR)fd.lpstrName);
   }
#endif
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
   virtual vec3 AsVector3() = 0;
   virtual vec4 AsVector4() = 0;
   virtual string AsScript(bool isScriptProtected) = 0;
   virtual FontDesc AsFontDescriptor() = 0;
   virtual void AsRaw(void *pvalue, const int size) = 0;
   virtual void AsObject(const std::function<bool(const int fieldTag, IObjectReader &fieldReader)> &processField, bool isSkippable = false) = 0;
};

class IObjectWriter
{
public:
   virtual ~IObjectWriter() = default;
   virtual bool HasError() const = 0;

   virtual void BeginObject(int objectId, bool isArray, bool isSkippable) = 0;
   virtual void WriteBool(int fieldId, bool value) = 0;
   virtual void WriteInt(int fieldId, int value) = 0;
   virtual void WriteUInt(int fieldId, unsigned int value) = 0;
   virtual void WriteFloat(int fieldId, float value) = 0;
   virtual void WriteString(int fieldId, const string& value) = 0;
   virtual void WriteWideString(int fieldId, const wstring& value) = 0;
   virtual void WriteVector2(int fieldId, const Vertex2D& value) = 0;
   virtual void WriteVector3(int fieldId, const vec3& value) = 0;
   virtual void WriteVector4(int fieldId, const vec4& value) = 0;
   virtual void WriteScript(int fieldId, const string &value) = 0;
   virtual void WriteFontDescriptor(int fieldId, const FontDesc& value) = 0;
   virtual void WriteRaw(int fieldId, const void* pvalue, const int size) = 0;
   virtual void EndObject() = 0;
};





// Read-only IStream over a buffer owned by the caller.
//
// Table loading reads each stream on one thread, in the order the streams are physically laid
// out, and hands the bytes to the worker pool to parse. The workers need an IStream to give
// BiffReader, but they must not touch the container: reading it from several threads at once
// measured 3.4x slower than from one, because concurrent readers at scattered offsets defeat
// readahead in the filesystem.
//
// Intended to be stack allocated for the duration of a parse. AddRef and Release only count,
// they never delete, so lifetime stays with whoever owns the buffer. Only Read, Seek and Stat
// are implemented, which is all BiffReader and VPX::Sound::CreateFromStream use.
class MemoryIStream final : public IStream
{
public:
   MemoryIStream(const uint8_t *const data, const size_t size)
      : m_data(data)
      , m_size(size)
   { }

   HRESULT __stdcall QueryInterface(const struct _GUID &, void **) override { return E_NOTIMPL; }
   ULONG __stdcall AddRef() override { return ++m_cref; }
   ULONG __stdcall Release() override { return --m_cref; }

   HRESULT __stdcall Read(void *pv, ULONG count, ULONG *pcbRead) override;
   HRESULT __stdcall Seek(union _LARGE_INTEGER move, ULONG origin, union _ULARGE_INTEGER *pNewPos) override;
   HRESULT __stdcall Stat(struct tagSTATSTG *pstatstg, ULONG) override;

   HRESULT __stdcall Write(const void *, ULONG, ULONG *) override { return E_NOTIMPL; }
   HRESULT __stdcall SetSize(union _ULARGE_INTEGER) override { return E_NOTIMPL; }
   HRESULT __stdcall CopyTo(struct IStream *, union _ULARGE_INTEGER, union _ULARGE_INTEGER *, union _ULARGE_INTEGER *) override { return E_NOTIMPL; }
   HRESULT __stdcall Commit(ULONG) override { return E_NOTIMPL; }
   HRESULT __stdcall Revert() override { return E_NOTIMPL; }
   HRESULT __stdcall LockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) override { return E_NOTIMPL; }
   HRESULT __stdcall UnlockRegion(union _ULARGE_INTEGER, union _ULARGE_INTEGER, ULONG) override { return E_NOTIMPL; }
   HRESULT __stdcall Clone(struct IStream **) override { return E_NOTIMPL; }

private:
   const uint8_t *const m_data;
   const size_t m_size;
   size_t m_pos = 0;
   ULONG m_cref = 0;
};

// Physical position of each named stream within its container, for callers that want to read
// streams in layout order rather than declaration order.
//
// Returns false when the storage cannot report positions, which is the case for Windows OLE
// structured storage: IStorage exposes nothing equivalent. Callers must therefore treat the
// ordering as a hint and keep working without it. This is a capability query rather than a
// platform check so that the code path stays single, and so that a future move to POLE on
// Windows starts reporting positions with no caller changes.
//
// Offsets are absolute byte positions and are comparable across every stream in the
// container, including those small enough to be packed inside the mini-stream container.
bool GetStreamOffsets(IStorage *pstg, const vector<wstring> &names, vector<uint64_t> &offsets);

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

   char *m_rg; // Data buffer
   WCHAR *m_wzName;
   unsigned int m_cSize; // Size of stream

private:
   void SetSize(const unsigned int i);

   int m_cref;

   unsigned int m_cMax; // Number of elements allocated
   unsigned int m_cSeek; // Last element used
};


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

