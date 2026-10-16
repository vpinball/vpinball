// license:GPLv3+

#include "PUPDMDOverlay.h"
#include "common.h" // Needed for logging

#include "nixie/NixieLayoutFile.h"

#include <nlohmann/json.hpp>
#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <deque>
#include <string>
#include <format>
#include <vector>

using std::string;
using namespace std::string_literals;
using namespace std::string_view_literals;

using std::vector;


namespace PUP
{

PUPDMDOverlay::PUPDMDOverlay(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver, VPXTexture& dmdTex)
   : m_resURIResolver(resURIResolver)
   , m_dmdTex(dmdTex)
   , m_vpxApi(vpxApi)
{
}

PUPDMDOverlay::~PUPDMDOverlay()
{
   if (m_glassTex != nullptr && m_vpxApi != nullptr)
      m_vpxApi->DeleteTexture(m_glassTex);
}

void PUPDMDOverlay::Render(VPXRenderContext2D* ctx, PUPDMDDisplay* display)
{
   // The display is enabled by its sole presence in the document
   PinballPlugin::ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState(display->source.empty() ? PinballPlugin::ResURIResolver::DefaultDmdUri : display->source);
   if (dmd.state.frame == nullptr)
      return;

   // Bounds are authored with a top left origin (like PUP screen custom positions), spanning
   // the whole rendered window: flip to the bottom up render space of the view
   const int x = display->x;
   const int y = static_cast<int>(ctx->srcHeight) - display->y - display->h;
   const int w = display->w;
   const int h = display->h;
   if (w == 0 || h == 0)
      return;
   if (y + h <= 0 || y >= static_cast<int>(ctx->srcHeight))
      return; // Fully off the window

   // The texture is owned by the manager, hence the null test: it may still have to be created here
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

   // The glass texture is owned by the overlay: (re)create it when the display's glass
   // image path changes. A staged pick/import (glassPending) is preferred as it already
   // resolves to a file before the commit copies it into the pack; the persisted path is
   // relative to the pup pack root
   string glassPath;
   if (!display->glassPending.empty())
      glassPath = PluginStrings::PathToUTF8(PluginStrings::PathFromNative(display->glassPending.c_str()));
   else if (display->doc != nullptr && !display->glassImage.empty())
      glassPath = PluginStrings::PathToUTF8((display->doc->path.parent_path() / PluginStrings::PathFromUTF8(display->glassImage)).lexically_normal());
   if (glassPath != m_glassPath)
   {
      if (m_glassTex != nullptr)
      {
         m_vpxApi->DeleteTexture(m_glassTex);
         m_glassTex = nullptr;
      }
      if (!glassPath.empty())
      {
         const std::filesystem::path resolvedPath = find_case_insensitive_file_path(PluginStrings::PathFromUTF8(glassPath));
         if (!resolvedPath.empty())
         {
            std::ifstream file(resolvedPath, std::ios::binary | std::ios::ate);
            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            vector<uint8_t> buffer(static_cast<size_t>(size));
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            file.close();
            m_glassTex = m_vpxApi->CreateTexture(buffer.data(), static_cast<int>(buffer.size()));
         }
         else
         {
            LOGE("Missing glass file: " + glassPath);
         }
      }
      m_glassPath = glassPath;
   }

   const PinballPlugin::OverlayDisplayOptions& o = display->options;
   float glassAreaX = 0.f, glassAreaY = 0.f, glassAreaW = 1.f, glassAreaH = 1.f;
   if (m_glassTex != nullptr && o.glassAreaW != 0 && o.glassAreaH != 0)
      if (const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(m_glassTex); texInfo != nullptr)
      {
         glassAreaX = o.glassAreaX / static_cast<float>(texInfo->width);
         glassAreaY = o.glassAreaY / static_cast<float>(texInfo->height);
         glassAreaW = o.glassAreaW / static_cast<float>(texInfo->width);
         glassAreaH = o.glassAreaH / static_cast<float>(texInfo->height);
      }
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
      m_glassTex, o.glassTintR, o.glassTintG, o.glassTintB, o.glassRoughness, // Glass texture, tint and roughness
      glassAreaX, glassAreaY, glassAreaW, glassAreaH, // Glass texture coordinates (inside overall glass texture)
      PinballPlugin::OverlayInvsRGB(o.glassAmbientR), PinballPlugin::OverlayInvsRGB(o.glassAmbientG), PinballPlugin::OverlayInvsRGB(o.glassAmbientB), // Glass lighting from room
      // Second layer: emitter
      m_dmdTex, o.tintR, o.tintG, o.tintB, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
      o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB, // Emitter padding (from glass border)
      // Render quad
      static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
}


///////////////////////////////////////////////////////////////////////////////
// Document editing page (PagedUIPlugin): the root page enumerates the records of the
// document's 'displays.pup' file, one section per display with an 'options' subpage
// editing the render style and the glass layer, plus 'add' and Nixie 'import' actions.
// Property item contexts directly point at the edited field of the display entries of
// doc->displays. CommitEdit copies the staged glass images into the pack's 'PUPDisplays'
// folder and rewrites the document file.

namespace
{

