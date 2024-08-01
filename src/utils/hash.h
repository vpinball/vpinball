#pragma once

// this is the djb2 (with XOR mod) string hash algorithm, str is converted to lower case
// in general found to only work well for chars/strings
inline size_t StringHash(const string& str)
{
   //MessageBox(0, str, 0, 0);
   unsigned int hash = 5381;

   const size_t l = str.length();
   for (size_t i = 0; i < l; ++i)
      hash = ((hash << 5) + hash) ^ cLower(str[i]); /* hash * 33 ^ str[i] */

   return hash;
}

// simple hash (FNV1-a), but good enough for the obj loader (i.e. hash gen speed matters!)
// in general a very good hash, but for large data blocks other ones (XXH3, etc) can be much faster and higher quality
template <size_t T>
size_t FloatHash(const float a[T])
{
   const unsigned char * __restrict in = reinterpret_cast<const unsigned char*>(a);
   unsigned int ret = 0x811c9dc5u;
   for (size_t i = 0; i < (T * sizeof(float)); ++i)
      ret = (ret ^ *in++) * 0x1000193u;

   /* optional post process scamble
   hash += hash << 13;
   hash ^= hash >> 7;
   hash += hash << 3;
   hash ^= hash >> 17;
   hash += hash << 5;
   */

   return ret;
}

struct StringHashFunctor
{
   size_t operator()(const string& str) const
   {
      // use case-insensitive hash because user can enter the names in lower case from the script
      return StringHash(str);
   }
};

struct StringComparator
{
   bool operator()(const string& str1, const string& str2) const
   {
      // use case-insensitive compare because user can enter the names in lower case from the script
      return StrCompareNoCase(str1, str2);
   }
};

//

// ignores Idx-int completely!
struct Vertex3D_NoTex2IdxHashFunctor
{
   size_t operator()(const std::pair<const Vertex3D_NoTex2*,const unsigned int>& a) const
   {
      return FloatHash<sizeof(Vertex3D_NoTex2)/sizeof(float)>((const float*)a.first);
   }
};

// ignores Idx-int completely!
struct Vertex3D_NoTex2IdxComparator
{
   bool operator()(const std::pair<const Vertex3D_NoTex2*,const unsigned int>& a, const std::pair<const Vertex3D_NoTex2*,const unsigned int>& b) const
   {
      return memcmp(a.first, b.first, sizeof(Vertex3D_NoTex2)) == 0;
   }
};

namespace robin_hood {
template<>
struct hash<robin_hood::pair<unsigned, unsigned>> {
   size_t operator()(robin_hood::pair<unsigned, unsigned> const& p) const noexcept {
      return hash<unsigned long long>{}((unsigned long long)p.first | (((unsigned long long)p.second) << 32));
   }
};
template<>
struct hash<robin_hood::pair<int, int>> {
   size_t operator()(robin_hood::pair<int, int> const& p) const noexcept {
      return hash<unsigned long long>{}((unsigned long long)(unsigned int)p.first | (((unsigned long long)(unsigned int)p.second) << 32));
   }
};
}

/////////////////////////////////////////////////////////////////////////////////////////
// Simple MD5 hashing from public domain implementation: https://github.com/Zunawe/md5-c

typedef struct
{
   uint64_t size; // Size of input in bytes
   uint32_t buffer[4]; // Current accumulation of hash
   uint8_t input[64]; // Input to be used in the next step
   uint8_t digest[16]; // Result of algorithm
} MD5Context;

