// license:GPLv3+

#pragma once

#include "fileio.h"
#include "TableHash.h"

class BiffWriter final :
   public IObjectWriter
{
public:
   BiffWriter(InMemStream* stream, TableHash* const hash);
   bool HasError() const override { return m_hasError; }

   void BeginObject(int objectId, bool isArray, bool isSkippable) override;
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

   void WriteBytes(const void* pv, const size_t count);
   void WriteRecordSize(const int size);

   InMemStream* m_stream;
   TableHash* m_hash; // null to write without contributing to the table hash

private:
   bool m_hasError = false;
   vector<int64_t> m_subObjectRecordSizePos;
};
