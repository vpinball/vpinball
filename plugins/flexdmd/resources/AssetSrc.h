#pragma once

#include "common.h"
#include "ScriptablePlugin.h"

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

class AssetSrc final
{
public:
   AssetSrc() = default;
   ~AssetSrc();

   PSC_IMPLEMENT_REFCOUNT()

   const string& GetId() { return m_id; }
   string GetIdWithoutOptions() const { return m_id.substr(0, m_id.find_first_of('&')); }
   void SetId(const string& id) { m_id = id; }
   void SetPath(const string& path) { m_path = path; }
   const string& GetPath() const { return m_path; }
   AssetType GetAssetType() const { return m_assetType; }
   void SetAssetType(AssetType assetType) { m_assetType = assetType; }
   AssetSrcType GetSrcType() const { return m_srcType; }
   void SetSrcType(AssetSrcType srcType) { m_srcType = srcType; }
   uint32_t GetFontTint() const { return m_fontTint; }
   void SetFontTint(uint32_t fontTint) { m_fontTint = fontTint; }
   uint32_t GetFontBorderTint() const { return m_fontBorderTint; }
   void SetFontBorderTint(uint32_t fontBorderTint) { m_fontBorderTint = fontBorderTint; }
   int GetFontBorderSize() const { return m_fontBorderSize; }
   void SetFontBorderSize(int fontBorderSize) { m_fontBorderSize = fontBorderSize; }
   vector<BitmapFilter*>& GetBitmapFilters() { return m_bitmapFilters; }

private:
   string m_id;
   string m_path;
   AssetType m_assetType = AssetType_Unknown;
   AssetSrcType m_srcType = AssetSrcType_File;

   uint32_t m_fontTint = 0x00FFFFFF;
   uint32_t m_fontBorderTint = 0x00FFFFFF;
   int m_fontBorderSize = 0;

   vector<BitmapFilter*> m_bitmapFilters;
};
