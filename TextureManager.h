#pragma once

#include <inc/robin_hood.h>

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
   robin_hood::unordered_map<BaseTexture*, TexInfo> m_map;
   typedef robin_hood::unordered_map<BaseTexture*, TexInfo>::iterator Iter;
};
