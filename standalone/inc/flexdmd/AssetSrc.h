#pragma once

#include "flexdmd_i.h"

typedef enum {
   AssetSrcType_File = 0,
   AssetSrcType_FlexResource = 1,
   AssetSrcType_VPXResource = 2
} AssetSrcType;

typedef enum {
   AssetType_Unknown = 0,
   AssetType_Image = 1,
   AssetType_GIF = 2,
   AssetType_Video = 3,
   AssetType_BMFont = 4
} AssetType;

class BitmapFilter;

class AssetSrc
{
public:
   AssetSrc();
   ~AssetSrc();

   const string& GetId() { return m_id; }
   string GetIdWithoutOptions() { return m_id.substr(0, m_id.find_first_of("&")); }
   void SetId(const string& id) { m_id = id; }
   void SetPath(const string& path) { m_path = path; }
   const string& GetPath() { return m_path; }
   AssetType GetAssetType() { return m_assetType; }
   void SetAssetType(AssetType assetType) { m_assetType = assetType; }
   AssetSrcType GetSrcType() { return m_srcType; }
   void SetSrcType(AssetSrcType srcType) { m_srcType = srcType; }
   OLE_COLOR GetFontTint() { return m_fontTint; }
   void SetFontTint(OLE_COLOR fontTint) { m_fontTint = fontTint; }
   OLE_COLOR GetFontBorderTint() { return m_fontBorderTint; }
   void SetFontBorderTint(OLE_COLOR fontBorderTint) { m_fontBorderTint = fontBorderTint; }
   int GetFontBorderSize() { return m_fontBorderSize; }
   void SetFontBorderSize(int fontBorderSize) { m_fontBorderSize = fontBorderSize; }
   vector<BitmapFilter*>& GetBitmapFilters() { return m_bitmapFilters; }

private:
   string m_id;
   string m_path;
   AssetType m_assetType;
   AssetSrcType m_srcType;

   OLE_COLOR m_fontTint;
   OLE_COLOR m_fontBorderTint;
   int m_fontBorderSize;

   vector<BitmapFilter*> m_bitmapFilters;
};
