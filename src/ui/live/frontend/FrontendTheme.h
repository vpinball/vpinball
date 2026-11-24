// license:GPLv3+

#pragma once

class BaseTexture;

namespace VPX::Frontend
{

// A frontend theme: a folder holding a color keyed layout map plus optional underlay/overlay
// medias. Medias come in per aspect ratio variants ('-WxH' suffix, e.g. 'layout-21x9.webp')
// selected to best fit the host window; files without a suffix are used as a universal fallback.
class FrontendTheme final
{
public:
   // A discovered theme: each direct subfolder of the scanned roots holding a layout map is a theme
   struct Entry
   {
      string name;
      std::filesystem::path folder;
   };
   static vector<Entry> Discover(const vector<std::filesystem::path> &roots);

   // Normalized [0..1] region of the layout image (x/y max exclusive)
   struct Region
   {
      float xMin = 0.f, yMin = 0.f, xMax = 0.f, yMax = 0.f;
      bool valid = false;
   };

   // Regions parsed from the color keyed layout map:
   // (0,255,0) playfield, (0,0,255) backglass, (0,0,128) instruction card, (255,0,255) menu band,
   // (128,0,128) info band, (255,0,0) table wheel band, (255,255,0) table info pane. Other colors
   // are ignored. Color matching is done against the nearest key color with a tolerance, to survive
   // lossy image compression.
   struct Layout
   {
      Region playfield;
      Region backglass;
      Region card;
      Region menuBand;
      Region infoBand;
      Region infoPane;
      // Table wheel band: for each layout image column between wheelFirstColumn and
      // wheelLastColumn, the normalized lower/upper Y bounds of the red area. Columns without red
      // pixels are interpolated between their nearest neighbors.
      vector<std::pair<float, float>> wheelColumns;
      int wheelFirstColumn = -1;
      int wheelLastColumn = -1;
   };
   static Layout ParseLayout(const BaseTexture &layout);

   // Selects the variant of a 'stem' media (e.g. "layout", "underlay", "overlay") best fitting a
   // target aspect ratio (width/height): the smallest variant ratio >= target, else the largest
   // variant; unsuffixed files are only used when no aspect ratio variant exists. When 'animated'
   // is set, video and animated image files are eligible alongside still images. Returns an empty
   // path when the stem is absent.
   static std::filesystem::path SelectVariant(const std::filesystem::path &folder, const string &stem, float targetAR, bool animated = false);

   // Computes the 'fit on height, center crop on width' transform used to draw a variant (and to
   // map its normalized layout regions) inside a window: window pixel = image pixel * scale + offset
   static void ComputeFit(int imgW, int imgH, int wndW, int wndH, float &scale, float &offsetX, float &offsetY);
};

}
