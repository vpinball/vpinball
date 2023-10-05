#include "stdafx.h"

#include "GIFImage.h"

GIFImage::GIFImage(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& path, const string& name) : AnimatedActor(pFlexDMD, name)
{
   m_pos = 0;
   m_pAssetManager = NULL;
   m_pSrc = pAssetManager->ResolveSrc(path, NULL);
   m_pAssetManager = pAssetManager;
   m_pBitmap = pAssetManager->GetBitmap(m_pSrc);
   SetPrefWidth(m_pBitmap->GetWidth());
   SetPrefHeight(m_pBitmap->GetHeight());
   SetLength(m_pBitmap->GetLength() / 1000.0f);
   Rewind();
   Pack();
   m_pBitmap = NULL;
   m_pActiveFrameSurface = NULL;
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

void GIFImage::Draw(VP::Graphics* pGraphics)
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
      pGraphics->DrawImage(m_pActiveFrameSurface, (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h);
   }
}