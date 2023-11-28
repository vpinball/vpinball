#pragma once

#include "AlphaNumeric.h"
#include "Serum.h"
#include "ZeDMDUtil.h"
#include "../common/Graphics.h"

class DMDUtil
{
public:
   static DMDUtil* GetInstance();

   void Register(int width, int height, int depth = 2);
   void Cleanup();
   void SetAlphaNumericLayout(NumericalLayout alphaNumeric) { m_alphaNumeric = alphaNumeric; }
   bool IsAlphaNumeric() { return m_alphaNumeric != NumericalLayout::None; }
   void SetSerum(const string& szPath, const string& szRom);
   void SetColor(OLE_COLOR color);
   void SetColor(OLE_COLOR color, int depth);
   UINT8* GetPalette() { return m_palette; }
   void SetPixelData(UINT8* pPixels, OLE_COLOR color, int depth);
   void SetRGB24PixelData(UINT8* pPixels);
   UINT8* GetRGB24PixelData() { return m_pRGBData; }
   void SetAlphaNumericData(UINT16* pData1, UINT16* pData2, bool reset, OLE_COLOR color);
   void SetData(UINT8* pData, OLE_COLOR color);
   UINT8* GetData() { return m_pData; }
   bool IsRGB24() { return m_rgb; }
   bool IsSerumLoaded() { return m_serum.IsLoaded(); }
   bool IsUpdated() { return m_updated; }
   void ResetUpdated() { m_updated = false; }
   int GetWidth() { return m_width; }
   int GetHeight() { return m_height; }
   bool IsActive() { return m_active; }
   void Render(VP::Graphics* pGraphics);

private:
   DMDUtil();
   ~DMDUtil();

   float CalcBrightness(float x);
   void RefreshPalette();

   int m_width;
   int m_height;
   int m_depth;
   NumericalLayout m_alphaNumeric;
   int m_dataSize;
   UINT8* m_pData;
   UINT8* m_pRenderData;
   UINT8* m_pBPData;
   int m_rgbDataSize;
   UINT8* m_pRGBData;
   UINT8* m_pRenderRGBData;
   UINT8 m_palette[64 * 3];
   UINT8 m_renderPalette[64 * 3];
   Serum m_serum;
   bool m_updated;
   bool m_rgb;
   OLE_COLOR m_color;
   SDL_Texture* m_pTexture;
   ZeDMDUtil* m_pZeDMDUtil;
   bool m_active;

   static DMDUtil* pInstance;     
};
