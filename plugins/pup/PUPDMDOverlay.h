// license:GPLv3+

#pragma once

#include "common.h"

#include "plugins/OverlayDisplayOptions.h"
#include "plugins/ResURIResolver.h"
#include "plugins/PagedUIPlugin.h"
#include "plugins/VPXPlugin.h"

#include <functional>


namespace PUP
{

struct PUPOverlayDoc;

// A user-authored record of the pup folder 'displays.pup' file: a DMD rendered on top of the
// content of one of the ancillary windows. Presence implies enabled.
struct PUPDMDDisplay final
{
   int target = 0; // Ancillary window the display is rendered on: 0 = backglass, 1 = score view, 2 = topper
   string source; // Controller resource URI (ResURIResolver syntax); empty = default DMD
   // Bounds in the target window space: pixels of the rendered window, origin at its top left
   int x = 0, y = 0, w = 0, h = 0;
   PinballPlugin::OverlayDisplayOptions options; // Render style, dot size, tint and glass options
   // Image covering the display as a glass: path relative to the pack root, inside the
   // 'PUPDisplays' folder of the pack
   string glassImage;
   // Runtime only (not persisted): file picked through the editing page or set by a Nixie
   // import, copied into the 'PUPDisplays' folder by the next successful commit (empty = none)
   string glassPending;
   // Runtime only (not persisted): resolved pixel size of the effective glass file (the staged
   // pick when present, else the pack-relative glassImage), cached against its path
   // (0 when there is no glass image or it fails to load)
   string glassImgKey;
   int glassImgW = 0, glassImgH = 0;
   PUPOverlayDoc* doc = nullptr; // Runtime only (not persisted): back link used by the editing page
};

// The document being authored through the in game UI: file under edit, its live content, and
// the bounds of the windows used to limit the edited display rects (0 when unknown)
struct PUPOverlayDoc final
{
   std::filesystem::path path;
   vector<PUPDMDDisplay> displays;
   int winW[3] = { 0, 0, 0 }; // Last rendered size of each target window, indexed by target
   int winH[3] = { 0, 0, 0 };
   std::function<void()> requestRefresh; // Asks the host to rebuild the editing page (display added/removed)
   vector<PUPDMDDisplay> editBackup; // Runtime only: displays captured at BeginEdit, restored on cancel and used for dirty state
   string importPath; // Runtime only (not persisted): Nixie layout file selected by the 'import' file item
};

class PUPDMDOverlay final
{
public:
   PUPDMDOverlay(const VPXPluginAPI* const vpxApi, PinballPlugin::ResURIResolver& resURIResolver, VPXTexture& dmdTex);
   ~PUPDMDOverlay();
   // Renders one display of the document (bounds in window space, origin at the top left)
   void Render(VPXRenderContext2D* context, PUPDMDDisplay* display);

   // Document editing page (see plugins/PagedUIPlugin.h): fills the sheet callbacks editing the
   // given document (sheet 'context' will be the doc pointer). CommitEdit rewrites the document's
   // 'displays.pup' file. The caller still defines the sheet name/description.
   static void InitEditSheetDef(PagedUISheetDef* sheet, PUPOverlayDoc* doc);

private:
   PinballPlugin::ResURIResolver& m_resURIResolver;
   VPXTexture& m_dmdTex;
   const VPXPluginAPI* const m_vpxApi;

   // Glass image texture owned by the overlay, recreated when the display's glass
   // image path changes (m_glassPath is the resolved path it was created from)
   VPXTexture m_glassTex = nullptr;
   string m_glassPath;

   // Identity of the frame last uploaded to the texture, to avoid a full copy plus a GPU re-upload every frame
   DisplaySrcId m_uploadedSrc {};
   unsigned int m_uploadedFrameId = 0;
   bool m_hasUploadedFrame = false;
};

}
