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


namespace POLE
{
class Storage;
}

// Simple in-memory byte stream
class InMemStream final
{
public:
   void Write(const void *pv, size_t count);
   void Seek(uint64_t pos) { m_cSeek = static_cast<size_t>(pos); }
   uint64_t Tell() const { return m_cSeek; }

   const uint8_t *Data() const { return m_data.data(); }
   size_t Size() const { return m_data.size(); }

private:
   vector<uint8_t> m_data; // Stream content
   size_t m_cSeek = 0; // Current position
};


// Simple in-memory structured storage: an ordered set of named streams that can be flushed to a POLE structured storage file.
class InMemStructuredStorage final
{
public:
   ~InMemStructuredStorage();

   // Creates a new empty stream, name being its full path (e.g. "GameStg/GameData")
   InMemStream *CreateStream(const string &name);

   // Writes all the streams to a POLE storage opened for writing
   bool WriteToStorage(POLE::Storage &storage) const;

private:
   vector<std::pair<string, InMemStream *>> m_streams;
};

