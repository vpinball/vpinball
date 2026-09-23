// license:GPLv3+

#include "core/stdafx.h"

#include "TableHash.h"

#include "core/def.h"

#include <limits>

namespace
{

// RFC 1319, appendix A.5. Keep all seven: the first four pad to a single block, where
// the reference checksum step and the (wrong) literal reading of the RFC prose happen to
// agree. Only the last three span several blocks and tell the two apart, so a trimmed
// vector set would pass while the digest was silently wrong for every real table
struct MD2Vector { const char *msg; const char *digest; };
constexpr MD2Vector MD2_VECTORS[] = {
   { "", "8350e5a3e24c153df2275c9f80692773" },
   { "a", "32ec01ec4a6dac72c0ab96fb34c0b5d1" },
   { "abc", "da853b0d3f88d99b30283a69e6ded6bb" },
   { "message digest", "ab4f496bfb2a530b219ff33031fe06b0" },
   { "abcdefghijklmnopqrstuvwxyz", "4e8ddff3650292ab5a4108c3aa47940b" },
   { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "da33def2a42df13975352846c30338cd" },
   { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "d5976f79d83d3a0dc9806c3c66f3efd8" },
};

string ToHex(const uint8_t (&digest)[MD2::DIGEST_SIZE])
{
   constexpr char nibble[] = "0123456789abcdef";
   string hex;
   hex.reserve(MD2::DIGEST_SIZE * 2);
   for (const uint8_t b : digest)
   {
      hex += nibble[b >> 4];
      hex += nibble[b & 0x0F];
   }
   return hex;
}

// A wrong table hash means silent file corruption, so report it every way we have and
// let the caller turn it into a failed load/save. Log first so the record survives even
// if the UI is gone, and assert last so the message box is up before a debugger breaks
// (and because assert() is compiled out of release builds, where the log and the box
// are all the user gets).
void ReportHashFailure(const string &detail)
{
   PLOGE << "Table hash failure: " << detail;
   ShowError("Table hash failure, aborting to avoid corrupting the file.\n\n" + detail);
   assert(!"table hash mismatch, see the message box for details");
}

}

bool TableHash::SelfTest()
{
   static const bool ok = []
   {
      for (const MD2Vector &v : MD2_VECTORS)
      {
         MD2 md2;
         md2.Update(v.msg, strlen(v.msg));
         uint8_t digest[MD2::DIGEST_SIZE];
         md2.Finish(digest);
         if (ToHex(digest) != v.digest)
         {
            ReportHashFailure("MD2 self test failed on RFC 1319 vector \""s + v.msg + "\"\nexpected " + v.digest
               + "\ngot      " + ToHex(digest));
            return false;
         }
      }
      return true;
   }();
   return ok;
}

TableHash::TableHash()
{
   m_failed = !SelfTest();

#ifdef VPX_HAS_CRYPTOAPI
   // Kept alongside our own MD2 purely to cross check it on every real table. Same flags
   // the table code used before, so it acquires the same CSP it always did
   if (!CryptAcquireContext(&m_hcp, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET)
      || !CryptCreateHash(m_hcp, CALG_MD2, NULL, 0, &m_hch))
   {
      // Losing the reference costs us the cross check, not the digest. This is an
      // improvement on the old code, which hashed through the CSP and so quietly failed
      // validation outright on machines where acquiring it did not work
      PLOGW << "CryptoAPI MD2 unavailable (" << GetLastError() << "), table hash cross check disabled";
      m_hch = NULL;
   }
#endif
}

TableHash::~TableHash()
{
#ifdef VPX_HAS_CRYPTOAPI
   if (m_hch)
      CryptDestroyHash(m_hch);
   if (m_hcp)
      CryptReleaseContext(m_hcp, 0);
#endif
}

void TableHash::Update(const void *const data, const size_t size)
{
   m_md2.Update(data, size);

#ifdef VPX_HAS_CRYPTOAPI
   if (m_hch)
   {
      // Every hashed stream is a table property block or the script, orders of magnitude
      // below the DWORD the CryptoAPI takes, so the narrowing cannot bite in practice
      assert(size <= std::numeric_limits<DWORD>::max());
      CryptHashData(m_hch, static_cast<const BYTE *>(data), static_cast<DWORD>(size), 0);
   }
#endif
}

bool TableHash::Finish(uint8_t (&digest)[MD2::DIGEST_SIZE])
{
   m_md2.Finish(digest);

#ifdef VPX_HAS_CRYPTOAPI
   if (m_hch)
   {
      uint8_t reference[MD2::DIGEST_SIZE];
      DWORD len = sizeof(reference);
      if (CryptGetHashParam(m_hch, HP_HASHVAL, reference, &len, 0) && len == MD2::DIGEST_SIZE)
      {
         if (memcmp(digest, reference, MD2::DIGEST_SIZE) != 0)
         {
            ReportHashFailure("our MD2 disagrees with the Windows CryptoAPI over the same bytes.\n\nours      "
               + ToHex(digest) + "\nCryptoAPI " + ToHex(reference));
            m_failed = true;
         }
      }
      else
         PLOGW << "CryptoAPI hash readback failed (" << GetLastError() << "), table hash cross check skipped";
   }
#endif

   return !m_failed;
}
