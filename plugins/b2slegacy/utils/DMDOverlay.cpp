// license:GPLv3+

#include "DMDOverlay.h"
#include "../common.h" // Needed for logging

#include <cstddef> // for size_t, ptrdiff_t
// Define ssize_t for Windows
#if defined(_WIN32) && !defined(__SSIZE_T_DEFINED)
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif
#define __SSIZE_T_DEFINED
#endif

#include "../base64.h"
#include "tinyxml2/tinyxml2.h"
#include "nixie/NixieLayoutFile.h"
#include "plugins/PluginStrings.h"

#include <cmath>
#include <deque>
#include <vector>
#include <stack>
#include <algorithm>
#include <string>
#include <tuple>
#include <format>
#include <vector>
#include <fstream>
#include <iterator>

using std::string;
using namespace std::string_literals;
using namespace std::string_view_literals;

using std::vector;


namespace B2SLegacy::DMDOverlay
{

DMDOverlay::DMDOverlay(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver, VPXTexture& dmdTex)
   : m_resURIResolver(resURIResolver)
   , m_dmdTex(dmdTex)
   , m_vpxApi(vpxApi)
{
}

DMDOverlay::~DMDOverlay()
{
   m_stopSearching = true;
   if (m_frameSearch.valid())
      m_frameSearch.wait();
   if (m_glassTex != nullptr && m_vpxApi != nullptr)
      m_vpxApi->DeleteTexture(m_glassTex);
}

void DMDOverlay::Render(VPXRenderContext2D* ctx, B2SDMDDisplay* display, VPXTexture docImage, int docW, int docH, int docCutY, int docCutH)
{
   PinballPlugin::ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState(display->source.empty() ? PinballPlugin::ResURIResolver::DefaultDmdUri : display->source);
   if (dmd.state.frame == nullptr)
      return;

   // Bounds detection request, issued by the 'Detect position' action of the editing page:
   // the detected frame is written back into the document fields (see below)
   if (display->detectRequest != 0 && !m_frameSearch.valid() && docImage != nullptr)
   {
      display->detectRequest = 0;
      const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(docImage);
      if (texInfo != nullptr)
      {
         m_stopSearching = false;
         const float ar = display->options.sizeW > 0 && display->options.sizeH > 0 ? static_cast<float>(display->options.sizeW) / static_cast<float>(display->options.sizeH)
                                                                                  : static_cast<float>(dmd.source->width) / static_cast<float>(dmd.source->height);
         m_frameSearch = std::async(std::launch::async, [this, docImage, texInfo, ar]() { return SearchDmdSubFrame(docImage, texInfo, ar); });
      }
   }
   if (m_frameSearch.valid() && m_frameSearch.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
   {
      const vec4<int> frame = m_frameSearch.get();
      if (frame.z != 0 && frame.w != 0)
      {
         display->x = frame.x;
         display->y = frame.y;
         display->w = frame.z;
         display->h = frame.w;
      }
   }

   // Map the display bounds from the B2S document space (native artwork, y downward) to the
   // render space (view size, y upward). The rendered artwork is the document image anisotropically
   // scaled, minus a cut band: doc rows [docCutY, docCutY + docCutH) are not displayed, doc rows
   // below them shift up (grill clipped away when hidden, small grill band removed when shown)
   const int docVisH = std::max(1, docH - docCutH);
   const float scaleX = ctx->srcWidth / static_cast<float>(docW);
   const float scaleY = ctx->srcHeight / static_cast<float>(docVisH);
   const auto mapY = [docCutY, docCutH](int y) { return y <= docCutY ? y : std::max(y - docCutH, docCutY); };
   const int visY0 = mapY(display->y);
   const int visY1 = mapY(display->y + display->h);
   m_frame.x = static_cast<int>(display->x * scaleX);
   m_frame.y = static_cast<int>(ctx->srcHeight - visY1 * scaleY);
   m_frame.z = static_cast<int>(display->w * scaleX);
   m_frame.w = static_cast<int>((visY1 - visY0) * scaleY);
   if (m_frame.z == 0 || m_frame.w == 0)
      return;
   if (m_frame.y + m_frame.w <= 0 || m_frame.y >= static_cast<int>(ctx->srcHeight))
      return; // Fully inside the grill (hidden) or off the artwork

   // The texture is owned by the form, hence the null test: it may still have to be created here
   if (!m_hasUploadedFrame || (m_dmdTex == nullptr) || (dmd.state.frameId != m_uploadedFrameId) || (*dmd.source != m_uploadedSrc))
   {
      switch (dmd.source->frameFormat)
      {
      case CTLPI_DISPLAY_FORMAT_LUM32F: m_vpxApi->UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_BW32F, dmd.state.frame); break;
      case CTLPI_DISPLAY_FORMAT_SRGB888: m_vpxApi->UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB8, dmd.state.frame); break;
      case CTLPI_DISPLAY_FORMAT_SRGB565: m_vpxApi->UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height, VPXTextureFormat::VPXTEXFMT_sRGB565, dmd.state.frame); break;
      default: return;
      }
      m_uploadedSrc = *dmd.source;
      m_uploadedFrameId = dmd.state.frameId;
      m_hasUploadedFrame = true;
   }

   // The glass texture is owned by the overlay: (re)create it when the display's
   // embedded image content changes (identified by a hash of its encoded bytes)
   if (!display->glassImage.empty())
   {
      size_t hash = 14695981039346656037ull;
      for (const uint8_t byte : display->glassImage)
         hash = (hash ^ byte) * 1099511628211ull;
      if (hash != m_glassHash)
      {
         if (m_glassTex != nullptr)
            m_vpxApi->DeleteTexture(m_glassTex);
         m_glassTex = m_vpxApi->CreateTexture(display->glassImage.data(), static_cast<int>(display->glassImage.size()));
         m_glassHash = m_glassTex != nullptr ? hash : 0;
      }
   }
   else if (m_glassTex != nullptr)
   {
      m_vpxApi->DeleteTexture(m_glassTex);
      m_glassTex = nullptr;
      m_glassHash = 0;
   }

   const PinballPlugin::OverlayDisplayOptions& o = display->options;
   vec4<float> glassArea;
   if (m_glassTex == nullptr || o.glassAreaW == 0 || o.glassAreaH == 0)
      glassArea = vec4<float>(0.f, 0.f, 1.f, 1.f);
   else if (const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(m_glassTex); texInfo != nullptr)
      glassArea = vec4<float>(o.glassAreaX / static_cast<float>(texInfo->width), o.glassAreaY / static_cast<float>(texInfo->height), o.glassAreaW / static_cast<float>(texInfo->width),
         o.glassAreaH / static_cast<float>(texInfo->height));
   else
      glassArea = vec4<float>(0.f, 0.f, 1.f, 1.f);
   vec4<float> glassAmbient(PinballPlugin::OverlayInvsRGB(o.glassAmbientR), PinballPlugin::OverlayInvsRGB(o.glassAmbientG), PinballPlugin::OverlayInvsRGB(o.glassAmbientB), 1.f);
   vec4<float> glassTint(o.glassTintR, o.glassTintG, o.glassTintB, 1.f);
   vec4<float> glassPad(o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB);
   vec4<float> dmdTint(o.tintR, o.tintG, o.tintB, 1.f);
   VPXDisplayRenderStyle style;
   if (!PinballPlugin::OverlayDisplayStyleToVPX(o.style, style))
   {
      style = VPXDisplayRenderStyle::VPXDMDStyle_Plasma;
      switch (dmd.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK)
      {
      case CTLPI_DISPLAY_HARDWARE_NEON_PLASMA: style = VPXDisplayRenderStyle::VPXDMDStyle_Plasma; break;
      case CTLPI_DISPLAY_HARDWARE_RED_LED: style = VPXDisplayRenderStyle::VPXDMDStyle_RedLED; break;
      case CTLPI_DISPLAY_HARDWARE_RGB_LED: style = VPXDisplayRenderStyle::VPXDMDStyle_GenLED; break;
      case CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY: style = VPXDisplayRenderStyle::VPXDMDStyle_CRT; break;
      case CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY: style = VPXDisplayRenderStyle::VPXDMDStyle_Pixelated; break;
      }
   }
   ctx->DrawDisplay(ctx, style,
      // First layer: glass
      m_glassTex, glassTint.x, glassTint.y, glassTint.z, o.glassRoughness, // Glass texture, tint and roughness
      glassArea.x, glassArea.y, glassArea.z, glassArea.w, // Glass texture coordinates (inside overall glass texture)
      glassAmbient.x, glassAmbient.y, glassAmbient.z, // Glass lighting from room
      // Second layer: emitter
      m_dmdTex, dmdTint.x, dmdTint.y, dmdTint.z, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
      glassPad.x, glassPad.y, glassPad.z, glassPad.w, // Emitter padding (from glass border)
      // Render quad
      static_cast<float>(m_frame.x), static_cast<float>(m_frame.y), static_cast<float>(m_frame.z), static_cast<float>(m_frame.w));
}