   // Lazily resolves the pixel size of the effective glass file (the staged pick when
   // present, else the pack-relative persisted image), cached against its path: the
   // image area fields need the image bounds at enumeration time
   void GlassImageSize(PUP::PUPDMDDisplay& display)
   {
      const string& key = !display.glassPending.empty() ? display.glassPending : display.glassImage;
      if (key == display.glassImgKey)
         return;
      display.glassImgKey = key;
      display.glassImgW = display.glassImgH = 0;
      if (key.empty() || display.doc == nullptr)
         return;
      const std::filesystem::path path
         = !display.glassPending.empty() ? PluginStrings::PathFromNative(display.glassPending.c_str()) : display.doc->path.parent_path() / PluginStrings::PathFromUTF8(display.glassImage);
      if (SDL_Surface* const surface = IMG_Load(PluginStrings::PathToUTF8(path).c_str()))
      {
         display.glassImgW = surface->w;
         display.glassImgH = surface->h;
         SDL_DestroySurface(surface);
      }
   }

   int MSGPIAPI OverlayFieldGet(void* context) { return *static_cast<int*>(context); }
   void MSGPIAPI OverlayFieldSet(void* context, int value) { *static_cast<int*>(context) = value; }
   const char* MSGPIAPI OverlaySourceGet(void* context) { return static_cast<PUP::PUPDMDDisplay*>(context)->source.c_str(); }
   void MSGPIAPI OverlaySourceSet(void* context, const char* value) { static_cast<PUP::PUPDMDDisplay*>(context)->source = value != nullptr ? value : ""; }
   // The picked file is only staged (see PUPDMDDisplay::glassPending): Get reports the
   // staged source path so that a new pick replaces it
   const char* MSGPIAPI OverlayGlassGet(void* context) { return static_cast<PUP::PUPDMDDisplay*>(context)->glassPending.c_str(); }
   void MSGPIAPI OverlayGlassSet(void* context, const char* value)
   {
      auto* const display = static_cast<PUP::PUPDMDDisplay*>(context);
      const std::filesystem::path picked = value != nullptr ? PluginStrings::PathFromNative(value) : std::filesystem::path();
      if (picked.empty())
      {
         // Reverting to the snapshot value also lands here: the persisted path has no
         // picker representation, so only the staged pick is cleared (the 'Remove'
         // action drops the glass image entirely)
         display->glassPending.clear();
      }
      else
      {
         // The staged path keeps the picker encoding (native): Get reports it back unchanged
         display->glassPending = value;
         display->glassImage = "PUPDisplays/"s + PluginStrings::PathToUTF8(picked.filename());
      }
      if (display->doc != nullptr && display->doc->requestRefresh)
         display->doc->requestRefresh();
   }
   void MSGPIAPI OverlayGlassRemoveRun(void* context)
   {
      auto* const display = static_cast<PUP::PUPDMDDisplay*>(context);
      display->glassImage.clear();
      display->glassPending.clear();
      if (display->doc != nullptr && display->doc->requestRefresh)
         display->doc->requestRefresh();
   }
   const char* MSGPIAPI OverlayImportGet(void* context) { return static_cast<PUP::PUPOverlayDoc*>(context)->importPath.c_str(); }
   void MSGPIAPI OverlayImportSet(void* context, const char* value) { static_cast<PUP::PUPOverlayDoc*>(context)->importPath = value != nullptr ? value : ""; }
   void MSGPIAPI OverlayImportRun(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
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
         const int target = layout.window == VPXWindowId::VPXWINDOW_ScoreView ? 1 : layout.window == VPXWindowId::VPXWINDOW_Topper ? 2 : layout.window == VPXWindowId::VPXWINDOW_Backglass ? 0 : -1;
         if (target < 0)
            continue;
         const float dstW = doc->winW[target] > 0 ? static_cast<float>(doc->winW[target]) : layout.width;
         const float dstH = doc->winH[target] > 0 ? static_cast<float>(doc->winH[target]) : layout.height;
         for (const PinballPlugin::NixieFile::Visual& visual : layout.visuals)
         {
            if (visual.type != PinballPlugin::NixieFile::DMD && visual.type != PinballPlugin::NixieFile::Screen)
               continue;
            PUP::PUPDMDDisplay& display = doc->displays.emplace_back();
            display.doc = doc;
            display.target = target;
            display.source = visual.srcUri == "ctrl://default/display" ? "" : visual.srcUri;
            display.x = static_cast<int>(std::lround(visual.x * dstW / layout.width));
            display.y = static_cast<int>(std::lround(visual.y * dstH / layout.height));
            display.w = static_cast<int>(std::lround(visual.w * dstW / layout.width));
            display.h = static_cast<int>(std::lround(visual.h * dstH / layout.height));
            PinballPlugin::ApplyNixieVisualOptions(display.options, visual);
            // The glass file is staged, not copied yet: it lands in 'PUPDisplays' at the
            // next successful commit, so cancelling the edit leaves no orphan files
            if (!visual.glassPath.empty())
            {
               const std::filesystem::path glassPath = nixPath.parent_path() / PluginStrings::PathFromUTF8(visual.glassPath);
               display.glassPending = PluginStrings::PathToNative(glassPath);
               display.glassImage = "PUPDisplays/"s + PluginStrings::PathToUTF8(glassPath.filename());
            }
         }
      }
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayRemoveRun(void* context)
   {
      auto* const display = static_cast<PUP::PUPDMDDisplay*>(context);
      PUP::PUPOverlayDoc* const doc = display->doc;
      std::erase_if(doc->displays, [display](const PUP::PUPDMDDisplay& d) { return &d == display; });
      if (doc->requestRefresh)
         doc->requestRefresh();
   }
   void MSGPIAPI OverlayAddRun(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      PUP::PUPDMDDisplay& display = doc->displays.emplace_back();
      display.doc = doc;
      display.options.glassRoughness = 0.1f;
      if (doc->requestRefresh)
         doc->requestRefresh();
   }

