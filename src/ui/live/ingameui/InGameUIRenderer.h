// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class InGameUI;
class InGameUIPage;

// Renders the InGameUI pages. The page objects own the model (items, selection,
// save flow); the renderer owns layout and widget-level interaction. Multiple
// implementations are expected for the different running targets (mobile touch,
// desktop, cabinet, VR).
class InGameUIRenderer
{
public:
   virtual ~InGameUIRenderer() = default;

   // Renders the stack of active pages (back = topmost). Page/pane layout is
   // entirely the implementation's choice.
   virtual void RenderPages(const vector<InGameUIPage*>& pages, float elapsedS) = 0;

   // Called when a new in-game UI session is opened.
   virtual void OnUIOpened() { }

   // Flipper/button navigation events. The default implementation declines the
   // event and the standard behavior applies (moving/adjusting the active page's
   // item selection); renderers with multi-level navigation consume the events.
   virtual bool OnUINavigatePrev(InGameUIPage* page) { return false; }
   virtual bool OnUINavigateNext(InGameUIPage* page) { return false; }
   virtual bool OnUIAdjust(InGameUIPage* page, float direction, bool isInitialPress) { return false; }
   virtual bool OnUIReleaseAdjust(InGameUIPage* page) { return false; }
   virtual bool OnUINavigateBack(InGameUIPage* page) { return false; }

   // Creates the renderer for the running target (mobile touch / desktop /
   // cabinet / VR). Single default implementation for now; the dispatch point
   // for per-target implementations.
   static std::unique_ptr<InGameUIRenderer> Create(InGameUI& owner);
};

}
