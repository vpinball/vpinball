// license:GPLv3+

#pragma once

#include "common.h"

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

   // Identity of the frame last uploaded to the texture, to avoid a full copy plus a GPU re-upload every frame
   DisplaySrcId m_uploadedSrc {};
   unsigned int m_uploadedFrameId = 0;
   bool m_hasUploadedFrame = false;
};

}