   // Edit session: the host only tracks item value changes through its docSnapshot, so the document
   // itself is captured here to detect (and discard) structural changes such as added/removed displays
   void MSGPIAPI OverlayBeginEdit(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      doc->editBackup = doc->displays;
   }

   // Persisted fields only: the doc back link and the staged glass source are runtime state
   static bool SameDisplay(const PUP::PUPDMDDisplay& a, const PUP::PUPDMDDisplay& b)
   {
      return a.target == b.target && a.source == b.source && a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h //
         && a.glassImage == b.glassImage && a.glassPending == b.glassPending && PinballPlugin::SameOverlayDisplayOptions(a.options, b.options);
   }

   int MSGPIAPI OverlayIsDirty(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      return doc->displays.size() != doc->editBackup.size() || !std::equal(doc->displays.begin(), doc->displays.end(), doc->editBackup.begin(), SameDisplay);
   }

   void MSGPIAPI OverlayEndEdit(void* context, int committed)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      // Discard uncommitted changes: item values are reverted through their Set callbacks, this covers
      // the structural changes (added/removed displays) including edits made after a save in the same
      // session (the backup is re-based on each successful commit)
      if (!committed || OverlayIsDirty(context))
      {
         doc->displays = doc->editBackup;
         for (PUP::PUPDMDDisplay& display : doc->displays)
            display.doc = doc;
      }
      doc->editBackup.clear();
   }

   unsigned int MSGPIAPI OverlayPageCount(void* context) { return 1 + static_cast<unsigned int>(static_cast<PUP::PUPOverlayDoc*>(context)->displays.size()); }
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

