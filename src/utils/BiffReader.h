// license:GPLv3+

#pragma once

#include "fileio.h"

#include "pole/pole.h"


class BiffReader final :
   public IObjectReader
{
public:
   BiffReader(POLE::Stream *stream, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey);
   BiffReader(const uint8_t *data, const uint32_t size, const int version, const HCRYPTHASH hcrypthash, const HCRYPTKEY hcryptkey);
   int GetVersion() const override { return m_version; }
   bool HasError() const override { return m_hasError; }

   bool AsBool() override;
   int AsInt() override;
   unsigned int AsUInt() override;
   float AsFloat() override;
   string AsString() override;
   wstring AsWideString() override;
   Vertex2D AsVector2() override;
   vec3 AsVector3() override;
   vec4 AsVector4() override;
   string AsScript(bool isScriptProtected) override;
   FontDesc AsFontDescriptor() override;
   void AsRaw(void *pvalue, const int size) override;
   void AsObject(const std::function<bool(const int tag, IObjectReader &reader)> &processToken, bool isSkippable = false) override;

   void ReadBytes(void *const pv, const uint32_t count);

   POLE::Stream *m_stream = nullptr;
   HCRYPTHASH m_hcrypthash;
   HCRYPTKEY m_hcryptkey;

private:
   uint64_t ReadSource(unsigned char *pv, const uint32_t count);
   int GetIntNoHash();

   const uint8_t *m_data = nullptr;
   uint32_t m_dataSize = 0;
   uint32_t m_dataPos = 0;

   const int m_version;
   int m_bytesinrecordremaining = 0;
   bool m_hasError = false;
};
