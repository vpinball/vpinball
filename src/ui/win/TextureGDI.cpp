// license:GPLv3+

#include "core/stdafx.h"
#include "ui/win/TextureGDI.h"

#include "renderer/Texture.h"

HBITMAP TextureGDICache::Get(const Texture& tex)
{
   if (const auto it = m_cache.find(&tex); it != m_cache.end())
   {
      if (it->second.liveHash == tex.GetLiveHash())
         return it->second.bitmap;
      DeleteObject(it->second.bitmap); // stale entry: a new Texture was allocated at this address
      m_cache.erase(it);
   }

   const auto buffer = tex.GetRawBitmap(false, 0);
   if (buffer == nullptr)
      return nullptr;

   const HDC hdcScreen = GetDC(nullptr);
   HBITMAP hbm = CreateCompatibleBitmap(hdcScreen, tex.m_width, tex.m_height);
   const HDC hdcNew = CreateCompatibleDC(hdcScreen);
   const HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, hbm);

   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = tex.m_width;
   bmi.bmiHeader.biHeight = -(LONG)tex.m_height;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   std::shared_ptr<BaseTexture> bgr32bits = buffer->ToBGRA();
   SetStretchBltMode(hdcNew, COLORONCOLOR);
   StretchDIBits(hdcNew, 0, 0, tex.m_width, tex.m_height, 0, 0, tex.m_width, tex.m_height, bgr32bits->data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
   bgr32bits = nullptr;

   SelectObject(hdcNew, hbmOld);
   DeleteDC(hdcNew);
   ReleaseDC(nullptr, hdcScreen);
   m_cache[&tex] = { hbm, tex.GetLiveHash() };
   return hbm;
}

void TextureGDICache::Remove(const Texture& tex)
{
   if (const auto it = m_cache.find(&tex); it != m_cache.end())
   {
      DeleteObject(it->second.bitmap);
      m_cache.erase(it);
   }
}

void TextureGDICache::Clear()
{
   for (const auto& [tex, entry] : m_cache)
      DeleteObject(entry.bitmap);
   m_cache.clear();
}
