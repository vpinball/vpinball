#pragma once

// this is the djb2 string hash algorithm, str is converted to lower case
inline unsigned long StringHash(const unsigned char *str)
{
   //MessageBox(0, str, 0, 0);
   unsigned long hash = 5381;
   int c;

   while ((c = *str++))
      hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */

   return hash;
}

// very simple hash, but good enough so far for the obj loader (i.e. hash gen speed matters!)
template <size_t T>
unsigned int FloatHash(const float a[T])
{
   const unsigned char *in = reinterpret_cast<const unsigned char*>(a);
   unsigned int ret = 2654435761u;
   for (size_t i = 0; i < (T * sizeof(float)); ++i)
      ret = (ret * 2654435761u) ^ *in++;
   
   return ret;
}


// case-insensitive hash
inline unsigned long StringHash(const char *str) { return StringHash((const unsigned char*)str); }

struct StringHashFunctor
{
   unsigned long operator()(const char* str) const
   {
      // use case-insensitive hash because user can enter the names in lower case from the script
      return StringHash(str);
   }
};

struct StringComparator
{
   bool operator()(const char* str1, const char* str2) const
   {
      // use case-insensitive compare because user can enter the names in lower case from the script
      return lstrcmpi(str1, str2) == 0;
   }
};

//

// ignores Idx-int completely!
struct Vertex3D_NoTex2IdxHashFunctor
{
   unsigned long operator()(std::pair<const Vertex3D_NoTex2*,const unsigned int> a) const
   {
      return FloatHash<sizeof(Vertex3D_NoTex2)/sizeof(float)>((const float*)a.first);
   }
};

// ignores Idx-int completely!
struct Vertex3D_NoTex2IdxComparator
{
   bool operator()(std::pair<const Vertex3D_NoTex2*,const unsigned int> a, std::pair<const Vertex3D_NoTex2*,const unsigned int> b) const
   {
      return memcmp(a.first, b.first, sizeof(Vertex3D_NoTex2)) == 0;
   }
};
