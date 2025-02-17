#include "Image.h"
#include "resources/Bitmap.h"
#include "resources/AssetManager.h"

Image::Image(FlexDMD* pFlexDMD, const string& name) : Actor(pFlexDMD, name)
{
}

Image* Image::Create(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& image, const string& name)
{
   AssetSrc* pSrc = pAssetManager->ResolveSrc(image, NULL);
   Bitmap* pBitmap = pAssetManager->GetBitmap(pSrc);
   if (!pBitmap) {
       free(pSrc);
       return NULL;
   }

   Image* pImage = new Image(pFlexDMD, name);
   pImage->m_alignment = Alignment_Center;
   pImage->m_scaling = Scaling_Stretch;
   pImage->m_pSrc = pSrc;
   pImage->m_pAssetManager = pAssetManager;
   pImage->m_pBitmap = pBitmap;
   pImage->SetPrefWidth(static_cast<float>(pBitmap->GetWidth()));
   pImage->SetPrefHeight(static_cast<float>(pBitmap->GetHeight()));
   pImage->Pack();
   pImage->m_pBitmap = NULL;

   return pImage;
}

Image::~Image()
{
   delete m_pSrc;

   if (m_pBitmap)
      m_pBitmap->Release();
}

Bitmap* Image::GetBitmap()
{
   if (m_pBitmap == nullptr)
      m_pBitmap = m_pAssetManager->GetBitmap(m_pSrc);
   return m_pBitmap;
}

void Image::SetBitmap(Bitmap *pBitmap)
{
   if (m_pBitmap)
      m_pBitmap->Release();
   m_pBitmap = pBitmap;
   if (m_pBitmap)
      m_pBitmap->AddRef();
}

void Image::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible() && m_pBitmap && m_pBitmap->GetSurface()) {
      float w = 0;
      float h = 0;
      Layout::Scale(m_scaling, GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), w, h);
      float x = 0;
      float y = 0;
      Layout::Align(m_alignment, w, h, GetWidth(), GetHeight(), x, y);
      SDL_Rect rect = { (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h };
      pGraphics->DrawImage(m_pBitmap->GetSurface(), NULL, &rect);
   }
}

void Image::OnStageStateChanged()
{
   if (m_pBitmap)
      m_pBitmap->Release();

   m_pBitmap = GetOnStage() ? m_pAssetManager->GetBitmap(m_pSrc) : NULL;

   if (m_pBitmap)
      m_pBitmap->AddRef();
}