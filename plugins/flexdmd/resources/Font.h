#pragma once

#include "AssetSrc.h"
#include "VPXPlugin.h"
#include "bmfonts/BitmapFont.h"
#include "SurfaceGraphics.h"

class AssetManager;

class Font final {
public:
   Font(AssetManager* pAssetManager, AssetSrc* pAssetSrc);
   ~Font();

   PSC_IMPLEMENT_REFCOUNT()

   void DrawCharacter(VP::SurfaceGraphics* pGraphics, char character, char previousCharacter, float& x, float& y);
   SDL_Rect MeasureFont(const string& text);
   void DrawText_(VP::SurfaceGraphics* pGraphics, float x, float y, const string& text);
   BitmapFont* GetBitmapFont() const { return m_pBitmapFont; }

private:
   BitmapFont* m_pBitmapFont;
   SDL_Surface** m_textures;
};
