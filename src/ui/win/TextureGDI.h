// license:GPLv3+

#pragma once

#include "unordered_dense.h"

class Texture;

// Per-table-editor cache of lazily created GDI bitmap views of table textures (UI thread only).
// Entries are keyed on the texture instance and its live hash, so a recycled Texture* never returns a stale bitmap.
class TextureGDICache
{
public:
   ~TextureGDICache() { Clear(); }

   HBITMAP Get(const Texture& tex);
   void Remove(const Texture& tex); // drop the cached bitmap of a texture being destroyed
   void Clear();

private:
   struct Entry
   {
      HBITMAP bitmap;
      uint64_t liveHash;
   };
   ankerl::unordered_dense::map<const Texture*, Entry> m_cache;
};
