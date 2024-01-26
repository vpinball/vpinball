#include "stdafx.h"
#include "AssetSrc.h"
#include "filters/BitmapFilter.h"

AssetSrc::AssetSrc()
{
   m_assetType = AssetType_Unknown;
   m_srcType = AssetSrcType_File;

   m_fontTint = RGB(255, 255, 255);
   m_fontBorderTint = RGB(255, 255, 255);
   m_fontBorderSize = 0;
}

AssetSrc::~AssetSrc()
{
   for (BitmapFilter* pBitmapFilter : m_bitmapFilters)
      delete pBitmapFilter;

   m_bitmapFilters.clear();
}