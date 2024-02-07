#include "stdafx.h"

#include "GIFImage.h"

GIFImage::GIFImage(FlexDMD* pFlexDMD, const string& name) : AnimatedActor(pFlexDMD, name)
{
}

GIFImage* GIFImage::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name)
{
   AssetSrc* pSrc = pAssetManager->ResolveSrc(path, NULL);
   Bitmap* pBitmap = pAssetManager->GetBitmap(pSrc);
   if (!pBitmap) {
       free(pSrc);
       return NULL;
   }

   GIFImage* pImage = new GIFImage(pFlexDMD, name);
   pImage->m_pos = 0;
   pImage->m_pSrc = pSrc;
   pImage->m_pAssetManager = pAssetManager;
   pImage->m_pBitmap = pBitmap;
   pImage->SetPrefWidth(pBitmap->GetWidth());
   pImage->SetPrefHeight(pBitmap->GetHeight());
   pImage->SetLength(pBitmap->GetLength() / 1000.0f);
   pImage->Rewind();
   pImage->Pack();
   pImage->m_pBitmap = NULL;
   pImage->m_pActiveFrameSurface = NULL;

   return pImage;
}

GIFImage::~GIFImage()
{
   if (m_pSrc)
      delete m_pSrc;
}

void GIFImage::OnStageStateChanged()
{
   m_pBitmap = GetOnStage() ? m_pAssetManager->GetBitmap(m_pSrc) : NULL;
   UpdateFrame();
}

void GIFImage::Rewind()
{
   AnimatedActor::Rewind();
   m_pos = 0;

   if (m_pBitmap)
      SetFrameDuration(m_pBitmap->GetFrameDelay(0) / 1000.0f);
}

void GIFImage::ReadNextFrame()
{
   if (!m_pBitmap)
      return;

   if (m_pos >= m_pBitmap->GetFrameCount() - 1)
      SetEndOfAnimation(true);
   else {
      m_pos++;
      SetFrameTime(0);
      for (int i = 0; i < m_pos; i++)
         SetFrameTime(GetFrameTime() + (m_pBitmap->GetFrameDelay(i) / 1000.0f));
        
      SetFrameDuration(m_pBitmap->GetFrameDelay(m_pos) / 1000.0f);
      UpdateFrame();
   }
}

void GIFImage::UpdateFrame() 
{
   if (m_pBitmap)
      m_pActiveFrameSurface = m_pBitmap->GetFrameSurface(m_pos);
}

void GIFImage::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (!m_pBitmap)
      return;

   if (GetVisible() && m_pActiveFrameSurface) {
      float w = 0;
      float h = 0;
      Layout::Scale(GetScaling(), GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), w, h);
      float x = 0;
      float y = 0;
      Layout::Align(GetAlignment(), w, h, GetWidth(), GetHeight(), x, y);
      SDL_Rect rect = { (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h };
      pGraphics->DrawImage(m_pActiveFrameSurface, NULL, &rect);
   }
}