// license:GPLv3+

#include "PUPDMDOverlay.h"
#include "common.h" // Needed for logging

#include <nlohmann/json.hpp>

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

PUPDMDOverlay::~PUPDMDOverlay() { }

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

   VPXDisplayRenderStyle style = VPXDisplayRenderStyle::VPXDMDStyle_Plasma;
   switch (dmd.source->hardware & 0xFFFF0000)
   {
   case CTLPI_DISPLAY_HARDWARE_NEON_PLASMA: style = VPXDisplayRenderStyle::VPXDMDStyle_Plasma; break;
   case CTLPI_DISPLAY_HARDWARE_RED_LED: style = VPXDisplayRenderStyle::VPXDMDStyle_RedLED; break;
   case CTLPI_DISPLAY_HARDWARE_RGB_LED: style = VPXDisplayRenderStyle::VPXDMDStyle_GenLED; break;
   case CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY: style = VPXDisplayRenderStyle::VPXDMDStyle_CRT; break;
   }
   ctx->DrawDisplay(ctx, style,
      // First layer: glass
      nullptr, 1.f, 1.f, 1.f, 0.f, // Glass texture, tint and roughness
      0.f, 0.f, 0.f, 0.f, // Glass texture coordinates (inside overall glass texture)
      1.f, 1.f, 1.f, // Glass lighting from room
      // Second layer: emitter
      m_dmdTex, 1.f, 1.f, 1.f, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
      0.f, 0.f, 0.f, 0.f, // Emitter padding (from glass border)
      // Render quad
      static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
}


///////////////////////////////////////////////////////////////////////////////
// Document editing page (PagedUIPlugin): enumerates the records of the document's
// 'displays.pup' file, one section per display plus an 'add' action. Property item contexts
// directly point at the edited field of the display entries of doc->displays. CommitEdit
// rewrites the document file.

namespace
{

   int MSGPIAPI OverlayFieldGet(void* context) { return *static_cast<int*>(context); }
   void MSGPIAPI OverlayFieldSet(void* context, int value) { *static_cast<int*>(context) = value; }
   const char* MSGPIAPI OverlaySourceGet(void* context) { return static_cast<PUP::PUPDMDDisplay*>(context)->source.c_str(); }
   void MSGPIAPI OverlaySourceSet(void* context, const char* value) { static_cast<PUP::PUPDMDDisplay*>(context)->source = value != nullptr ? value : ""; }
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

   // Persisted fields only: the doc back link is runtime state
   static bool SameDisplay(const PUP::PUPDMDDisplay& a, const PUP::PUPDMDDisplay& b)
   {
      return a.target == b.target && a.source == b.source && a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
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

   unsigned int MSGPIAPI OverlayPageCount(void*) { return 1; }
   void MSGPIAPI OverlayPageInfo(void*, unsigned int, PagedUIPageInfo* info)
   {
      info->id = "displays";
      info->title = nullptr;
      info->info = nullptr;
   }
   unsigned int MSGPIAPI OverlayItemCount(void* context, const char*) { return static_cast<unsigned int>(static_cast<PUP::PUPOverlayDoc*>(context)->displays.size()) * 8 + 1; }

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

   void MSGPIAPI OverlayGetItems(void* context, const char*, PagedUIItemDef* items)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      // Generated ids/labels are kept in static storage: they are consumed by the host right
      // after this call and the callbacks only run on the API thread. A deque keeps the
      // pointers stable: item ids are the c_str() of these strings, read after the loop
      static std::deque<string> strings;
      strings.clear();
      const auto str = [](string s) { return strings.emplace_back(std::move(s)).c_str(); };
      static const char* targetValues[] = { "Backglass", "Score view", "Topper" };
      for (unsigned int i = 0; i < doc->displays.size(); i++)
      {
         PUP::PUPDMDDisplay& display = doc->displays[i];
         const int target = display.target >= 0 && display.target < 3 ? display.target : 0;
         const int maxW = doc->winW[target] > 0 ? doc->winW[target] : 0xFFFF;
         const int maxH = doc->winH[target] > 0 ? doc->winH[target] : 0xFFFF;
         PagedUIItemDef* const it = items + i * 8;
         FillOverlayItem(it[0], nullptr, str(std::format("DMD display #{}", i + 1)), nullptr);
         it[0].type = PAGEDUI_ITEM_HEADER;
         FillOverlayItem(it[1], str(std::format("dmd{}.target", i)), "Show on", "Window the display is rendered on");
         it[1].type = PAGEDUI_ITEM_ENUM;
         it[1].context = &display.target;
         it[1].enumProp = { 0, 2, targetValues, 0, &OverlayFieldGet, &OverlayFieldSet };
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
         FillOverlayItem(it[7], str(std::format("dmd{}.remove", i)), "Remove display", "Remove this display from the displays file");
         it[7].type = PAGEDUI_ITEM_ACTION;
         it[7].context = &display;
         it[7].action.Run = &OverlayRemoveRun;
      }
      PagedUIItemDef& addItem = items[doc->displays.size() * 8];
      FillOverlayItem(addItem, "dmdAdd", "Add DMD display", "Add a DMD overlay to the displays file");
      addItem.type = PAGEDUI_ITEM_ACTION;
      addItem.context = doc;
      addItem.action.Run = &OverlayAddRun;
   }

   // Rewrites the whole document file: a JSON 'Displays' array of { Target, Source, X, Y, W, H }
   // objects, matching the other .pup files of the pack
   int MSGPIAPI OverlayCommitEdit(void* context)
   {
      auto* const doc = static_cast<PUP::PUPOverlayDoc*>(context);
      if (doc->path.empty())
         return 1;
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
         json["Displays"].push_back({ { "Target", target }, { "Source", display.source }, { "X", display.x }, { "Y", display.y }, { "W", display.w }, { "H", display.h } });
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
