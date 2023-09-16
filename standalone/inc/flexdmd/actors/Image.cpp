#include "stdafx.h"

#include "Image.h"
#include "../Bitmap.h"

Image::Image(FlexDMD* pFlexDMD, AssetManager* pAssetManager, const string& image, const string& name) : Actor(pFlexDMD, name)
{
   m_alignment = Alignment_Center;
   m_scaling = Scaling_Stretch;
   m_pAssetManager = NULL;
   m_pSrc = pAssetManager->ResolveSrc(image, NULL);
   m_pAssetManager = pAssetManager;
   m_pBitmap = pAssetManager->GetBitmap(m_pSrc);
   SetPrefWidth(m_pBitmap->GetWidth());
   SetPrefHeight(m_pBitmap->GetHeight());
   Pack();
   m_pBitmap = NULL;
}

Image::~Image()
{
   if (m_pSrc)
      delete m_pSrc;

   if (m_pBitmap)
      m_pBitmap->Release();
}

STDMETHODIMP Image::get_Bitmap(_Bitmap **pRetVal)
{
   Bitmap* obj = m_pBitmap ? m_pBitmap : m_pAssetManager->GetBitmap(m_pSrc);
   return obj->QueryInterface(IID_IDispatch, (void**)pRetVal);
}

STDMETHODIMP Image::putref_Bitmap(_Bitmap *pRetVal)
{
   if (m_pBitmap)
      m_pBitmap->Release();

   m_pBitmap = dynamic_cast<Bitmap*>((Bitmap*)pRetVal);

   if (m_pBitmap)
      m_pBitmap->AddRef();

   return S_OK;
}

void Image::Draw(Graphics* graphics)
{
   if (GetVisible() && m_pBitmap && m_pBitmap->GetSurface()) {
      float w = 0;
      float h = 0;
      Layout::Scale(m_scaling, GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), w, h);
      float x = 0;
      float y = 0;
      Layout::Align(m_alignment, w, h, GetWidth(), GetHeight(), x, y);
      graphics->DrawImage(m_pBitmap->GetSurface(), (int)(GetX() + x), (int)(GetY() + y), (int)w, (int)h);
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