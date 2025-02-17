#include "AssetSrc.h"
#include "BitmapFilter.h"

AssetSrc::~AssetSrc()
{
   for (BitmapFilter* pBitmapFilter : m_bitmapFilters)
      delete pBitmapFilter;

   m_bitmapFilters.clear();
}
