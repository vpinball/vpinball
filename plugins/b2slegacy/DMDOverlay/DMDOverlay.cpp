// license:GPLv3+

#include "DMDOverlay.h"

// Logging goes through the Doc::log hook (bound by the plugin): only the level constants and
// the function macro of the shared logging support are used here
#include "plugins/LoggingPlugin.h"
#include "plugins/PluginStrings.h"

#include "nixie/NixieLayoutFile.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <format>
#include <fstream>
#include <iterator>
#include <stack>
#include <string>
#include <vector>

using std::string;
using namespace std::string_literals;
using std::vector;


namespace DMDOverlay
{

///////////////////////////////////////////////////////////////////////////////
// Renderer: resolves the display source through the ResURIResolver, uploads new frames to its
// own texture, maintains a per-display glass texture (encoded content provided through the
// Doc::glassBytes hook) and draws both layers in the quad mapped by Doc::mapQuad.

Renderer::Renderer(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver)
   : m_resURIResolver(resURIResolver)
   , m_vpxApi(vpxApi)
{
}

Renderer::~Renderer()
{
   m_stopSearching = true;
   if (m_frameSearch.valid())
      m_frameSearch.wait();
   if (m_glassTex != nullptr && m_vpxApi != nullptr)
      m_vpxApi->DeleteTexture(m_glassTex);
   if (m_dmdTex != nullptr && m_vpxApi != nullptr)
      m_vpxApi->DeleteTexture(m_dmdTex);
}

void Renderer::Render(VPXRenderContext2D* ctx, Display& display, VPXTexture artwork)
{
   // The display is enabled by its sole presence in the document
   PinballPlugin::ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState(display.source.empty() ? PinballPlugin::ResURIResolver::DefaultDmdUri : display.source);
   if (dmd.state.frame == nullptr)
      return;

   Doc* const doc = display.doc;

   // Bounds detection request, issued by the 'Detect position' action of the editing page:
   // the detected frame is written back into the document fields (see below)
   if (display.detectRequest != 0 && !m_frameSearch.valid() && artwork != nullptr)
   {
      display.detectRequest = 0;
      const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(artwork);
      if (texInfo != nullptr)
      {
         m_stopSearching = false;
         const float ar = display.options.sizeW > 0 && display.options.sizeH > 0 ? static_cast<float>(display.options.sizeW) / static_cast<float>(display.options.sizeH)
                                                                                 : static_cast<float>(dmd.source->width) / static_cast<float>(dmd.source->height);
         const LogFn log = doc != nullptr ? doc->log : nullptr;
         m_frameSearch = std::async(std::launch::async, [this, artwork, texInfo, ar, log]() { return SearchDmdSubFrame(artwork, texInfo, ar, log); });
      }
   }
   if (m_frameSearch.valid() && m_frameSearch.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
   {
      const vec4<int> detected = m_frameSearch.get();
      if (detected.z != 0 && detected.w != 0)
      {
         display.x = detected.x;
         display.y = detected.y;
         display.w = detected.z;
         display.h = detected.w;
      }
   }

   // Map the authored bounds (document space, origin at the top left) to the render quad
   // (bottom-up render space of the view) through the plugin hook; the default maps the
   // bounds against the whole view, flipping the origin
   const vec4<int> frame
      = doc != nullptr && doc->mapQuad != nullptr ? doc->mapQuad(ctx, display) : vec4<int>(display.x, static_cast<int>(ctx->srcHeight) - display.y - display.h, display.w, display.h);
   if (frame.z == 0 || frame.w == 0)
      return;
   if (frame.y + frame.w <= 0 || frame.y >= static_cast<int>(ctx->srcHeight))
      return; // Fully clipped away or off the view

   // The texture is owned by the plugin, hence the null test: it may still have to be created here
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

   // The glass texture is owned by the overlay: (re)create it when the content provided by the
   // Doc::glassBytes hook changes (identified by the key the hook maintains on the display)
   const vector<uint8_t>* const glass = doc != nullptr && doc->glassBytes != nullptr ? doc->glassBytes(display) : nullptr;
   if (glass != nullptr && !glass->empty() && display.glassKey != 0)
   {
      if (display.glassKey != m_glassHash || m_glassTex == nullptr)
      {
         if (m_glassTex != nullptr)
            m_vpxApi->DeleteTexture(m_glassTex);
         m_glassTex = m_vpxApi->CreateTexture(const_cast<uint8_t*>(glass->data()), static_cast<int>(glass->size()));
         m_glassHash = m_glassTex != nullptr ? display.glassKey : 0;
      }
   }
   else if (m_glassTex != nullptr)
   {
      m_vpxApi->DeleteTexture(m_glassTex);
      m_glassTex = nullptr;
      m_glassHash = 0;
   }

   const PinballPlugin::OverlayDisplayOptions& o = display.options;
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
      static_cast<float>(frame.x), static_cast<float>(frame.y), static_cast<float>(frame.z), static_cast<float>(frame.w));
}

vec4<int> Renderer::SearchDmdSubFrame(VPXTexture image, const VPXTextureInfo* const texInfo, float dmdAspectRatio, const LogFn& log) const
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

