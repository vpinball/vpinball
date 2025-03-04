#include "AssetSrc.h"
#include "BitmapFilter.h"

AssetSrc::~AssetSrc()
{
   assert(m_refCount == 0);
   
   for (BitmapFilter* pBitmapFilter : m_bitmapFilters)
      delete pBitmapFilter;
   m_bitmapFilters.clear();
}
