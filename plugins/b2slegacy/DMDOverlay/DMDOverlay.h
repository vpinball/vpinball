// license:GPLv3+

#pragma once

// Shared implementation of the authored DMD overlay documents used by the B2S, B2SLegacy and
// PUP plugins: an identical copy of this folder lives inside each plugin and provides all the
// code they have in common:
//   - the document model (Display records collected in a Doc)
//   - the in-game UI editing support (InitEditSheetDef fills the PagedUI sheet callbacks)
//   - the renderer (Renderer): source frame upload, glass layer, bounds detection
//   - the shared display options (OverlayDisplayOptions.h)
// Each plugin binds the shared implementation to its own data model through the Doc hooks
// (target views, glass image storage, document persistence, bounds mapping) and only keeps
// its document file format parsing and render view setup.

#include "OverlayDisplayOptions.h"

#include "plugins/PagedUIPlugin.h"
#include "plugins/ResURIResolver.h"
#include "plugins/VPXPlugin.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <vector>


namespace DMDOverlay
{

// Logger bound by the plugin (LPI functions of LoggingPlugin.h: func, line, level, message)
using LogFn = std::function<void(const char*, int, unsigned int, const std::string&)>;

// A rectangle in pixels
template <class T> class vec4 final
{
public:
   constexpr vec4() { }
   constexpr vec4(T px, T py, T pz, T pw)
      : x(px)
      , y(py)
      , z(pz)
      , w(pw)
   {
   }

   T x = static_cast<T>(0), y = static_cast<T>(0), z = static_cast<T>(0), w = static_cast<T>(0);
};

struct Doc;

// An authored display record of the document: a DMD rendered on top of an artwork or an
// ancillary window. Presence implies enabled.
struct Display final
{
   int target = 0; // View the display is rendered on (index into Doc::targetNames/viewW/viewH)
   std::string source; // Controller resource URI (ResURIResolver syntax); empty = default DMD
   // Bounds in the plugin's document space, origin at the top left
   int x = 0, y = 0, w = 0, h = 0;
   PinballPlugin::OverlayDisplayOptions options; // Render style, dot size, tint and glass options
   // Image covering the display as a glass. Two storage flavors, chosen by the plugin through
   // the Doc glass hooks: the encoded content of the image file embedded in the document
   // (persisted inline, e.g. as base64) is kept in glassImage, while plugins referencing a
   // file keep the document-relative path in glassPath with a picked file staged in
   // glassPending until commit
   std::vector<uint8_t> glassImage;
   std::string glassPath;
   std::string glassPending;
   int detectRequest = 0; // Runtime only (not persisted): requests automatic bounds detection
   Doc* doc = nullptr; // Runtime only (not persisted): back link used by the editing sheet and the renderer
   // Runtime only (not persisted): encoded content resolved by the Doc::glassBytes hook and
   // the key identifying it (0 = none); glassSizeKey is the key last decoded into glassImgW/H
   std::vector<uint8_t> glassCache;
   size_t glassKey = 0;
   size_t glassSizeKey = 0;
   int glassImgW = 0, glassImgH = 0;
};

// The document being authored through the in-game UI: the file under edit, its live displays
// and the bounds of the target views used to limit the edited display rects (0 when unknown),
// plus the plugin-provided hooks binding the shared implementation to the plugin data model.
// The shared code degrades gracefully on missing hooks: no mapQuad falls back to a fullscreen
// flip, no glassBytes means no glass layer, no commitEdit makes the sheet read-only-ish.
struct Doc final
{
   std::filesystem::path path;
   std::vector<Display> displays;
   int viewW[3] = { 0, 0, 0 }, viewH[3] = { 0, 0, 0 }; // Pixel bounds of each target view (0 = unknown)
   const VPXPluginAPI* vpxApi = nullptr; // Runtime only: used to decode glass images
   LogFn log; // Runtime only: plugin logger (nullptr = silent)
   std::function<void()> requestRefresh; // Asks the host to rebuild the editing sheet (displays added/removed)
   std::vector<Display> editBackup; // Runtime only: displays captured at BeginEdit, restored on cancel
   std::string importPath; // Runtime only (not persisted): Nixie layout file selected by the 'import' file item
   // Plugin-provided bindings
   int targetCount = 0; // Number of render targets (viewW/viewH entries used)
   const char** targetNames = nullptr; // targetCount labels of the 'Show on' item
   bool detectable = false; // Emit the 'Detect position' action (needs 'artwork' at render)
   // Maps a display's document bounds (top-left origin) to the render quad (bottom-up render
   // space of the view); nullptr = bounds mapped against the whole view, flipping the origin
   std::function<vec4<int>(VPXRenderContext2D*, const Display&)> mapQuad;
   std::function<bool(const Display&)> hasGlass; // Whether a glass image is defined
   // Encoded content of the display's glass image (nullptr/empty = none). May lazily resolve
   // the content into glassCache and must maintain glassKey as the identity of the content
   std::function<const std::vector<uint8_t>*(Display&)> glassBytes;
   std::function<bool(Display&, const std::filesystem::path&)> setGlass; // Applies a picked or imported image file
   std::function<void(Display&)> clearGlass;
   std::function<int(Doc&)> commitEdit; // Persists the document file (0 = success)
};

class Renderer final
{
public:
   Renderer(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver);
   ~Renderer();
   // Renders one display of the document: resolves the source frame, runs a pending 'Detect
   // position' bounds search against 'artwork' (nullptr disables it), maps the document
   // bounds through Doc::mapQuad and draws the glass and emitter layers
   void Render(VPXRenderContext2D* context, Display& display, VPXTexture artwork);

private:
   vec4<int> SearchDmdSubFrame(VPXTexture image, const VPXTextureInfo* const texInfo, float dmdAspectRatio, const LogFn& log) const;

   PinballPlugin::ResURIResolver& m_resURIResolver;
   // Source frame texture owned by the overlay: as only new frames are uploaded, it can not be shared
   // between displays (a display would show the frame last uploaded by another one)
   VPXTexture m_dmdTex = nullptr;
   const VPXPluginAPI* const m_vpxApi;

   // Glass image texture owned by the overlay, recreated when the content provided by the
   // Doc::glassBytes hook changes (identified by its key)
   VPXTexture m_glassTex = nullptr;
   size_t m_glassHash = 0;

   bool m_stopSearching = false;
   std::future<vec4<int>> m_frameSearch;

   // Identity of the frame last uploaded to the texture, to avoid a full copy plus a GPU re-upload every frame
   DisplaySrcId m_uploadedSrc {};
   unsigned int m_uploadedFrameId = 0;
   bool m_hasUploadedFrame = false;
};

// Reads a whole file into 'out' (helper for the Doc::setGlass and Doc::glassBytes hooks)
bool ReadFileBytes(const std::filesystem::path& path, std::vector<uint8_t>& out);

// Fills the PagedUI sheet callbacks editing the given document (the sheet 'context' will be
// the doc pointer); the caller defines the sheet id, name and description and binds the Doc
// hooks beforehand. CommitEdit delegates to Doc::commitEdit.
void InitEditSheetDef(PagedUISheetDef* sheet, Doc* doc);

}
