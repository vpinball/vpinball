// license:GPLv3+

#pragma once

// MD2 message digest (RFC 1319), streaming. Single use and not thread safe: digest one
// message per object, from one thread.
//
// Table files carry an MD2 of parts of their contents in the 'GameStg/MAC' stream. That
// digest was historically produced by the Windows CryptoAPI (CALG_MD2), so this has to
// agree with it byte for byte or existing tables stop validating. Two details make that
// agreement exact rather than hopeful:
//
//  - The checksum step follows RFC 1319's *reference implementation*
//    ('checksum[i] ^= S[block[i] ^ t]'), not the RFC prose ('checksum[i] = S[...]').
//    The two agree for any message that pads to a single block and diverge from the
//    second block onwards, so the prose form passes short test vectors and then
//    silently produces wrong digests for real data. The XOR form is what reproduces
//    RFC 1319's own published vectors, and what CryptoAPI computes.
//
//  - MD2 has no length field, so digesting in arbitrary chunks is exactly equivalent to
//    digesting the concatenation. That is what lets Update() stand in for
//    CryptHashData() call for call.
//
// Not worth optimizing: the 18x48 round loop is a serial S-box feedback chain (each
// lookup is indexed by the previous result), so it cannot be vectorized, and only ~30KB
// per table is hashed anyway.
//
// MD2 is long broken as a cryptographic hash. It is used here only as a file format
// checksum, and cannot be changed without invalidating every existing table
class MD2 final
{
public:
   static constexpr unsigned int DIGEST_SIZE = 16;

   void Update(const void *data, size_t size);

   // Finalizes the digest. The object must not be updated or finalized again
   void Finish(uint8_t (&digest)[DIGEST_SIZE]);

private:
   void Transform(const uint8_t block[16]);

   uint8_t m_state[48] = {};     // running state in [0,16), scratch for Transform above that
   uint8_t m_checksum[16] = {};
   uint8_t m_block[16] = {};     // bytes of the next block received so far
   unsigned int m_blockFill = 0; // how many of them, always < 16 between Update() calls
};
