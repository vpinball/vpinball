// license:GPLv3+

#pragma once

#include "md2.h"

// The MD2 stored in a table file's 'GameStg/MAC' stream. It covers the version, the
// TableInfo streams, the custom info tags, GameData and the collections - not the game
// items, sounds, images or fonts, which are written with a null hash.
//
// The digest always comes from our own portable MD2, so loading and saving agree on
// every target. Where the Windows CryptoAPI is available it is run alongside over the
// exact same bytes and Finish() compares the two, which makes every load and save a
// differential test of the implementation this replaced, on real tables, rather than a
// one-off check. A mismatch is not recoverable - we cannot tell which digest is right,
// so we cannot say whether the file is intact - hence Finish() reports it to the user,
// asserts (debug builds only), and returns false so the caller aborts.
//
// Update() stands in one for one for CryptHashData(): pass a null TableHash* where a
// null HCRYPTHASH used to mean "do not hash this".
//
// Not thread safe, and MD2 is order dependent, so a single TableHash must not be fed
// from concurrent readers - see the load thread pool in PinTable::LoadGameFromFilename
class TableHash final
{
public:
   TableHash();
   ~TableHash();

   // A recording hash for backwards compatible loading digests nothing. It only keeps the bytes handed to Update() so they
   // can be replayed into a real hash afterwards, in an order the reader cannot produce itself: up to file version 1000 the game items are part of the digest and have to
   // reach it by index, but they are read concurrently and in storage order
   struct RecordOnly { };
   explicit TableHash(RecordOnly);
   void ReplayInto(TableHash &target) const;

   TableHash(const TableHash &) = delete;
   TableHash &operator=(const TableHash &) = delete;

   void Update(const void *data, size_t size);

   // Finalizes the digest. Returns false if the two implementations disagree or if the
   // self test failed, in which case the digest must not be written or compared. The
   // object must not be updated or finalized again
   [[nodiscard]] bool Finish(uint8_t (&digest)[MD2::DIGEST_SIZE]);

   // Convenience for the null-hash case, so call sites stay free of null checks
   static void Update(TableHash *const hash, const void *const data, const size_t size)
   {
      if (hash)
         hash->Update(data, size);
   }

   // Bytes handed to Update() so far, recorded ones included. Reported alongside a
   // digest mismatch, where it narrows the cause down to the stream that diverges
   size_t BytesHashed() const { return m_bytes; }

   // Runs the RFC 1319 test vectors. Without CryptoAPI there is nothing to cross check
   // against, so on those targets this is the only thing standing between a mistyped
   // S-box and silently corrupt files. Result is cached; safe to call often
   static bool SelfTest();

private:
   MD2 m_md2;
   bool m_failed = false; // sticky: once set, Finish() can no longer vouch for the digest
   size_t m_bytes = 0;
   bool m_recording = false;
   vector<uint8_t> m_recorded; // only used while recording

#ifdef VPX_HAS_CRYPTOAPI
   // Reference implementation, kept only to cross check m_md2. Null if unavailable, which costs us the cross check but not the digest
   HCRYPTPROV m_hcp = NULL;
   HCRYPTHASH m_hch = NULL;
#endif
};
