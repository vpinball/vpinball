#include "stdafx.h"
#include "Font.h"

Font::Font(AssetManager* pAssetManager, AssetSrc* pAssetSrc)
{
   m_pBitmapFont = (BitmapFont*)pAssetManager->Open(pAssetSrc);
 
   m_textures = new SDL_Surface*[m_pBitmapFont->GetPageCount()];
   memset((char*)m_textures, 0, sizeof(SDL_Surface*) * m_pBitmapFont->GetPageCount());

   for (int i = 0; i < m_pBitmapFont->GetPageCount(); i++) {
      AssetSrc* pTextureAssetSrc = pAssetManager->ResolveSrc(m_pBitmapFont->GetPage(i)->GetFilename(), pAssetSrc);
      m_textures[i] = (SDL_Surface*)pAssetManager->Open(pTextureAssetSrc);
      delete pTextureAssetSrc;
   }

   if (pAssetSrc->GetFontBorderSize() > 0) {
      for (int i = 0; i < m_pBitmapFont->GetPageCount(); i++) {
         if (!m_textures[i])
            continue;

         SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
         SDL_Surface* src = SDL_ConvertSurface(m_textures[i], format, 0);
         SDL_FreeFormat(format);

         int w = src->w;
         int h = src->h;

         SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);

         UINT32 outline = SDL_MapRGBA(dst->format, 
            GetRValue(pAssetSrc->GetFontBorderTint()), 
            GetGValue(pAssetSrc->GetFontBorderTint()), 
            GetBValue(pAssetSrc->GetFontBorderTint()), 
            255);

         SDL_LockSurface(src);
         SDL_LockSurface(dst);

         UINT32* pixels_src = (UINT32*)src->pixels;
         UINT32* pixels_dst = (UINT32*)dst->pixels;

         UINT8 r;
         UINT8 g;
         UINT8 b;
         UINT8 a;

         for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
               SDL_GetRGBA(pixels_src[y * w + x], src->format, &r, &g, &b, &a);

               if (a == 0)
                  continue;

               if (x > 0) {
                  if (y > 0)
                     pixels_dst[(y - 1) * w + (x - 1)] = outline;
                  pixels_dst[y * w + (x - 1)] = outline;
                  if (y < h - 1)
                     pixels_dst[(y + 1) * w + (x - 1)] = outline;
               }
               if (y > 0)
                  pixels_dst[(y - 1) * w + x] = outline;
               if (y < h - 1)
                  pixels_dst[(y + 1) * w + x] = outline;
               if (x < w - 1) {
                  if (y > 0)
                     pixels_dst[(y - 1) * w + (x + 1)] = outline;
                  pixels_dst[y * w + (x + 1)] = outline;
                  if (y < h - 1)
                     pixels_dst[(y + 1) * w + (x + 1)] = outline;
               }
            }
         }

         UINT8 tint_r = GetRValue(pAssetSrc->GetFontTint());
         UINT8 tint_g = GetGValue(pAssetSrc->GetFontTint());
         UINT8 tint_b = GetBValue(pAssetSrc->GetFontTint());

         for (int idx = 0; idx < dst->w * dst->h; ++idx) {
            SDL_GetRGBA(pixels_src[idx], src->format, &r, &g, &b, &a);

            if (a == 0)
               continue;

            r = (UINT8)SDL_min((r * tint_r) / 255, 255);
            g = (UINT8)SDL_min((g * tint_g) / 255, 255);
            b = (UINT8)SDL_min((b * tint_b) / 255, 255);

            pixels_dst[idx] = SDL_MapRGBA(dst->format, r, g, b, a);
         }

         SDL_UnlockSurface(src);
         SDL_UnlockSurface(dst);
         SDL_FreeSurface(src);
         SDL_FreeSurface(m_textures[i]);

         m_textures[i] = dst;
      }

      for(const auto& pair : m_pBitmapFont->GetCharacters()) {
         Character* character = pair.second;
         character->SetXAdvance(character->GetXAdvance() + 2);
      }
   }
   else if (pAssetSrc->GetFontTint() != RGB(255, 255, 255)) {
      for (int i = 0; i < m_pBitmapFont->GetPageCount(); i++) {
         if (!m_textures[i])
            continue;

         SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
         SDL_Surface* dst = SDL_ConvertSurface(m_textures[i], format, 0);
         SDL_FreeFormat(format);

         int w = dst->w;
         int h = dst->h;

         SDL_LockSurface(dst);

         UINT32* pixels_dst = (UINT32*)dst->pixels;

         UINT8 r;
         UINT8 g;
         UINT8 b;
         UINT8 a;

         UINT8 tint_r = GetRValue(pAssetSrc->GetFontTint());
         UINT8 tint_g = GetGValue(pAssetSrc->GetFontTint());
         UINT8 tint_b = GetBValue(pAssetSrc->GetFontTint());

         for (int idx = 0; idx < dst->w * dst->h; ++idx) {
            SDL_GetRGBA(pixels_dst[idx], dst->format, &r, &g, &b, &a);

            r = (UINT8)SDL_min((r * tint_r) / 255, 255);
            g = (UINT8)SDL_min((g * tint_g) / 255, 255);
            b = (UINT8)SDL_min((b * tint_b) / 255, 255);

            pixels_dst[idx] = SDL_MapRGBA(dst->format, r, g, b, a);
         }

         SDL_UnlockSurface(dst);
         SDL_FreeSurface(m_textures[i]);

         m_textures[i] = dst;
      }
   }
}

