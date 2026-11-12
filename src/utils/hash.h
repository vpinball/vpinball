#pragma once

#include "unordered_dense.h"

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

template <>
struct ankerl::unordered_dense::hash<std::pair<unsigned, unsigned>> {
   using is_avalanching = void;

   [[nodiscard]] auto operator()(std::pair<unsigned, unsigned> const& x) const noexcept -> uint64_t {
      return detail::hash_impl::hash_int((uint64_t)x.first | (((uint64_t)x.second) << 32));
   }
};

template <>
struct ankerl::unordered_dense::hash<std::pair<int, int>> {
   using is_avalanching = void;

   [[nodiscard]] auto operator()(std::pair<int, int> const& x) const noexcept -> uint64_t {
      return detail::hash_impl::hash_int((uint64_t)(uint32_t)x.first | (((uint64_t)(uint32_t)x.second) << 32));
   }
};
