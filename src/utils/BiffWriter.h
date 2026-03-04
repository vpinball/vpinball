// license:GPLv3+

#pragma once

#include "fileio.h"

class BiffWriter final :
   public IObjectWriter
{
public:
   BiffWriter(IStream *pistream, const HCRYPTHASH hcrypthash);
   bool HasError() const override { return m_hasError; }

   void BeginObject(int objectId, bool isArray) override;
   void WriteBool(int id, bool value) override;
   void WriteInt(int id, int value) override;
   void WriteUInt(int id, unsigned int value) override;
   void WriteFloat(int id, float value) override;
   void WriteString(int id, const string& value) override;
   void WriteWideString(int id, const wstring& value) override;
   void WriteVector2(int id, const Vertex2D& value) override;
   void WriteVector3(int id, const vec3& value) override;
   void WriteVector4(int id, const vec4& value) override;
   void WriteScript(int fieldId, const string &value) override;
   void WriteFontDescriptor(int fieldId, const FontDesc &value) override;
   void WriteRaw(const int id, const void* pvalue, const int size) override;
   void EndObject() override;

   void WriteBytes(const void *pv, const ULONG count);
   void WriteRecordSize(const int size);

   IStream *m_pistream;
   HCRYPTHASH m_hcrypthash;

private:
   bool m_hasError = false;
};