DMDOverlay::vec4<int> DMDOverlay::SearchDmdSubFrame(VPXTexture image, const VPXTextureInfo* const texInfo, float dmdAspectRatio) const
{
   unsigned int pos_step;
   switch (texInfo->format)
   {
   case VPXTEXFMT_BW32F: pos_step = 1; break;
   case VPXTEXFMT_sRGB8: pos_step = 3; break;
   case VPXTEXFMT_sRGBA8: pos_step = 4; break;
   default: pos_step = 0;
   }

   // Heuristic to select large rectangles, favoring screen centered ones
   auto heuristic = [texW = static_cast<float>(texInfo->width)](const vec4<int>& frame)
   { return static_cast<float>(frame.z) / texW - 1.f * fabs(0.5f - static_cast<float>(frame.x + frame.z / 2) / texW); };

   const int padding = (2 * texInfo->width) / 1920;
   float lumMin = 0.f;
   float lumMax = 256.f;
   vec4<int> searchFrame(0, 0, texInfo->width, texInfo->height);
   vec4<int> bestFrame;
   for (int search = 0; search < 7; search++)
   {
      const float lumLimit = (lumMin + lumMax) * 0.5f;

      LOGD(std::format("DMD area search thr: {} area is {},{} {}x{}", lumLimit, searchFrame.x, searchFrame.y, searchFrame.z, searchFrame.w));

      // Find the largest dark rectangle in the background image
      vec4<int> subFrame;
      vec4<int> searchSubFrame;
      std::stack<int> st;
      vector<int> heights(texInfo->width, 0); // height of empty columns above each pixels in the row as we scan them downward
      for (int y = searchFrame.y; y < (searchFrame.y + searchFrame.w); ++y)
      {
         // If disabled while searching, just abort
         if (m_stopSearching)
            return vec4<int>();
         unsigned int pos = (y * texInfo->width + searchFrame.x) * pos_step;
         for (int x = searchFrame.x; x < (searchFrame.x + searchFrame.z); ++x, pos += pos_step)
         {
            // FIXME the two branches are not comparable, yet both are thresholded against the
            // same lumLimit: BW32F is a linear luminance scaled by 255, the sRGB one a gamma
            // encoded luma, with Rec.601 weights where the rest of the codebase uses Rec.709.
            // A linear 0.5 and an sRGB 128 both land near 128 but are very different
            // brightnesses (0.5 against 0.216 linear). Only used to find lit dots for
            // cropping, and lumLimit was tuned against one of the two, so fixing this means
            // revisiting that threshold rather than correcting the math on its own
            float lum = 0;
            switch (texInfo->format)
            {
            case VPXTEXFMT_BW32F: lum = 255.f * static_cast<float*>(texInfo->data)[pos]; break;

            case VPXTEXFMT_sRGB8:
            case VPXTEXFMT_sRGBA8:
               lum = 0.299f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos]) + 0.587f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos + 1])
                  + 0.114f * static_cast<float>(static_cast<uint8_t*>(texInfo->data)[pos + 2]);
               break;

            default: return vec4<int>();
            }
            if (lum < lumLimit)
               heights[x] += 1;
            else
               heights[x] = 0;
         }

         // Evaluate each rectangle that can be formed with the heights of the columns
         for (int x = searchFrame.x; x <= searchFrame.x + searchFrame.z; ++x)
         {
            while (!st.empty() && (x == (searchFrame.x + searchFrame.z) || heights[st.top()] > heights[x]))
            {
               const int height = heights[st.top()];
               st.pop();
               const int width = st.empty() ? (x - searchFrame.x) : (x - st.top() - 1);
               if (width > 6 * padding && height > 3 * padding)
               {
                  vec4<int> unpaddedFrame;
                  unpaddedFrame.x = st.empty() ? searchFrame.x : st.top() + 1;
                  unpaddedFrame.y = y - height + 1;
                  unpaddedFrame.z = width;
                  unpaddedFrame.w = height;
                  // Extends search frame for next pass to include this area even if not selected
                  if (searchSubFrame.z == 0)
                     searchSubFrame = unpaddedFrame;
                  else
                  {
                     vec4<int> newSearchFrame;
                     newSearchFrame.x = std::min(searchSubFrame.x, unpaddedFrame.x);
                     newSearchFrame.y = std::min(searchSubFrame.y, unpaddedFrame.y);
                     newSearchFrame.z = std::max(searchSubFrame.x + searchSubFrame.z, unpaddedFrame.x + unpaddedFrame.z) - newSearchFrame.x;
                     newSearchFrame.w = std::max(searchSubFrame.y + searchSubFrame.w, unpaddedFrame.y + unpaddedFrame.w) - newSearchFrame.y;
                     searchSubFrame = newSearchFrame;
                  }
                  // Add some padding and fit to searched aspect ratio
                  vec4<int> frame;
                  frame.x = unpaddedFrame.x + padding;
                  frame.y = unpaddedFrame.y + padding;
                  frame.z = unpaddedFrame.z - 2 * padding;
                  frame.w = unpaddedFrame.w - 2 * padding;
                  if (const float ar = static_cast<float>(frame.z) / static_cast<float>(frame.w); ar > dmdAspectRatio)
                  {
                     const int w = static_cast<int>(static_cast<float>(frame.w) * dmdAspectRatio);
                     frame.x += (frame.z - w) / 2;
                     frame.z = w;
                  }
                  else
                  {
                     const int h = static_cast<int>(static_cast<float>(frame.z) / dmdAspectRatio);
                     frame.y += (frame.w - h) / 2;
                     frame.w = h;
                  }
                  // Selected area must be large enough (at least 1/3 of the backglass width)
                  if ((3 * frame.z >= static_cast<int>(texInfo->width)) && (heuristic(frame) > heuristic(subFrame)))
                     subFrame = frame;
               }
            }
            if (x < (searchFrame.x + searchFrame.z))
               st.push(x);
         }
      }

      LOGD(std::format("DMD area search {} -> {}, thr: {} lead to {},{} {}x{} Heur:{}, PrevHeur: {}", lumMin, lumMax, lumLimit, subFrame.x, subFrame.y, subFrame.z, subFrame.w,
         heuristic(subFrame), heuristic(bestFrame)));

      // There are no heuristics between luminance levels, this can lead to unwanted behavior (for example drifting inside a box)
      if (subFrame.w > 0) // && (bestFrame.w == 0 || (heuristic(subFrame) > 0.6f * heuristic(bestFrame))))
      {
         lumMax = lumLimit;
         bestFrame = subFrame;
         searchFrame = searchSubFrame;
      }
      else
      {
         lumMin = lumLimit;
      }
   }

   return bestFrame;
}