Font::~Font()
{
   if (m_pBitmapFont) {
      for (int i = 0; i < m_pBitmapFont->GetPageCount(); ++i) {
         if (m_textures[i])
            SDL_FreeSurface(m_textures[i]);
      }

      delete m_pBitmapFont;
   }
}

void Font::DrawCharacter(VP::SurfaceGraphics* pGraphics, char character, char previousCharacter, float& x, float& y)
{
   if (character == '\n') {
      x = 0;
      y += m_pBitmapFont->GetLineHeight();
   }
   else {
      Character* pCharacter = m_pBitmapFont->GetCharacter(character);
      if (pCharacter) {
         int kerning = m_pBitmapFont->GetKerning(previousCharacter, character);
         if (pGraphics) {
            SDL_Surface* pSource = m_textures[pCharacter->GetTexturePage()];
            if (pSource) {
               SDL_Rect bounds = pCharacter->GetBounds();
               SDL_Point offset = pCharacter->GetOffset();
               SDL_Rect rect = { (int)(x + offset.x + kerning), (int)(y + offset.y), bounds.w, bounds.h };
               pGraphics->DrawImage(pSource, &bounds, &rect);
            }
         }
         x += pCharacter->GetXAdvance() + kerning;
      }
      else if ('a' <= character && character <= 'z' && m_pBitmapFont->GetCharacter(toupper(character))) {
         m_pBitmapFont->SetCharacter(character, m_pBitmapFont->GetCharacter(toupper(character)));
         DrawCharacter(pGraphics, character, previousCharacter, x, y);
      }
      else if (m_pBitmapFont->GetCharacter(' ')) {
         PLOGD.printf("Missing character 0x%02X replaced by ' '", character);
         m_pBitmapFont->SetCharacter(character, m_pBitmapFont->GetCharacter(' '));
         DrawCharacter(pGraphics, character, previousCharacter, x, y);
      }
   }
}

SDL_Rect Font::MeasureFont(const string& text)
{
   DrawText_(NULL, 0, 0, text);
   return m_pBitmapFont->MeasureFont(text);
}

void Font::DrawText_(VP::SurfaceGraphics* pGraphics, float x, float y, const string& text)
{
   char previousCharacter = ' ';
   for (int i = 0; i < text.length(); i++) {
      char character = text[i];
      DrawCharacter(pGraphics, character, previousCharacter, x, y);
      previousCharacter = character;
   }
}