// license:GPLv3+

#include "core/stdafx.h"

#include "md2.h"

// PI_SUBST from RFC 1319: a permutation of 0..255 derived from the digits of pi
static constexpr uint8_t PI_SUBST[256] = {
    41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6, 19,
    98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188, 76, 130, 202,
    30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24, 138, 23, 229, 18,
    190, 78, 196, 214, 218, 158, 222, 73, 160, 251, 245, 142, 187, 47, 238, 122,
    169, 104, 121, 145, 21, 178, 7, 63, 148, 194, 16, 137, 11, 34, 95, 33,
    128, 127, 93, 154, 90, 144, 50, 39, 53, 62, 204, 231, 191, 247, 151, 3,
    255, 25, 48, 179, 72, 165, 181, 209, 215, 94, 146, 42, 172, 86, 170, 198,
    79, 184, 56, 210, 150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241,
    69, 157, 112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2,
    27, 96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
    85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197, 234, 38,
    44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65, 129, 77, 82,
    106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123, 8, 12, 189, 177, 74,
    120, 136, 149, 139, 227, 99, 232, 109, 233, 203, 213, 254, 59, 0, 29, 57,
    242, 239, 183, 14, 102, 88, 208, 228, 166, 119, 114, 248, 235, 117, 75, 10,
    49, 68, 80, 180, 143, 237, 31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

void MD2::Transform(const uint8_t block[16])
{
   // Encryption block: state | block | state^block
   memcpy(m_state + 16, block, 16);
   for (unsigned int i = 0; i < 16; i++)
      m_state[i + 32] = m_state[i] ^ block[i];

   // 18 rounds. Each lookup is indexed by the previous one's result, so this chain is
   // strictly serial: it does not vectorize, and reordering it would change the digest
   unsigned int t = 0;
   for (unsigned int i = 0; i < 18; i++)
   {
      for (unsigned int j = 0; j < 48; j++)
         t = m_state[j] ^= PI_SUBST[t];
      t = (t + i) & 0xFF;
   }

   // Checksum, seeded from the previous block through checksum[15]
   t = m_checksum[15];
   for (unsigned int i = 0; i < 16; i++)
      t = m_checksum[i] ^= PI_SUBST[block[i] ^ t];
}

void MD2::Update(const void *const data, const size_t size)
{
   const uint8_t *p = static_cast<const uint8_t *>(data);
   size_t left = size;

   // Top up a partially filled block first
   if (m_blockFill != 0)
   {
      const size_t take = std::min(left, static_cast<size_t>(16 - m_blockFill));
      memcpy(m_block + m_blockFill, p, take);
      m_blockFill += static_cast<unsigned int>(take);
      p += take;
      left -= take;
      if (m_blockFill < 16)
         return;
      Transform(m_block);
      m_blockFill = 0;
   }

   while (left >= 16)
   {
      Transform(p);
      p += 16;
      left -= 16;
   }

   memcpy(m_block, p, left);
   m_blockFill = static_cast<unsigned int>(left);
}

void MD2::Finish(uint8_t (&digest)[DIGEST_SIZE])
{
   // Pad with N bytes of value N, N being what is missing to fill the block. A full
   // block gets a whole extra block of 16s, so the padding is never empty
   const uint8_t pad = static_cast<uint8_t>(16 - m_blockFill);
   uint8_t padding[16];
   memset(padding, pad, pad);
   Update(padding, pad);

   // Then extend with the checksum. Snapshot it first: Transform() writes m_checksum
   // while reading its block argument, so the two must not alias
   uint8_t checksum[16];
   memcpy(checksum, m_checksum, 16);
   Update(checksum, 16);

   memcpy(digest, m_state, DIGEST_SIZE);
}
