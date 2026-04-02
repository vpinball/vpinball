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