static constexpr uint8_t MD5S[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4,
   11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

static constexpr uint32_t MD5K[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122,
   0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
   0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085,
   0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0,
   0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

/*
 * Padding used to make the size (in bits) of the input congruent to 448 mod 512
 */
static constexpr uint8_t MD5PADDING[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/*
 * Bit-manipulation functions defined by the MD5 algorithm
 */
#define MD5F(X, Y, Z) ((X & Y) | (~X & Z))
#define MD5G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define MD5H(X, Y, Z) (X ^ Y ^ Z)
#define MD5I(X, Y, Z) (Y ^ (X | ~Z))

/*
 * Rotates a 32-bit word left by n bits
 */
inline uint32_t rotateLeft(const uint32_t x, const uint8_t n) { return (x << n) | (x >> (32 - n)); } //!! opt.?

/*
 * Initialize a context
 */
inline void md5Init(MD5Context* ctx)
{
   ctx->size = 0;

   ctx->buffer[0] = 0x67452301u;
   ctx->buffer[1] = 0xefcdab89u;
   ctx->buffer[2] = 0x98badcfeu;
   ctx->buffer[3] = 0x10325476u;
}

/*
 * Step on 512 bits of input with the main MD5 algorithm.
 */
static void md5Step(uint32_t buffer[4], const uint32_t input[16])
{
   uint32_t AA = buffer[0];
   uint32_t BB = buffer[1];
   uint32_t CC = buffer[2];
   uint32_t DD = buffer[3];

   for (unsigned int i = 0; i < 64; ++i)
   {
      uint32_t E;
      unsigned int j;

      switch (i / 16)
      {
      case 0:
          E = MD5F(BB, CC, DD);
          j = i;
          break;
      case 1:
          E = MD5G(BB, CC, DD);
          j = ((i * 5) + 1) % 16;
          break;
      case 2:
          E = MD5H(BB, CC, DD);
          j = ((i * 3) + 5) % 16;
          break;
      default:
          E = MD5I(BB, CC, DD);
          j = (i * 7) % 16;
          break;
      }

      const uint32_t temp = DD;
      DD = CC;
      CC = BB;
      BB = BB + rotateLeft(AA + E + MD5K[i] + input[j], MD5S[i]);
      AA = temp;
   }

   buffer[0] += AA;
   buffer[1] += BB;
   buffer[2] += CC;
   buffer[3] += DD;
}

/*
 * Add some amount of input to the context
 *
 * If the input fills out a block of 512 bits, apply the algorithm (md5Step)
 * and save the result in the buffer. Also updates the overall size.
 */
static void md5Update(MD5Context* const ctx, const uint8_t* const input_buffer, size_t input_len)
{
   uint32_t input[16];
   unsigned int offset = ctx->size % 64;
   ctx->size += (uint64_t)input_len;

   // Copy each byte in input_buffer into the next space in our context input
   for (size_t i = 0; i < input_len; ++i)
   {
      ctx->input[offset++] = (uint8_t) * (input_buffer + i);

      // If we've filled our context input, copy it into our local array input
      // then reset the offset to 0 and fill in a new buffer.
      // Every time we fill out a chunk, we run it through the algorithm
      // to enable some back and forth between cpu and i/o
      if (offset == 64)
      {
         for (unsigned int j = 0; j < 16; ++j)
         {
            // Convert to little-endian
            // The local variable `input` our 512-bit chunk separated into 32-bit words
            // we can use in calculations
            input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 | (uint32_t)(ctx->input[(j * 4) + 2]) << 16 | (uint32_t)(ctx->input[(j * 4) + 1]) << 8 | (uint32_t)(ctx->input[(j * 4)]);
         }
         md5Step(ctx->buffer, input);
         offset = 0;
      }
   }
}

/*
 * Pad the current input to get to 448 bytes, append the size in bits to the very end,
 * and save the result of the final iteration into digest.
 */
static void md5Finalize(MD5Context* const ctx)
{
   uint32_t input[16];
   unsigned int offset = ctx->size % 64;
   unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

   // Fill in the padding and undo the changes to size that resulted from the update
   md5Update(ctx, MD5PADDING, padding_length);
   ctx->size -= (uint64_t)padding_length;

   // Do a final update (internal to this function)
   // Last two 32-bit words are the two halves of the size (converted from bytes to bits)
   for (unsigned int j = 0; j < 14; ++j)
   {
      input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 | (uint32_t)(ctx->input[(j * 4) + 2]) << 16 | (uint32_t)(ctx->input[(j * 4) + 1]) << 8 | (uint32_t)(ctx->input[(j * 4)]);
   }
   input[14] = (uint32_t)(ctx->size * 8);
   input[15] = (uint32_t)((ctx->size * 8) >> 32);

   md5Step(ctx->buffer, input);

   // Move the result into digest (convert from little-endian)
   for (unsigned int i = 0; i < 4; ++i)
   {
      ctx->digest[(i * 4) + 0] = (uint8_t)((ctx->buffer[i] & 0x000000FFu));
      ctx->digest[(i * 4) + 1] = (uint8_t)((ctx->buffer[i] & 0x0000FF00u) >> 8);
      ctx->digest[(i * 4) + 2] = (uint8_t)((ctx->buffer[i] & 0x00FF0000u) >> 16);
      ctx->digest[(i * 4) + 3] = (uint8_t)((ctx->buffer[i] & 0xFF000000u) >> 24);
   }
}

static void generateMD5(const uint8_t* const __restrict data, const size_t num, uint8_t md5Hash[16])
{
   MD5Context ctx;
   md5Init(&ctx);
   md5Update(&ctx, data, num);
   md5Finalize(&ctx);
   memcpy(md5Hash, ctx.digest, 16);
}