   // Root page: 'add' action, 9 items per display, then the 3 'import' items
   constexpr unsigned int kRootDisplayItems = 9;
   unsigned int MSGPIAPI OverlayItemCount(void* context, const char* pageId)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      // Display subpage: shared display option items (incl. the glass section header), shared
      // glass option items (image area fields only when a glass image is defined), then the
      // image file picker or the remove action
      if (strncmp(pageId, "dmd", 3) == 0)
      {
         const unsigned int i = static_cast<unsigned int>(atoi(pageId + 3));
         if (i >= doc->displays.size())
            return 0;
         PUP::PUPDMDDisplay& display = doc->displays[i];
         GlassImageSize(display);
         return PinballPlugin::OverlayUI::OverlayDisplayOptionItemCount
            + PinballPlugin::OverlayUI::OverlayGlassOptionItemCount(display.glassPending.empty() && display.glassImage.empty() ? 0 : 1) + 1;
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
   // glass option items, then the image file picker staged into the pack's
   // 'PUPDisplays' folder at commit)
   void FillDisplayPageItems(PagedUIItemDef* items, PUP::PUPDMDDisplay& display, unsigned int i, const std::function<const char*(string)>& str)
   {
      const string prefix = std::format("dmd{}.", i);
      PagedUIItemDef* it = items;
      GlassImageSize(display);
      it += PinballPlugin::OverlayUI::FillDisplayOptionItems(it, str, prefix.c_str(), &display.options);
      // When a glass is defined (persisted path or staged pick) the texture section only shows
      // the image area fields (bounded by the decoded image size, with a fallback range when it
      // cannot be resolved) and the remove action; the file picker replaces them otherwise
      const bool hasGlass = !display.glassPending.empty() || !display.glassImage.empty();
      it += PinballPlugin::OverlayUI::FillDisplayGlassItems(it, str, prefix.c_str(), &display.options,
         hasGlass && display.glassImgW > 0 ? display.glassImgW
            : hasGlass                     ? 0xFFFF
                                           : 0,
         hasGlass && display.glassImgH > 0 ? display.glassImgH
            : hasGlass                     ? 0xFFFF
                                           : 0);
      if (hasGlass)
      {
         FillOverlayItem(it[0], str(prefix + "glassRemove"), "Remove glass image", "Remove the glass image from the display");
         it[0].type = PAGEDUI_ITEM_ACTION;
         it[0].context = &display;
         it[0].action.Run = &OverlayGlassRemoveRun;
      }
      else
      {
         FillOverlayItem(it[0], str(prefix + "glassFile"), "Glass image", "Pick an image file to copy into the pack's PUPDisplays folder as the glass covering the display");
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
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
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
      static const char* targetValues[] = { "Backglass", "Score view", "Topper" };
      FillOverlayItem(items[0], "dmdAdd", "Add DMD display", "Add a DMD overlay to the displays file");
      items[0].type = PAGEDUI_ITEM_ACTION;
      items[0].context = doc;
      items[0].action.Run = &OverlayAddRun;
      for (unsigned int i = 0; i < doc->displays.size(); i++)
      {
         PUP::PUPDMDDisplay& display = doc->displays[i];
         const int target = display.target >= 0 && display.target < 3 ? display.target : 0;
         const int maxW = doc->winW[target] > 0 ? doc->winW[target] : 0xFFFF;
         const int maxH = doc->winH[target] > 0 ? doc->winH[target] : 0xFFFF;
         PagedUIItemDef* const it = items + 1 + i * kRootDisplayItems;
         FillOverlayItem(it[0], nullptr, str(std::format("DMD display #{}", i + 1)), nullptr);
         it[0].type = PAGEDUI_ITEM_HEADER;
         FillOverlayItem(it[1], str(std::format("dmd{}.target", i)), "Show on", "Window the display is rendered on");
         it[1].type = PAGEDUI_ITEM_ENUM;
         it[1].context = &display.target;
         it[1].enumProp = { 0, 3, targetValues, 0, &OverlayFieldGet, &OverlayFieldSet };
         FillOverlayItem(it[2], str(std::format("dmd{}.source", i)), "Source", "Controller resource URI of the DMD to render (ctrl://..., empty = default DMD)");
         it[2].type = PAGEDUI_ITEM_STRING;
         it[2].context = &display;
         it[2].stringProp.defVal = "";
         it[2].stringProp.Get = &OverlaySourceGet;
         it[2].stringProp.Set = &OverlaySourceSet;
         FillOverlayInt(it[3], str(std::format("dmd{}.x", i)), "X position", "Overlay bounds in window pixels, from the top left", &display.x, maxW);
         FillOverlayInt(it[4], str(std::format("dmd{}.y", i)), "Y position", "Overlay bounds in window pixels, from the top left", &display.y, maxH);
         FillOverlayInt(it[5], str(std::format("dmd{}.w", i)), "Width", "Overlay bounds in window pixels", &display.w, maxW);
         FillOverlayInt(it[6], str(std::format("dmd{}.h", i)), "Height", "Overlay bounds in window pixels", &display.h, maxH);
         FillOverlayItem(it[7], str(std::format("dmd{}.options", i)), "Options", "Render style, tint and glass options of the display");
         it[7].type = PAGEDUI_ITEM_SUBPAGE;
         it[7].subpage.pageId = str(std::format("dmd{}", i));
         FillOverlayItem(it[8], str(std::format("dmd{}.remove", i)), "Remove display", "Remove this display from the displays file");
         it[8].type = PAGEDUI_ITEM_ACTION;
         it[8].context = &display;
         it[8].action.Run = &OverlayRemoveRun;
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

   // Copies the glass images staged by a file pick or a Nixie import into the pack's
   // 'PUPDisplays' folder, deduplicating names; updates glassImage to the stored
   // relative path and clears the staged source
   void CopyStagedGlassImages(PUP::PUPOverlayDoc* doc)
   {
      const std::filesystem::path glassDir = doc->path.parent_path() / "PUPDisplays";
      for (PUP::PUPDMDDisplay& display : doc->displays)
      {
         if (display.glassPending.empty())
            continue;
         const std::filesystem::path src = PluginStrings::PathFromNative(display.glassPending.c_str());
         std::error_code ec;
         std::filesystem::create_directories(glassDir, ec);
         std::filesystem::path dst = glassDir / src.filename();
         if (!std::filesystem::equivalent(src, dst, ec))
         {
            for (int n = 1; std::filesystem::exists(dst, ec); n++)
            {
               std::filesystem::path candidate = src.stem();
               candidate += "_" + std::to_string(n);
               candidate += src.extension();
               dst = glassDir / candidate;
            }
            if (!std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec))
               LOGE("Failed to copy glass image " + PluginStrings::PathToUTF8(src) + " to " + PluginStrings::PathToUTF8(dst) + ": " + ec.message());
         }
         display.glassImage = "PUPDisplays/"s + PluginStrings::PathToUTF8(dst.filename());
         display.glassPending.clear();
      }
   }

   // Rewrites the whole document file: a JSON 'Displays' array of { Target, Source, X, Y, W, H,
   // plus the optional render style/tint/glass fields }, matching the other .pup files of the pack
   int MSGPIAPI OverlayCommitEdit(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      if (doc->path.empty())
         return 1;
      CopyStagedGlassImages(doc);
      std::ofstream out(doc->path, std::ios::binary | std::ios::trunc);
      if (!out)
      {
         LOGE("Failed to write displays file: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      static const char* targetNames[] = { "Backglass", "ScoreView", "Topper" };
      nlohmann::json json;
      json["Displays"] = nlohmann::json::array();
      for (const PUP::PUPDMDDisplay& display : doc->displays)
      {
         const char* const target = display.target >= 0 && display.target < 3 ? targetNames[display.target] : targetNames[0];
         nlohmann::json entry = { { "Target", target }, { "Source", display.source }, { "X", display.x }, { "Y", display.y }, { "W", display.w }, { "H", display.h } };
         const PinballPlugin::OverlayDisplayOptions& o = display.options;
         if (o.style > 0 && o.style <= 8)
            entry["Style"] = PinballPlugin::OverlayDisplayStyleNames[o.style - 1];
         if (o.sizeW > 0 || o.sizeH > 0)
            entry["Dots"] = { o.sizeW, o.sizeH };
         if (o.tintR != 1.f || o.tintG != 1.f || o.tintB != 1.f)
            entry["Tint"] = { o.tintR, o.tintG, o.tintB };
         nlohmann::json glass;
         if (!display.glassImage.empty())
            glass["Image"] = display.glassImage;
         if (o.glassAreaX != 0 || o.glassAreaY != 0 || o.glassAreaW != 0 || o.glassAreaH != 0)
            glass["Area"] = { o.glassAreaX, o.glassAreaY, o.glassAreaW, o.glassAreaH };
         if (o.glassTintR != 1.f || o.glassTintG != 1.f || o.glassTintB != 1.f)
            glass["Tint"] = { o.glassTintR, o.glassTintG, o.glassTintB };
         if (o.glassRoughness != 0.f)
            glass["Roughness"] = o.glassRoughness;
         if (o.glassAmbientR != 1.f || o.glassAmbientG != 1.f || o.glassAmbientB != 1.f)
            glass["Ambient"] = { o.glassAmbientR, o.glassAmbientG, o.glassAmbientB };
         if (o.glassPadL != 0.f || o.glassPadT != 0.f || o.glassPadR != 0.f || o.glassPadB != 0.f)
            glass["Pad"] = { o.glassPadL, o.glassPadT, o.glassPadR, o.glassPadB };
         if (!glass.empty())
            entry["Glass"] = glass;
         json["Displays"].push_back(entry);
      }
      out << json.dump(4);
      out.close();
      if (out.fail())
      {
         LOGE("Failed to write displays file: " + PluginStrings::PathToUTF8(doc->path));
         return 1;
      }
      doc->editBackup = doc->displays; // Saved state becomes the new dirty baseline
      return 0;
   }

} // namespace


void PUPDMDOverlay::InitEditSheetDef(PagedUISheetDef* sheet, PUP::PUPOverlayDoc* doc)
{
   sheet->id = "displays";
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