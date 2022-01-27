#pragma once

#include <map>

#include "Texture.h"
#include "stdafx.h"
#include "typedefs3D.h"

class RenderDevice;

class TextureManager
{
public:
   TextureManager(RenderDevice& rd) : m_rd(rd)
   { }

   ~TextureManager()
   {
      UnloadAll();
   }

   D3DTexture* LoadTexture(BaseTexture* memtex, const bool linearRGB, const bool clamptoedge = false);
   void SetDirty(BaseTexture* memtex);
   void UnloadTexture(BaseTexture* memtex);
   void UnloadAll();

private:
   struct TexInfo
   {
      D3DTexture* d3dtex;
      int texWidth;
      int texHeight;
      bool dirty;
   };

   RenderDevice& m_rd;
   std::map<BaseTexture*, TexInfo> m_map;
   typedef std::map<BaseTexture*, TexInfo>::iterator Iter;
};