///////////////////////////////////////////////////////////////////////////////
// Document editing page (PagedUIPlugin): the root page enumerates the <DMD> children of
// the document's <Displays> node, one section per display with an 'options' subpage
// editing the render style and the glass layer, plus 'add' and Nixie 'import' actions.
// Property item contexts directly point at the edited field of the display entries of
// doc->displays. CommitEdit patches the document file.

namespace
{

   bool ReadFileBytes(const std::filesystem::path& path, vector<uint8_t>& out)
   {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file.is_open())
         return false;
      const std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      out.resize(static_cast<size_t>(size));
      file.read(reinterpret_cast<char*>(out.data()), size);
      return file.good() || file.gcount() == size;
   }

   // Lazily resolves the pixel size of the embedded glass image, cached against a hash of
   // its encoded content: the image area fields need the image bounds at enumeration time
   void GlassImageSize(B2SLegacy::B2SDMDDisplay& display)
   {
      size_t hash = 14695981039346656037ull;
      for (const uint8_t byte : display.glassImage)
         hash = (hash ^ byte) * 1099511628211ull;
      if (hash == display.glassImgHash)
         return;
      display.glassImgHash = hash;
      display.glassImgW = display.glassImgH = 0;
      if (!display.glassImage.empty() && display.doc != nullptr && display.doc->vpxApi != nullptr)
         if (VPXTexture texture = display.doc->vpxApi->CreateTexture(display.glassImage.data(), static_cast<int>(display.glassImage.size())))
         {
            if (const VPXTextureInfo* const info = display.doc->vpxApi->GetTextureInfo(texture))
            {
               display.glassImgW = static_cast<int>(info->width);
               display.glassImgH = static_cast<int>(info->height);
            }
            display.doc->vpxApi->DeleteTexture(texture);
         }
   }

   int MSGPIAPI OverlayFieldGet(void* context) { return *static_cast<int*>(context); }
   void MSGPIAPI OverlayFieldSet(void* context, int value) { *static_cast<int*>(context) = value; }
   const char* MSGPIAPI OverlaySourceGet(void* context) { return static_cast<B2SLegacy::B2SDMDDisplay*>(context)->source.c_str(); }
   void MSGPIAPI OverlaySourceSet(void* context, const char* value) { static_cast<B2SLegacy::B2SDMDDisplay*>(context)->source = value != nullptr ? value : ""; }
   // The embedded image has no file path to display: Get reports the empty string
   const char* MSGPIAPI OverlayGlassGet(void*) { return ""; }
   void MSGPIAPI OverlayGlassSet(void* context, const char* value)
   {
      // Get always reports an empty path (the image is embedded), so empty values are
      // also emitted when a pick is reverted: only an actual pick updates the image
      if (value == nullptr || *value == '\0')
         return;
      auto* const display = static_cast<B2SLegacy::B2SDMDDisplay*>(context);
      vector<uint8_t> image;
      if (!ReadFileBytes(PluginStrings::PathFromNative(value), image))
      {
         LOGE("Failed to read glass image file: "s + value);
         return;
      }
      display->glassImage = std::move(image);
      if (display->doc != nullptr && display->doc->requestRefresh)
         display->doc->requestRefresh();
   }
   void MSGPIAPI OverlayGlassRemoveRun(void* context)
   {
      auto* const display = static_cast<B2SLegacy::B2SDMDDisplay*>(context);
      display->glassImage.clear();
      if (display->doc != nullptr && display->doc->requestRefresh)
         display->doc->requestRefresh();
   }
   const char* MSGPIAPI OverlayImportGet(void* context) { return static_cast<B2SLegacy::B2SOverlayDoc*>(context)->importPath.c_str(); }
   void MSGPIAPI OverlayImportSet(void* context, const char* value) { static_cast<B2SLegacy::B2SOverlayDoc*>(context)->importPath = value != nullptr ? value : ""; }
   void MSGPIAPI OverlayImportRun(void* context)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      if (doc->importPath.empty())
         return;
      const std::filesystem::path nixPath = PluginStrings::PathFromNative(doc->importPath.c_str());
      std::vector<PinballPlugin::NixieFile::Layout> layouts;
      string error;
      if (!PinballPlugin::NixieFile::Parse(nixPath, layouts, error))
      {
         LOGE(error);
         return;
      }
      for (const PinballPlugin::NixieFile::Layout& layout : layouts)
      {
         const int target = layout.window == VPXWindowId::VPXWINDOW_ScoreView ? 1 : layout.window == VPXWindowId::VPXWINDOW_Backglass ? 0 : -1;
         if (target < 0)
            continue;
         const int boundW = target == 1 ? doc->scoreViewW : doc->backglassW;
         const int boundH = target == 1 ? doc->scoreViewH : doc->backglassH;
         const float dstW = boundW > 0 ? static_cast<float>(boundW) : layout.width;
         const float dstH = boundH > 0 ? static_cast<float>(boundH) : layout.height;
         for (const PinballPlugin::NixieFile::Visual& visual : layout.visuals)
         {
            if (visual.type != PinballPlugin::NixieFile::DMD && visual.type != PinballPlugin::NixieFile::Screen)
               continue;
            B2SLegacy::B2SDMDDisplay& display = doc->displays.emplace_back();
            display.doc = doc;
            display.target = target;
            display.source = visual.srcUri == "ctrl://default/display" ? "" : visual.srcUri;
            display.x = static_cast<int>(std::lround(visual.x * dstW / layout.width));
            display.y = static_cast<int>(std::lround(visual.y * dstH / layout.height));
            display.w = static_cast<int>(std::lround(visual.w * dstW / layout.width));
            display.h = static_cast<int>(std::lround(visual.h * dstH / layout.height));
            PinballPlugin::ApplyNixieVisualOptions(display.options, visual);
            if (!visual.glassPath.empty())
            {
               const std::filesystem::path glassPath = nixPath.parent_path() / PluginStrings::PathFromUTF8(visual.glassPath);
               if (!ReadFileBytes(glassPath, display.glassImage))
                  LOGE("Failed to read glass image file: " + PluginStrings::PathToUTF8(glassPath));
            }
         }
      }
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayDetectRun(void* context) { static_cast<B2SLegacy::B2SDMDDisplay*>(context)->detectRequest = 1; }
   void MSGPIAPI OverlayRemoveRun(void* context)
   {
      auto* const display = static_cast<B2SLegacy::B2SDMDDisplay*>(context);
      B2SLegacy::B2SOverlayDoc* const doc = display->doc;
      std::erase_if(doc->displays, [display](const B2SLegacy::B2SDMDDisplay& d) { return &d == display; });
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayAddRun(void* context)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      B2SLegacy::B2SDMDDisplay& display = doc->displays.emplace_back();
      display.doc = doc;
      display.options.glassRoughness = 0.1f;
      if (doc->requestRefresh)
         doc->requestRefresh();
   }

   // Edit session: the host only tracks item value changes through its docSnapshot, so the document
   // itself is captured here to detect (and discard) structural changes such as added/removed displays
   void MSGPIAPI OverlayBeginEdit(void* context)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      doc->editBackup = doc->displays;
   }

   // Persisted fields only: detectRequest and the doc back link are runtime state
   static bool SameDisplay(const B2SLegacy::B2SDMDDisplay& a, const B2SLegacy::B2SDMDDisplay& b)
   {
      return a.target == b.target && a.source == b.source && a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h //
         && a.glassImage == b.glassImage && PinballPlugin::SameOverlayDisplayOptions(a.options, b.options);
   }

   int MSGPIAPI OverlayIsDirty(void* context)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      return doc->displays.size() != doc->editBackup.size() || !std::equal(doc->displays.begin(), doc->displays.end(), doc->editBackup.begin(), SameDisplay);
   }

   void MSGPIAPI OverlayEndEdit(void* context, int committed)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      // Discard uncommitted changes: item values are reverted through their Set callbacks, this covers
      // the structural changes (added/removed displays) including edits made after a save in the same
      // session (the backup is re-based on each successful commit)
      if (!committed || OverlayIsDirty(context))
      {
         doc->displays = doc->editBackup;
         for (B2SLegacy::B2SDMDDisplay& display : doc->displays)
            display.doc = doc;
      }
      doc->editBackup.clear();
   }

   unsigned int MSGPIAPI OverlayPageCount(void* context) { return 1 + static_cast<unsigned int>(static_cast<B2SLegacy::B2SOverlayDoc*>(context)->displays.size()); }
   void MSGPIAPI OverlayPageInfo(void*, unsigned int pageIndex, PagedUIPageInfo* info)
   {
      // Generated ids are interned for the duration of the enumeration call (see GetItems)
      static std::deque<string> strings;
      strings.clear();
      const auto str = [](string s) { return strings.emplace_back(std::move(s)).c_str(); };
      info->id = pageIndex == 0 ? "displays" : str(std::format("dmd{}", pageIndex - 1));
      info->title = pageIndex == 0 ? nullptr : str(std::format("DMD display #{}", pageIndex));
      info->info = nullptr;
   }

   // Root page: 'add' action, 10 items per display, then the 3 'import' items
   constexpr unsigned int kRootDisplayItems = 10;
   unsigned int MSGPIAPI OverlayItemCount(void* context, const char* pageId)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      // Display subpage: shared display option items (incl. the glass section header), shared
      // glass option items (image area fields only when a glass image is defined), then the
      // image file picker or the remove action
      if (strncmp(pageId, "dmd", 3) == 0)
      {
         const unsigned int i = static_cast<unsigned int>(atoi(pageId + 3));
         if (i >= doc->displays.size())
            return 0;
         B2SLegacy::B2SDMDDisplay& display = doc->displays[i];
         GlassImageSize(display);
         return PinballPlugin::OverlayUI::OverlayDisplayOptionItemCount + PinballPlugin::OverlayUI::OverlayGlassOptionItemCount(display.glassImage.empty() ? 0 : 1) + 1;
      }
      return static_cast<unsigned int>(doc->displays.size()) * kRootDisplayItems + 4;
   }

   void FillOverlayItem(PagedUIItemDef& item, const char* id, const char* label, const char* tooltip)
   {
      item = PagedUIItemDef {};
      item.id = id;
      item.label = label;
      item.tooltip = tooltip;
      item.storage = PAGEDUI_STORAGE_PLUGIN;
   }

   void FillOverlayInt(PagedUIItemDef& item, const char* id, const char* label, const char* tooltip, int* field, int maxVal)
   {
      FillOverlayItem(item, id, label, tooltip);
      item.type = PAGEDUI_ITEM_INT;
      item.context = field;
      item.intProp.minVal = 0;
      item.intProp.maxVal = maxVal;
      item.intProp.defVal = 0;
      item.intProp.format = nullptr;
      item.intProp.Get = &OverlayFieldGet;
      item.intProp.Set = &OverlayFieldSet;
   }

   // Options subpage of one display: render options, then the glass section (shared
   // glass option items, then the image file picker embedded in the document)
   void FillDisplayPageItems(PagedUIItemDef* items, B2SLegacy::B2SDMDDisplay& display, unsigned int i, const std::function<const char*(string)>& str)
   {
      const string prefix = std::format("dmd{}.", i);
      PagedUIItemDef* it = items;
      GlassImageSize(display);
      it += PinballPlugin::OverlayUI::FillDisplayOptionItems(it, str, prefix.c_str(), &display.options);
      // When a glass is defined the texture section only shows the image area fields (bounded by
      // the decoded image size, with a fallback range when it cannot be resolved) and the remove
      // action; the file picker replaces them when there is no glass image
      const bool hasGlass = !display.glassImage.empty();
      it += PinballPlugin::OverlayUI::FillDisplayGlassItems(it, str, prefix.c_str(), &display.options,
         hasGlass && display.glassImgW > 0 ? display.glassImgW
            : hasGlass                     ? 0xFFFF
                                           : 0,
         hasGlass && display.glassImgH > 0 ? display.glassImgH
            : hasGlass                     ? 0xFFFF
                                           : 0);
      if (hasGlass)
      {
         FillOverlayItem(it[0], str(prefix + "glassRemove"), "Remove glass image", "Remove the embedded glass image from the display");
         it[0].type = PAGEDUI_ITEM_ACTION;
         it[0].context = &display;
         it[0].action.Run = &OverlayGlassRemoveRun;
      }
      else
      {
         FillOverlayItem(it[0], str(prefix + "glassFile"), "Glass image", "Pick an image file to embed in the document as the glass covering the display");
         it[0].type = PAGEDUI_ITEM_FILE;
         it[0].context = &display;
         it[0].fileProp.defVal = "";
         it[0].fileProp.fileFilter = "png;jpg;jpeg;webp;bmp";
         it[0].fileProp.Get = &OverlayGlassGet;
         it[0].fileProp.Set = &OverlayGlassSet;
      }
   }

   void MSGPIAPI OverlayGetItems(void* context, const char* pageId, PagedUIItemDef* items)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      // Generated ids/labels are kept in static storage: they are consumed by the host right
      // after this call and the callbacks only run on the API thread. A deque keeps the
      // pointers stable: item ids are the c_str() of these strings, read after the loop
      static std::deque<string> strings;
      strings.clear();
      const auto str = [](string s) { return strings.emplace_back(std::move(s)).c_str(); };
      if (strncmp(pageId, "dmd", 3) == 0)
      {
         const unsigned int i = static_cast<unsigned int>(atoi(pageId + 3));
         if (i < doc->displays.size())
            FillDisplayPageItems(items, doc->displays[i], i, str);
         return;
      }
      static const char* targetValues[] = { "Backglass", "Score view" };
      FillOverlayItem(items[0], "dmdAdd", "Add DMD display", "Add a DMD overlay to the backglass file");
      items[0].type = PAGEDUI_ITEM_ACTION;
      items[0].context = doc;
      items[0].action.Run = &OverlayAddRun;
      for (unsigned int i = 0; i < doc->displays.size(); i++)
      {
         B2SLegacy::B2SDMDDisplay& display = doc->displays[i];
         const int maxW = display.target == 1 ? (doc->scoreViewW > 0 ? doc->scoreViewW : 0xFFFF) : (doc->backglassW > 0 ? doc->backglassW : 0xFFFF);
         const int maxH = display.target == 1 ? (doc->scoreViewH > 0 ? doc->scoreViewH : 0xFFFF) : (doc->backglassH > 0 ? doc->backglassH : 0xFFFF);
         PagedUIItemDef* const it = items + 1 + i * kRootDisplayItems;
         FillOverlayItem(it[0], nullptr, str(std::format("DMD display #{}", i + 1)), nullptr);
         it[0].type = PAGEDUI_ITEM_HEADER;
         FillOverlayItem(it[1], str(std::format("dmd{}.target", i)), "Show on", "View the display is rendered on");
         it[1].type = PAGEDUI_ITEM_ENUM;
         it[1].context = &display.target;
         it[1].enumProp = { 0, 2, targetValues, 0, &OverlayFieldGet, &OverlayFieldSet };
         FillOverlayItem(it[2], str(std::format("dmd{}.source", i)), "Source", "Controller resource URI of the DMD to render (ctrl://..., empty = default DMD)");
         it[2].type = PAGEDUI_ITEM_STRING;
         it[2].context = &display;
         it[2].stringProp.defVal = "";
         it[2].stringProp.Get = &OverlaySourceGet;
         it[2].stringProp.Set = &OverlaySourceSet;
         FillOverlayItem(it[3], str(std::format("dmd{}.detect", i)), "Detect position", "Automatically detect the DMD bounds on the artwork image");
         it[3].type = PAGEDUI_ITEM_ACTION;
         it[3].context = &display;
         it[3].action.Run = &OverlayDetectRun;
         FillOverlayInt(it[4], str(std::format("dmd{}.x", i)), "X position", "Overlay bounds in artwork pixels, from the top left of the image", &display.x, maxW);
         FillOverlayInt(it[5], str(std::format("dmd{}.y", i)), "Y position", "Overlay bounds in artwork pixels, from the top left of the image (grill included)", &display.y, maxH);
         FillOverlayInt(it[6], str(std::format("dmd{}.w", i)), "Width", "Overlay bounds in artwork pixels", &display.w, maxW);
         FillOverlayInt(it[7], str(std::format("dmd{}.h", i)), "Height", "Overlay bounds in artwork pixels", &display.h, maxH);
         FillOverlayItem(it[8], str(std::format("dmd{}.options", i)), "Options", "Render style, tint and glass options of the display");
         it[8].type = PAGEDUI_ITEM_SUBPAGE;
         it[8].subpage.pageId = str(std::format("dmd{}", i));
         FillOverlayItem(it[9], str(std::format("dmd{}.remove", i)), "Remove display", "Remove this display from the backglass file");
         it[9].type = PAGEDUI_ITEM_ACTION;
         it[9].context = &display;
         it[9].action.Run = &OverlayRemoveRun;
      }
      PagedUIItemDef* const tail = items + 1 + doc->displays.size() * kRootDisplayItems;
      FillOverlayItem(tail[0], nullptr, str("Import from a Nixie layout file"s), nullptr);
      tail[0].type = PAGEDUI_ITEM_HEADER;
      FillOverlayItem(tail[1], "importNix", "Nixie file", "Nixie layout file (.nix) to import displays from");
      tail[1].type = PAGEDUI_ITEM_FILE;
      tail[1].context = doc;
      tail[1].fileProp.defVal = "";
      tail[1].fileProp.fileFilter = "nix";
      tail[1].fileProp.Get = &OverlayImportGet;
      tail[1].fileProp.Set = &OverlayImportSet;
      FillOverlayItem(tail[2], "importRun", "Import displays", "Append the DMD and screen visuals of the Nixie file to the document displays");
      tail[2].type = PAGEDUI_ITEM_ACTION;
      tail[2].context = doc;
      tail[2].action.Run = &OverlayImportRun;
   }

   // Rewrites only the <DMD> children of the document's <Displays> element, leaving all other
   // content (including the embedded base64 images) untouched
   int MSGPIAPI OverlayCommitEdit(void* context)
   {
      auto* const doc = static_cast<B2SLegacy::B2SOverlayDoc*>(context);
      if (doc->path.empty())
         return 1;
      std::ifstream in(doc->path, std::ios::binary);
      if (!in)
      {
         LOGE("Failed to open B2S file for writing: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      const std::string buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      in.close();
      tinyxml2::XMLDocument xmlDoc;
      if (xmlDoc.Parse(buffer.c_str(), buffer.size()) != tinyxml2::XML_SUCCESS)
      {
         LOGE("Failed to parse B2S file: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("DirectB2SData");
      if (root == nullptr)
         return 1;
      auto* displays = root->FirstChildElement("Displays");
      if (displays == nullptr)
         displays = static_cast<tinyxml2::XMLElement*>(root->InsertEndChild(xmlDoc.NewElement("Displays")));
      // Remove the previously authored DMD children; children of other display kinds are preserved
      while (tinyxml2::XMLElement* dmd = displays->FirstChildElement("DMD"))
         displays->DeleteChild(dmd);
      for (const B2SLegacy::B2SDMDDisplay& display : doc->displays)
      {
         auto* const e = xmlDoc.NewElement("DMD");
         e->SetAttribute("Target", display.target == 1 ? "ScoreView" : "Backglass");
         if (!display.source.empty())
            e->SetAttribute("Source", display.source.c_str());
         e->SetAttribute("X", display.x);
         e->SetAttribute("Y", display.y);
         e->SetAttribute("W", display.w);
         e->SetAttribute("H", display.h);
         const PinballPlugin::OverlayDisplayOptions& o = display.options;
         if (o.style > 0 && o.style <= 8)
            e->SetAttribute("Style", PinballPlugin::OverlayDisplayStyleNames[o.style - 1]);
         if (o.sizeW > 0 || o.sizeH > 0)
            e->SetAttribute("Dots", std::format("{},{}", o.sizeW, o.sizeH).c_str());
         if (o.tintR != 1.f || o.tintG != 1.f || o.tintB != 1.f)
         {
            const float tint[] = { o.tintR, o.tintG, o.tintB };
            e->SetAttribute("Tint", PinballPlugin::OverlayFormatFloats(tint, 3).c_str());
         }
         if (o.glassTintR != 1.f || o.glassTintG != 1.f || o.glassTintB != 1.f)
         {
            const float tint[] = { o.glassTintR, o.glassTintG, o.glassTintB };
            e->SetAttribute("GlassTint", PinballPlugin::OverlayFormatFloats(tint, 3).c_str());
         }
         if (o.glassRoughness != 0.f)
            e->SetAttribute("GlassRoughness", PinballPlugin::OverlayFormatFloats(&o.glassRoughness, 1).c_str());
         if (o.glassAmbientR != 1.f || o.glassAmbientG != 1.f || o.glassAmbientB != 1.f)
         {
            const float ambient[] = { o.glassAmbientR, o.glassAmbientG, o.glassAmbientB };
            e->SetAttribute("GlassAmbient", PinballPlugin::OverlayFormatFloats(ambient, 3).c_str());
         }
         if (o.glassAreaX != 0 || o.glassAreaY != 0 || o.glassAreaW != 0 || o.glassAreaH != 0)
            e->SetAttribute("GlassArea", std::format("{},{},{},{}", o.glassAreaX, o.glassAreaY, o.glassAreaW, o.glassAreaH).c_str());
         if (o.glassPadL != 0.f || o.glassPadT != 0.f || o.glassPadR != 0.f || o.glassPadB != 0.f)
         {
            const float pad[] = { o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB };
            e->SetAttribute("GlassPad", PinballPlugin::OverlayFormatFloats(pad, 4).c_str());
         }
         if (!display.glassImage.empty())
         {
            auto* const image = xmlDoc.NewElement("GlassImage");
            image->SetAttribute("Value", to_base64(display.glassImage.data(), display.glassImage.size()).c_str());
            e->InsertEndChild(image);
         }
         displays->InsertEndChild(e);
      }
      tinyxml2::XMLPrinter printer;
      xmlDoc.Print(&printer);
      std::ofstream out(doc->path, std::ios::binary | std::ios::trunc);
      if (!out)
      {
         LOGE("Failed to write B2S file: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      out << printer.CStr();
      out.close();
      if (out.fail())
      {
         LOGE("Failed to write B2S file: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      doc->editBackup = doc->displays; // Saved state becomes the new dirty baseline
      return 0;
   }

} // namespace


void DMDOverlay::InitEditSheetDef(PagedUISheetDef* sheet, B2SLegacy::B2SOverlayDoc* doc)
{
   sheet->id = "backglass";
   sheet->context = doc;
   sheet->GetPageCount = &OverlayPageCount;
   sheet->GetPageInfo = &OverlayPageInfo;
   sheet->GetItemCount = &OverlayItemCount;
   sheet->GetItems = &OverlayGetItems;
   sheet->BeginEdit = &OverlayBeginEdit;
   sheet->CommitEdit = &OverlayCommitEdit;
   sheet->EndEdit = &OverlayEndEdit;
   sheet->IsDirty = &OverlayIsDirty;
}
}