      if (log != nullptr)
         log(LPI_FUNC, __LINE__, LPI_LVL_DEBUG, std::format("DMD area search thr: {} area is {},{} {}x{}", lumLimit, searchFrame.x, searchFrame.y, searchFrame.z, searchFrame.w));

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

      if (log != nullptr)
         log(LPI_FUNC, __LINE__, LPI_LVL_DEBUG,
            std::format("DMD area search {} -> {}, thr: {} lead to {},{} {}x{} Heur:{}, PrevHeur: {}", lumMin, lumMax, lumLimit, subFrame.x, subFrame.y, subFrame.z, subFrame.w,
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
// Document editing page (PagedUIPlugin): the root page enumerates the displays of the
// document, one section per display with an 'options' subpage editing the render style and
// the glass layer, plus 'add' and Nixie 'import' actions. Property item contexts directly
// point at the edited field of the display entries of doc->displays. CommitEdit delegates
// the document rewrite to the plugin-provided Doc::commitEdit hook.

namespace
{

   bool ReadFileBytesImpl(const std::filesystem::path& path, vector<uint8_t>& out)
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

   // Lazily resolves the pixel size of the glass image, cached against the content key
   // maintained by the Doc::glassBytes hook: the image area fields need the image bounds
   // at enumeration time
   void GlassImageSize(Display& display)
   {
      Doc* const doc = display.doc;
      const vector<uint8_t>* const glass = doc != nullptr && doc->glassBytes != nullptr ? doc->glassBytes(display) : nullptr;
      const size_t key = glass != nullptr && !glass->empty() ? display.glassKey : 0;
      if (key == display.glassSizeKey)
         return;
      display.glassSizeKey = key;
      display.glassImgW = display.glassImgH = 0;
      if (key != 0 && doc != nullptr && doc->vpxApi != nullptr)
         if (VPXTexture texture = doc->vpxApi->CreateTexture(const_cast<uint8_t*>(glass->data()), static_cast<int>(glass->size())))
         {
            if (const VPXTextureInfo* const info = doc->vpxApi->GetTextureInfo(texture))
            {
               display.glassImgW = static_cast<int>(info->width);
               display.glassImgH = static_cast<int>(info->height);
            }
            doc->vpxApi->DeleteTexture(texture);
         }
   }

   int MSGPIAPI OverlayFieldGet(void* context) { return *static_cast<int*>(context); }
   void MSGPIAPI OverlayFieldSet(void* context, int value) { *static_cast<int*>(context) = value; }
   const char* MSGPIAPI OverlaySourceGet(void* context) { return static_cast<Display*>(context)->source.c_str(); }
   void MSGPIAPI OverlaySourceSet(void* context, const char* value) { static_cast<Display*>(context)->source = value != nullptr ? value : ""; }
   // The picker reports the staged pick (empty when the glass storage has no file
   // representation, e.g. an embedded image)
   const char* MSGPIAPI OverlayGlassGet(void* context) { return static_cast<Display*>(context)->glassPending.c_str(); }
   void MSGPIAPI OverlayGlassSet(void* context, const char* value)
   {
      auto* const display = static_cast<Display*>(context);
      Doc* const doc = display->doc;
      if (value == nullptr || *value == '\0')
      {
         // Reverting to the snapshot value also lands here: drop the staged pick if any
         // (the 'Remove glass image' action drops the glass image entirely)
         display->glassPending.clear();
      }
      else if (doc == nullptr || doc->setGlass == nullptr || !doc->setGlass(*display, PluginStrings::PathFromNative(value)))
         return;
      if (doc != nullptr && doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayGlassRemoveRun(void* context)
   {
      auto* const display = static_cast<Display*>(context);
      if (display->doc != nullptr && display->doc->clearGlass != nullptr)
         display->doc->clearGlass(*display);
      if (display->doc != nullptr && display->doc->requestRefresh)
         display->doc->requestRefresh();
   }
   const char* MSGPIAPI OverlayImportGet(void* context) { return static_cast<Doc*>(context)->importPath.c_str(); }
   void MSGPIAPI OverlayImportSet(void* context, const char* value) { static_cast<Doc*>(context)->importPath = value != nullptr ? value : ""; }
   void MSGPIAPI OverlayImportRun(void* context)
   {
      auto* const doc = static_cast<Doc*>(context);
      if (doc->importPath.empty())
         return;
      const std::filesystem::path nixPath = PluginStrings::PathFromNative(doc->importPath.c_str());
      std::vector<PinballPlugin::NixieFile::Layout> layouts;
      string error;
      if (!PinballPlugin::NixieFile::Parse(nixPath, layouts, error))
      {
         if (doc->log != nullptr)
            doc->log(LPI_FUNC, __LINE__, LPI_LVL_ERROR, error);
         return;
      }
      for (const PinballPlugin::NixieFile::Layout& layout : layouts)
      {
         const int target = layout.window == VPXWindowId::VPXWINDOW_ScoreView ? 1
            : layout.window == VPXWindowId::VPXWINDOW_Topper                  ? 2
            : layout.window == VPXWindowId::VPXWINDOW_Backglass               ? 0
                                                                              : -1;
         if (target < 0 || target >= doc->targetCount)
            continue;
         const float dstW = doc->viewW[target] > 0 ? static_cast<float>(doc->viewW[target]) : layout.width;
         const float dstH = doc->viewH[target] > 0 ? static_cast<float>(doc->viewH[target]) : layout.height;
         for (const PinballPlugin::NixieFile::Visual& visual : layout.visuals)
         {
            if (visual.type != PinballPlugin::NixieFile::DMD && visual.type != PinballPlugin::NixieFile::Screen)
               continue;
            Display& display = doc->displays.emplace_back();
            display.doc = doc;
            display.target = target;
            display.source = visual.srcUri == "ctrl://default/display" ? "" : visual.srcUri;
            display.x = static_cast<int>(std::lround(visual.x * dstW / layout.width));
            display.y = static_cast<int>(std::lround(visual.y * dstH / layout.height));
            display.w = static_cast<int>(std::lround(visual.w * dstW / layout.width));
            display.h = static_cast<int>(std::lround(visual.h * dstH / layout.height));
            PinballPlugin::ApplyNixieVisualOptions(display.options, visual);
            if (!visual.glassPath.empty() && doc->setGlass != nullptr)
               doc->setGlass(display, nixPath.parent_path() / PluginStrings::PathFromUTF8(visual.glassPath));
         }
      }
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayDetectRun(void* context) { static_cast<Display*>(context)->detectRequest = 1; }
   void MSGPIAPI OverlayRemoveRun(void* context)
   {
      auto* const display = static_cast<Display*>(context);
      Doc* const doc = display->doc;
      if (doc == nullptr)
         return;
      std::erase_if(doc->displays, [display](const Display& d) { return &d == display; });
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayAddRun(void* context)
   {
      auto* const doc = static_cast<Doc*>(context);
      Display& display = doc->displays.emplace_back();
      display.doc = doc;
      display.options.glassRoughness = 0.1f;
      if (doc->requestRefresh)
         doc->requestRefresh();
   }

   // Edit session: the host only tracks item value changes through its docSnapshot, so the document
   // itself is captured here to detect (and discard) structural changes such as added/removed displays
   void MSGPIAPI OverlayBeginEdit(void* context)
   {
      auto* const doc = static_cast<Doc*>(context);
      doc->editBackup = doc->displays;
   }

   // Persisted fields only: detectRequest, the glass content cache and the doc back link are runtime state
   static bool SameDisplay(const Display& a, const Display& b)
   {
      return a.target == b.target && a.source == b.source && a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h //
         && a.glassImage == b.glassImage && a.glassPath == b.glassPath && a.glassPending == b.glassPending //
         && PinballPlugin::SameOverlayDisplayOptions(a.options, b.options);
   }

   int MSGPIAPI OverlayIsDirty(void* context)
   {
      auto* const doc = static_cast<Doc*>(context);
      return doc->displays.size() != doc->editBackup.size() || !std::equal(doc->displays.begin(), doc->displays.end(), doc->editBackup.begin(), SameDisplay);
   }

   void MSGPIAPI OverlayEndEdit(void* context, int committed)
   {
      auto* const doc = static_cast<Doc*>(context);
      // Discard uncommitted changes: item values are reverted through their Set callbacks, this covers
      // the structural changes (added/removed displays) including edits made after a save in the same
      // session (the backup is re-based on each successful commit)
      if (!committed || OverlayIsDirty(context))
      {
         doc->displays = doc->editBackup;
         for (Display& display : doc->displays)
            display.doc = doc;
      }
      doc->editBackup.clear();
   }

   unsigned int MSGPIAPI OverlayPageCount(void* context) { return 1 + static_cast<unsigned int>(static_cast<Doc*>(context)->displays.size()); }
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

   // Root page: 'add' action, the per-display items ('Detect position' only when the plugin
   // detects bounds on an artwork), then the 3 'import' items
   unsigned int RootDisplayItemCount(const Doc* doc) { return doc->detectable ? 10 : 9; }

   unsigned int MSGPIAPI OverlayItemCount(void* context, const char* pageId)
   {
      auto* const doc = static_cast<Doc*>(context);
      // Display subpage: shared display option items (incl. the glass section header), shared
      // glass option items (image area fields only when a glass image is defined), then the
      // image file picker or the remove action
      if (strncmp(pageId, "dmd", 3) == 0)
      {
         const unsigned int i = static_cast<unsigned int>(atoi(pageId + 3));
         if (i >= doc->displays.size())
            return 0;
         Display& display = doc->displays[i];
         GlassImageSize(display);
         const bool hasGlass = doc->hasGlass != nullptr && doc->hasGlass(display);
         return PinballPlugin::OverlayUI::OverlayDisplayOptionItemCount + PinballPlugin::OverlayUI::OverlayGlassOptionItemCount(hasGlass ? 1 : 0) + 1;
      }
      return static_cast<unsigned int>(doc->displays.size()) * RootDisplayItemCount(doc) + 4;
   }

   // Options subpage of one display: render options, then the glass section (shared glass
   // option items, then the image file picker or the remove action)
   void FillDisplayPageItems(PagedUIItemDef* items, Display& display, unsigned int i, const std::function<const char*(string)>& str)
   {
      const string prefix = std::format("dmd{}.", i);
      PagedUIItemDef* it = items;
      GlassImageSize(display);
      it += PinballPlugin::OverlayUI::FillDisplayOptionItems(it, str, prefix.c_str(), &display.options);
      // When a glass is defined the texture section only shows the image area fields (bounded by
      // the decoded image size, with a fallback range when it cannot be resolved) and the remove
      // action; the file picker replaces them when there is no glass image
      Doc* const doc = display.doc;
      const bool hasGlass = doc != nullptr && doc->hasGlass != nullptr && doc->hasGlass(display);
      it += PinballPlugin::OverlayUI::FillDisplayGlassItems(it, str, prefix.c_str(), &display.options,
         hasGlass && display.glassImgW > 0 ? display.glassImgW
            : hasGlass                     ? 0xFFFF
                                           : 0,
         hasGlass && display.glassImgH > 0 ? display.glassImgH
            : hasGlass                     ? 0xFFFF
                                           : 0);
      if (hasGlass)
      {
         PinballPlugin::OverlayUI::FillItem(it[0], str(prefix + "glassRemove"), "Remove glass image", "Remove the glass image from the display");
         it[0].type = PAGEDUI_ITEM_ACTION;
         it[0].context = &display;
         it[0].action.Run = &OverlayGlassRemoveRun;
      }
      else
      {
         PinballPlugin::OverlayUI::FillItem(it[0], str(prefix + "glassFile"), "Glass image", "Pick an image file as the glass covering the display");
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
      auto* const doc = static_cast<Doc*>(context);
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
      const unsigned int displayItems = RootDisplayItemCount(doc);
      PinballPlugin::OverlayUI::FillItem(items[0], "dmdAdd", "Add DMD display", "Add a DMD overlay to the document");
      items[0].type = PAGEDUI_ITEM_ACTION;
      items[0].context = doc;
      items[0].action.Run = &OverlayAddRun;
      for (unsigned int i = 0; i < doc->displays.size(); i++)
      {
         Display& display = doc->displays[i];
         const int target = std::clamp(display.target, 0, doc->targetCount - 1);
         const int maxW = doc->viewW[target] > 0 ? doc->viewW[target] : 0xFFFF;
         const int maxH = doc->viewH[target] > 0 ? doc->viewH[target] : 0xFFFF;
         PagedUIItemDef* const it = items + 1 + i * displayItems;
         unsigned int n = 0;
         PinballPlugin::OverlayUI::FillItem(it[n], nullptr, str(std::format("DMD display #{}", i + 1)), nullptr);
         it[n++].type = PAGEDUI_ITEM_HEADER;
         PinballPlugin::OverlayUI::FillItem(it[n], str(std::format("dmd{}.target", i)), "Show on", "View the display is rendered on");
         it[n].type = PAGEDUI_ITEM_ENUM;
         it[n].context = &display.target;
         it[n++].enumProp = { 0, doc->targetCount, doc->targetNames, 0, &OverlayFieldGet, &OverlayFieldSet };
         PinballPlugin::OverlayUI::FillItem(it[n], str(std::format("dmd{}.source", i)), "Source", "Controller resource URI of the DMD to render (ctrl://..., empty = default DMD)");
         it[n].type = PAGEDUI_ITEM_STRING;
         it[n].context = &display;
         it[n].stringProp.defVal = "";
         it[n].stringProp.Get = &OverlaySourceGet;
         it[n++].stringProp.Set = &OverlaySourceSet;
         if (doc->detectable)
         {
            PinballPlugin::OverlayUI::FillItem(it[n], str(std::format("dmd{}.detect", i)), "Detect position", "Automatically detect the DMD bounds on the artwork image");
            it[n].type = PAGEDUI_ITEM_ACTION;
            it[n].context = &display;
            it[n++].action.Run = &OverlayDetectRun;
         }
         PinballPlugin::OverlayUI::FillIntItem(it[n++], str(std::format("dmd{}.x", i)), "X position", "Overlay bounds in document space, from the top left", &display.x, maxW);
         PinballPlugin::OverlayUI::FillIntItem(it[n++], str(std::format("dmd{}.y", i)), "Y position", "Overlay bounds in document space, from the top left", &display.y, maxH);
         PinballPlugin::OverlayUI::FillIntItem(it[n++], str(std::format("dmd{}.w", i)), "Width", "Overlay bounds in document space", &display.w, maxW);
         PinballPlugin::OverlayUI::FillIntItem(it[n++], str(std::format("dmd{}.h", i)), "Height", "Overlay bounds in document space", &display.h, maxH);
         PinballPlugin::OverlayUI::FillItem(it[n], str(std::format("dmd{}.options", i)), "Options", "Render style, tint and glass options of the display");
         it[n].type = PAGEDUI_ITEM_SUBPAGE;
         it[n++].subpage.pageId = str(std::format("dmd{}", i));
         PinballPlugin::OverlayUI::FillItem(it[n], str(std::format("dmd{}.remove", i)), "Remove display", "Remove this display from the document");
         it[n].type = PAGEDUI_ITEM_ACTION;
         it[n].context = &display;
         it[n++].action.Run = &OverlayRemoveRun;
      }
      PagedUIItemDef* const tail = items + 1 + doc->displays.size() * displayItems;
      PinballPlugin::OverlayUI::FillItem(tail[0], nullptr, str("Import from a Nixie layout file"s), nullptr);
      tail[0].type = PAGEDUI_ITEM_HEADER;
      PinballPlugin::OverlayUI::FillItem(tail[1], "importNix", "Nixie file", "Nixie layout file (.nix) to import displays from");
      tail[1].type = PAGEDUI_ITEM_FILE;
      tail[1].context = doc;
      tail[1].fileProp.defVal = "";
      tail[1].fileProp.fileFilter = "nix";
      tail[1].fileProp.Get = &OverlayImportGet;
      tail[1].fileProp.Set = &OverlayImportSet;
      PinballPlugin::OverlayUI::FillItem(tail[2], "importRun", "Import displays", "Append the DMD and screen visuals of the Nixie file to the document displays");
      tail[2].type = PAGEDUI_ITEM_ACTION;
      tail[2].context = doc;
      tail[2].action.Run = &OverlayImportRun;
   }

   // Persists the document through the plugin hook; the saved state becomes the new dirty baseline
   int MSGPIAPI OverlayCommitEdit(void* context)
   {
      auto* const doc = static_cast<Doc*>(context);
      if (doc->commitEdit == nullptr || doc->commitEdit(*doc) != 0)
         return 1;
      doc->editBackup = doc->displays;
      if (doc->requestRefresh)
         doc->requestRefresh();
      return 0;
   }

} // namespace


bool ReadFileBytes(const std::filesystem::path& path, std::vector<uint8_t>& out) { return ReadFileBytesImpl(path, out); }

void InitEditSheetDef(PagedUISheetDef* sheet, Doc* doc)
{
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
